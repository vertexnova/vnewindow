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

#include "wasm_window.h"

#include <algorithm>
#include <chrono>
#include <thread>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

namespace vne::xwin {

WasmWindowManager::WasmWindowManager() = default;

WasmWindowManager::~WasmWindowManager() {
    shutdown();
}

void WasmWindowManager::notifyWindowEvent(IWindow* window, const WindowEventData& event) {
    if (callback_ && window) {
        callback_(window, event);
    }
}

bool WasmWindowManager::initialize() {
    initialized_ = true;
    return true;
}

void WasmWindowManager::shutdown() {
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
    auto w = std::make_shared<WasmWindow>();
    w->setEventOwner(this);
    w->initialize(descriptor);
    windows_.push_back(w);
    if (!primary_) {
        primary_ = w;
    }
    focused_ = w;
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
    }
}

void WasmWindowManager::destroyAllWindows() {
    for (auto& w : windows_) {
        if (w) {
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
    focused_ = std::move(window);
}

void WasmWindowManager::processEvents() {}

void WasmWindowManager::setEventCallback(const WindowManagerEventCallback_T& callback) {
    callback_ = callback;
}

void WasmWindowManager::setEventBridgeCallbacks(EventBridgeCallbacks callbacks) {
    event_bridge_callbacks_ = std::move(callbacks);
}

bool WasmWindowManager::shouldClose() const {
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

}  // namespace vne::xwin
