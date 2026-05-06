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

#include "android_window_manager.h"

#include "android_window.h"

#include <algorithm>
#include <chrono>
#include <thread>

namespace vne::xwin {

AndroidWindowManager_C::AndroidWindowManager_C() = default;

AndroidWindowManager_C::~AndroidWindowManager_C() {
    Shutdown();
}

void AndroidWindowManager_C::NotifyWindowEvent(IWindow* window, const WindowEventData& event) {
    if (callback_ && window) {
        callback_(window, event);
    }
}

bool AndroidWindowManager_C::Initialize() {
    initialized_ = true;
    return true;
}

void AndroidWindowManager_C::Shutdown() {
    DestroyAllWindows();
    initialized_ = false;
}

bool AndroidWindowManager_C::IsInitialized() const {
    return initialized_;
}

std::shared_ptr<IWindow> AndroidWindowManager_C::OpenWindow(const WindowDescriptor& descriptor) {
    if (!initialized_) {
        return nullptr;
    }
    auto w = std::make_shared<AndroidWindow_C>();
    w->Initialize(descriptor);
    if (!w->IsOpen()) {
        return nullptr;
    }
    windows_.push_back(w);
    if (!primary_) {
        primary_ = w;
    }
    focused_ = w;
    return w;
}

std::shared_ptr<IWindow> AndroidWindowManager_C::OpenWindow(const std::string& title, uint32_t width, uint32_t height) {
    WindowDescriptor d(title, width, height);
    return OpenWindow(d);
}

void AndroidWindowManager_C::RemoveWindow(std::shared_ptr<IWindow> window) {
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

void AndroidWindowManager_C::DestroyAllWindows() {
    for (auto& w : windows_) {
        if (w) {
            w->Close();
        }
    }
    windows_.clear();
    primary_.reset();
    focused_.reset();
}

size_t AndroidWindowManager_C::GetWindowCount() const {
    return windows_.size();
}

std::vector<std::shared_ptr<IWindow>> AndroidWindowManager_C::GetWindows() const {
    return windows_;
}

std::shared_ptr<IWindow> AndroidWindowManager_C::GetPrimaryWindow() const {
    return primary_;
}

std::shared_ptr<IWindow> AndroidWindowManager_C::GetFocusedWindow() const {
    return focused_;
}

void AndroidWindowManager_C::SetPrimaryWindow(std::shared_ptr<IWindow> window) {
    primary_ = std::move(window);
}

void AndroidWindowManager_C::FocusWindow(std::shared_ptr<IWindow> window) {
    focused_ = std::move(window);
}

void AndroidWindowManager_C::ProcessEvents() {}

void AndroidWindowManager_C::SetEventCallback(const WindowManagerEventCallback_T& callback) {
    callback_ = callback;
}

void AndroidWindowManager_C::setEventBridgeCallbacks(EventBridgeCallbacks callbacks) {
    event_bridge_callbacks_ = std::move(callbacks);
}

bool AndroidWindowManager_C::ShouldClose() const {
    for (const auto& w : windows_) {
        if (w && !w->IsOpen()) {
            return true;
        }
    }
    return false;
}

bool AndroidWindowManager_C::ShouldCloseAll() const {
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

WindowAPI AndroidWindowManager_C::GetWindowAPI() const {
    return WindowAPI::eAndroidSurfaceWindow;
}

std::string AndroidWindowManager_C::GetPlatformInfo() const {
    return "Android / ANativeWindow";
}

bool AndroidWindowManager_C::IsFeatureSupported(const std::string& feature) const {
    return feature == "native_window" || feature == "resize";
}

std::string AndroidWindowManager_C::GetProperties() const {
    return properties_;
}

void AndroidWindowManager_C::SetProperties(const std::string& properties) {
    properties_ = properties;
}

uint64_t AndroidWindowManager_C::GetCurrentTime() const {
    using namespace std::chrono;
    return static_cast<uint64_t>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}

void AndroidWindowManager_C::Sleep(uint32_t milliseconds) const {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

double AndroidWindowManager_C::GetPlatformTime() const {
    using namespace std::chrono;
    return duration<double>(steady_clock::now().time_since_epoch()).count();
}

}  // namespace vne::xwin
