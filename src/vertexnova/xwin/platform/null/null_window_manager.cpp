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

#include "null_window_manager.h"

#include "null_window.h"

#include <algorithm>
#include <thread>

namespace vne::xwin {

NullWindowManager_C::~NullWindowManager_C() {
    Shutdown();
}

bool NullWindowManager_C::Initialize() {
    initialized_ = true;
    return true;
}

void NullWindowManager_C::Shutdown() {
    DestroyAllWindows();
    initialized_ = false;
}

bool NullWindowManager_C::IsInitialized() const {
    return initialized_;
}

std::shared_ptr<IWindow> NullWindowManager_C::CreateWindow(const WindowDescriptor& descriptor) {
    if (!initialized_) {
        return nullptr;
    }
    auto w = std::make_shared<NullWindow_C>();
    w->Initialize(descriptor);
    windows_.push_back(w);
    if (!primary_) {
        primary_ = w;
    }
    focused_ = w;
    return w;
}

std::shared_ptr<IWindow> NullWindowManager_C::CreateWindow(const std::string& title, uint32_t width, uint32_t height) {
    WindowDescriptor d(title, width, height);
    return CreateWindow(d);
}

void NullWindowManager_C::DestroyWindow(std::shared_ptr<IWindow> window) {
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

void NullWindowManager_C::DestroyAllWindows() {
    for (auto& w : windows_) {
        if (w) {
            w->Close();
        }
    }
    windows_.clear();
    primary_.reset();
    focused_.reset();
}

size_t NullWindowManager_C::GetWindowCount() const {
    return windows_.size();
}

std::vector<std::shared_ptr<IWindow>> NullWindowManager_C::GetWindows() const {
    return windows_;
}

std::shared_ptr<IWindow> NullWindowManager_C::GetPrimaryWindow() const {
    return primary_;
}

std::shared_ptr<IWindow> NullWindowManager_C::GetFocusedWindow() const {
    return focused_;
}

void NullWindowManager_C::SetPrimaryWindow(std::shared_ptr<IWindow> window) {
    primary_ = std::move(window);
}

void NullWindowManager_C::FocusWindow(std::shared_ptr<IWindow> window) {
    focused_ = std::move(window);
}

void NullWindowManager_C::ProcessEvents() {}

void NullWindowManager_C::SetEventCallback(const WindowManagerEventCallback_T& callback) {
    callback_ = callback;
}

void NullWindowManager_C::setEventBridgeCallbacks(EventBridgeCallbacks callbacks) {
    event_bridge_callbacks_ = std::move(callbacks);
}

bool NullWindowManager_C::ShouldClose() const {
    for (const auto& w : windows_) {
        if (w && !w->IsOpen()) {
            return true;
        }
    }
    return false;
}

bool NullWindowManager_C::ShouldCloseAll() const {
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

WindowAPI NullWindowManager_C::GetWindowAPI() const {
    return WindowAPI::eNullWindow;
}

std::string NullWindowManager_C::GetPlatformInfo() const {
    return "NullWindowManager (headless / tests)";
}

bool NullWindowManager_C::IsFeatureSupported(const std::string& /*feature*/) const {
    return false;
}

std::string NullWindowManager_C::GetProperties() const {
    return properties_;
}

void NullWindowManager_C::SetProperties(const std::string& properties) {
    properties_ = properties;
}

uint64_t NullWindowManager_C::GetCurrentTime() const {
    using namespace std::chrono;
    return static_cast<uint64_t>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}

void NullWindowManager_C::Sleep(uint32_t milliseconds) const {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

double NullWindowManager_C::GetPlatformTime() const {
    using namespace std::chrono;
    return duration<double>(steady_clock::now().time_since_epoch()).count();
}

}  // namespace vne::xwin
