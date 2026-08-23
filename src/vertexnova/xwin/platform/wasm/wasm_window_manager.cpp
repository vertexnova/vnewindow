/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   April 2026
 *
 * Autodoc:   yes
 * ----------------------------------------------------------------------
 */

#include "wasm_window_manager.h"

#include <vertexnova/logging/logging.h>

#include "wasm_map_key.h"
#include "wasm_window.h"

#include <algorithm>
#include <chrono>
#include <thread>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>

extern "C" void vne_xwin_shell_focus_window(int id);
#endif

CREATE_VNE_LOGGER_CATEGORY("vne.xwin.wasm_window_manager");

namespace vne::xwin {

WasmWindowManager::WasmWindowManager() = default;

WasmWindowManager::~WasmWindowManager() {
    shutdown();
}

bool WasmWindowManager::initialize() {
    initialized_ = true;
    registerGlobalCallbacks();
    return true;
}

void WasmWindowManager::shutdown() {
    unregisterGlobalCallbacks();
    destroyAllWindows();
    initialized_ = false;
}

bool WasmWindowManager::isInitialized() const noexcept {
    return initialized_;
}

std::shared_ptr<IWindow> WasmWindowManager::openWindow(const WindowDescriptor& descriptor) {
    if (!initialized_) {
        return nullptr;
    }
#ifdef __EMSCRIPTEN__
    if (!windows_.empty() && !WasmWindow::detectVneShell()) {
        // Was a silent nullptr, which is impossible to diagnose from the browser. The DOM only
        // gives us one default canvas; extra windows need a host shell to create more.
        VNE_LOG_WARN << "[xwin/wasm] Refusing a second window: no window.VneShell found. The page "
                        "must provide a shell that can create additional canvases, or the app "
                        "should check IWindowManager::supportsMultipleWindows() first.";
        return nullptr;
    }
#endif
    auto w = std::make_shared<WasmWindow>();
    w->setEventOwner(this);
    const bool is_primary = windows_.empty();
    w->prepareInitialize(is_primary);
    w->initialize(descriptor);
    if (!w->isOpen()) {
        return nullptr;
    }
    windows_.push_back(w);
    if (!primary_) {
        primary_ = w;
    }
    focused_ = w;
#ifdef __EMSCRIPTEN__
    focusWindow(w);
#endif
    return w;
}

std::shared_ptr<IWindow> WasmWindowManager::openWindow(const std::string& title, uint32_t width, uint32_t height) {
    WindowDescriptor descriptor(title, width, height);
    return openWindow(descriptor);
}

void WasmWindowManager::removeWindow(std::shared_ptr<IWindow> window) {
    if (!window) {
        return;
    }
    if (auto wasm = std::dynamic_pointer_cast<WasmWindow>(window)) {
        wasm->setEventOwner(nullptr);
    }
    window->close();
    auto it = std::find(windows_.begin(), windows_.end(), window);
    if (it != windows_.end()) {
        windows_.erase(it);
    }
    if (primary_ == window) {
        primary_ = windows_.empty() ? nullptr : windows_.front();
    }
    if (focused_ == window) {
        focused_ = primary_;
        if (focused_) {
            focusWindow(focused_);
        }
    }
}

void WasmWindowManager::destroyAllWindows() {
    for (auto& w : windows_) {
        if (w) {
            if (auto wasm = std::dynamic_pointer_cast<WasmWindow>(w)) {
                wasm->setEventOwner(nullptr);
            }
            w->close();
        }
    }
    windows_.clear();
    primary_.reset();
    focused_.reset();
}

size_t WasmWindowManager::getWindowCount() const noexcept {
    return windows_.size();
}

std::vector<std::shared_ptr<IWindow>> WasmWindowManager::getWindows() const {
    return windows_;
}

std::shared_ptr<IWindow> WasmWindowManager::getPrimaryWindow() const noexcept {
    return primary_;
}

std::shared_ptr<IWindow> WasmWindowManager::getFocusedWindow() const noexcept {
    return focused_;
}

void WasmWindowManager::setPrimaryWindow(std::shared_ptr<IWindow> window) {
    primary_ = std::move(window);
}

void WasmWindowManager::focusWindow(std::shared_ptr<IWindow> window) {
    if (focused_ && focused_ != window) {
        if (auto* prev = dynamic_cast<WasmWindow*>(focused_.get())) {
            prev->emitWindowFocus(false);
        }
    }
    focused_ = std::move(window);
    if (!focused_) {
        return;
    }
    if (auto* ww = dynamic_cast<WasmWindow*>(focused_.get())) {
        if (ww->usesVneShell()) {
            vne_xwin_shell_focus_window(static_cast<int>(ww->getId()));
        } else {
            EM_ASM({
                var c = document.getElementById('canvas');
                if (c && c.focus) {
                    c.focus();
                }
            });
        }
        ww->emitWindowFocus(true);
    }
}

void WasmWindowManager::focusWindowFromCanvas(WasmWindow* window) {
    if (!window) {
        return;
    }
    for (const auto& w : windows_) {
        if (w.get() == window) {
            focusWindow(w);
            return;
        }
    }
}

bool WasmWindowManager::supportsMultipleWindows() const noexcept {
#ifdef __EMSCRIPTEN__
    return WasmWindow::detectVneShell();
#else
    return false;
#endif
}

void WasmWindowManager::processEvents() {}

bool WasmWindowManager::shouldClose() const noexcept {
    for (const auto& w : windows_) {
        if (w && !w->isOpen()) {
            return true;
        }
    }
    return false;
}

bool WasmWindowManager::shouldCloseAll() const noexcept {
    if (windows_.empty()) {
        return false;
    }
    for (const auto& w : windows_) {
        if (w && w->isOpen()) {
            return false;
        }
    }
    return true;
}

WindowAPI WasmWindowManager::getWindowAPI() const noexcept {
    return WindowAPI::eWasmWindow;
}

std::string WasmWindowManager::getPlatformInfo() const {
    return "WebAssembly / Emscripten";
}

bool WasmWindowManager::isFeatureSupported(const std::string& feature) const {
    if (feature == "multi_window") {
#ifdef __EMSCRIPTEN__
        return WasmWindow::detectVneShell();
#else
        return false;
#endif
    }
    return feature == "resize" || feature == "canvas" || feature == "fullscreen";
}

std::string WasmWindowManager::getProperties() const {
    return properties_;
}

void WasmWindowManager::setProperties(const std::string& properties) {
    properties_ = properties;
}

uint64_t WasmWindowManager::getCurrentTime() const noexcept {
#ifdef __EMSCRIPTEN__
    return static_cast<uint64_t>(emscripten_get_now());
#else
    using namespace std::chrono;
    return static_cast<uint64_t>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
#endif
}

void WasmWindowManager::sleep(uint32_t milliseconds) const noexcept {
#ifdef __EMSCRIPTEN__
    emscripten_sleep(milliseconds);
#else
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
#endif
}

double WasmWindowManager::getPlatformTime() const noexcept {
#ifdef __EMSCRIPTEN__
    return emscripten_get_now();
#else
    using namespace std::chrono;
    return duration<double>(steady_clock::now().time_since_epoch()).count();
#endif
}

void WasmWindowManager::registerGlobalCallbacks() {
#ifdef __EMSCRIPTEN__
    if (global_callbacks_registered_) {
        return;
    }
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, 1, &WasmWindowManager::GlobalKeyDownCallback);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, 1, &WasmWindowManager::GlobalKeyUpCallback);
    emscripten_set_visibilitychange_callback(this, 1, &WasmWindowManager::GlobalVisibilityCallback);
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, 1, &WasmWindowManager::GlobalResizeCallback);
    global_callbacks_registered_ = true;
#endif
}

void WasmWindowManager::unregisterGlobalCallbacks() {
#ifdef __EMSCRIPTEN__
    if (!global_callbacks_registered_) {
        return;
    }
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, 0, nullptr);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, 0, nullptr);
    emscripten_set_visibilitychange_callback(nullptr, 0, nullptr);
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, 0, nullptr);
    global_callbacks_registered_ = false;
#endif
}

#ifdef __EMSCRIPTEN__

EM_BOOL WasmWindowManager::GlobalKeyDownCallback(int /*event_type*/,
                                                 const EmscriptenKeyboardEvent* ev,
                                                 void* user_data) {
    auto* mgr = static_cast<WasmWindowManager*>(user_data);
    if (!mgr || !ev || !mgr->focused_) {
        return EM_FALSE;
    }
    auto* ww = dynamic_cast<WasmWindow*>(mgr->focused_.get());
    if (!ww) {
        return EM_FALSE;
    }
    const vne::events::KeyCode kc = mapEmscriptenKey(ev->code);
    const uint8_t mods = mapEmscriptenModifiers(ev->shiftKey, ev->ctrlKey, ev->altKey, ev->metaKey);
    ww->dispatchKeyDown(kc, mods, ev->repeat);
    return EM_TRUE;
}

EM_BOOL WasmWindowManager::GlobalKeyUpCallback(int /*event_type*/, const EmscriptenKeyboardEvent* ev, void* user_data) {
    auto* mgr = static_cast<WasmWindowManager*>(user_data);
    if (!mgr || !ev || !mgr->focused_) {
        return EM_FALSE;
    }
    auto* ww = dynamic_cast<WasmWindow*>(mgr->focused_.get());
    if (!ww) {
        return EM_FALSE;
    }
    const vne::events::KeyCode kc = mapEmscriptenKey(ev->code);
    const uint8_t mods = mapEmscriptenModifiers(ev->shiftKey, ev->ctrlKey, ev->altKey, ev->metaKey);
    ww->dispatchKeyUp(kc, mods);
    return EM_TRUE;
}

EM_BOOL WasmWindowManager::GlobalVisibilityCallback(int /*event_type*/,
                                                    const EmscriptenVisibilityChangeEvent* ev,
                                                    void* /*user_data*/) {
    if (!ev) {
        return EM_FALSE;
    }
    EventEmitter::applicationLifecycle(ev->hidden ? ApplicationLifecycle::ePause : ApplicationLifecycle::eResume);
    return EM_TRUE;
}

EM_BOOL WasmWindowManager::GlobalResizeCallback(int /*event_type*/,
                                                const EmscriptenUiEvent* /*event*/,
                                                void* user_data) {
    auto* mgr = static_cast<WasmWindowManager*>(user_data);
    if (!mgr || mgr->windows_.size() != 1U || !mgr->primary_) {
        return EM_FALSE;
    }
    auto* ww = dynamic_cast<WasmWindow*>(mgr->primary_.get());
    if (!ww || !ww->isPrimary()) {
        return EM_FALSE;
    }
    int width = 0;
    int height = 0;
    if (!WasmWindow::queryBrowserViewport(width, height)) {
        return EM_FALSE;
    }
    ww->applyViewportSize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    return EM_TRUE;
}

#endif  // __EMSCRIPTEN__

}  // namespace vne::xwin
