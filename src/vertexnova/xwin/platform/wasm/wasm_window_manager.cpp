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
    Shutdown();
}

void WasmWindowManager::notifyWindowEvent(IWindow* window, const WindowEventData& event) {
    if (callback_ && window) {
        callback_(window, event);
    }
}

bool WasmWindowManager::Initialize() {
    initialized_ = true;
    return true;
}

void WasmWindowManager::Shutdown() {
    DestroyAllWindows();
    initialized_ = false;
}

bool WasmWindowManager::IsInitialized() const noexcept {
    return initialized_;
}

std::shared_ptr<IWindow> WasmWindowManager::OpenWindow(const WindowDescriptor& descriptor) {
    if (!initialized_) {
        return nullptr;
    }
    auto w = std::make_shared<WasmWindow>();
    w->setEventOwner(this);
    w->Initialize(descriptor);
    windows_.push_back(w);
    if (!primary_) {
        primary_ = w;
    }
    focused_ = w;
    return w;
}

std::shared_ptr<IWindow> WasmWindowManager::OpenWindow(const std::string& title, uint32_t width, uint32_t height) {
    WindowDescriptor descriptor(title, width, height);
    return OpenWindow(descriptor);
}

void WasmWindowManager::RemoveWindow(std::shared_ptr<IWindow> window) {
    if (!window) {
        return;
    }
    window->Close();
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

void WasmWindowManager::DestroyAllWindows() {
    for (auto& w : windows_) {
        if (w) {
            w->Close();
        }
    }
    windows_.clear();
    primary_.reset();
    focused_.reset();
}

size_t WasmWindowManager::GetWindowCount() const noexcept {
    return windows_.size();
}

std::vector<std::shared_ptr<IWindow>> WasmWindowManager::GetWindows() const {
    return windows_;
}

std::shared_ptr<IWindow> WasmWindowManager::GetPrimaryWindow() const noexcept {
    return primary_;
}

std::shared_ptr<IWindow> WasmWindowManager::GetFocusedWindow() const noexcept {
    return focused_;
}

void WasmWindowManager::SetPrimaryWindow(std::shared_ptr<IWindow> window) {
    primary_ = std::move(window);
}

void WasmWindowManager::FocusWindow(std::shared_ptr<IWindow> window) {
    focused_ = std::move(window);
}

void WasmWindowManager::ProcessEvents() {}

void WasmWindowManager::SetEventCallback(const WindowManagerEventCallback_T& callback) {
    callback_ = callback;
}

void WasmWindowManager::setEventBridgeCallbacks(EventBridgeCallbacks callbacks) {
    event_bridge_callbacks_ = std::move(callbacks);
}

bool WasmWindowManager::ShouldClose() const {
    for (const auto& w : windows_) {
        if (w && !w->IsOpen()) {
            return true;
        }
    }
    return false;
}

bool WasmWindowManager::ShouldCloseAll() const noexcept {
    if (windows_.empty()) {
        return false;
    }
    for (const auto& w : windows_) {
        if (w && w->IsOpen()) {
            return false;
        }
    }
    return true;
}

WindowAPI WasmWindowManager::GetWindowAPI() const noexcept {
    return WindowAPI::eWasmWindow;
}

std::string WasmWindowManager::GetPlatformInfo() const {
    return "WebAssembly / Emscripten";
}

bool WasmWindowManager::IsFeatureSupported(const std::string& feature) const {
    return feature == "resize" || feature == "canvas" || feature == "fullscreen";
}

std::string WasmWindowManager::GetProperties() const {
    return properties_;
}

void WasmWindowManager::SetProperties(const std::string& properties) {
    properties_ = properties;
}

uint64_t WasmWindowManager::GetCurrentTime() const noexcept {
#ifdef __EMSCRIPTEN__
    return static_cast<uint64_t>(emscripten_get_now());
#else
    using namespace std::chrono;
    return static_cast<uint64_t>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
#endif
}

void WasmWindowManager::Sleep(uint32_t milliseconds) const noexcept {
#ifdef __EMSCRIPTEN__
    emscripten_sleep(milliseconds);
#else
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
#endif
}

double WasmWindowManager::GetPlatformTime() const noexcept {
#ifdef __EMSCRIPTEN__
    return emscripten_get_now();
#else
    using namespace std::chrono;
    return duration<double>(steady_clock::now().time_since_epoch()).count();
#endif
}

}  // namespace vne::xwin
