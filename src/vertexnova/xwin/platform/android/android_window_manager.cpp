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

AndroidWindowManager::AndroidWindowManager() = default;

AndroidWindowManager::~AndroidWindowManager() {
    Shutdown();
}

void AndroidWindowManager::notifyWindowEvent(IWindow* window, const WindowEventData& event) {
    if (callback_ && window) {
        callback_(window, event);
    }
}

bool AndroidWindowManager::Initialize() {
    initialized_ = true;
    return true;
}

void AndroidWindowManager::Shutdown() {
    DestroyAllWindows();
    initialized_ = false;
}

bool AndroidWindowManager::IsInitialized() const noexcept {
    return initialized_;
}

std::shared_ptr<IWindow> AndroidWindowManager::OpenWindow(const WindowDescriptor& descriptor) {
    if (!initialized_) {
        return nullptr;
    }
    auto w = std::make_shared<AndroidWindow>();
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

std::shared_ptr<IWindow> AndroidWindowManager::OpenWindow(const std::string& title, uint32_t width, uint32_t height) {
    WindowDescriptor descriptor(title, width, height);
    return OpenWindow(descriptor);
}

void AndroidWindowManager::RemoveWindow(std::shared_ptr<IWindow> window) {
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

void AndroidWindowManager::DestroyAllWindows() {
    for (auto& w : windows_) {
        if (w) {
            w->Close();
        }
    }
    windows_.clear();
    primary_.reset();
    focused_.reset();
}

size_t AndroidWindowManager::GetWindowCount() const noexcept {
    return windows_.size();
}

std::vector<std::shared_ptr<IWindow>> AndroidWindowManager::GetWindows() const {
    return windows_;
}

std::shared_ptr<IWindow> AndroidWindowManager::GetPrimaryWindow() const noexcept {
    return primary_;
}

std::shared_ptr<IWindow> AndroidWindowManager::GetFocusedWindow() const noexcept {
    return focused_;
}

void AndroidWindowManager::SetPrimaryWindow(std::shared_ptr<IWindow> window) {
    primary_ = std::move(window);
}

void AndroidWindowManager::FocusWindow(std::shared_ptr<IWindow> window) {
    focused_ = std::move(window);
}

void AndroidWindowManager::ProcessEvents() {}

void AndroidWindowManager::SetEventCallback(const WindowManagerEventCallback_T& callback) {
    callback_ = callback;
}

void AndroidWindowManager::setEventBridgeCallbacks(EventBridgeCallbacks callbacks) {
    event_bridge_callbacks_ = std::move(callbacks);
}

bool AndroidWindowManager::ShouldClose() const noexcept {
    for (const auto& w : windows_) {
        if (w && !w->IsOpen()) {
            return true;
        }
    }
    return false;
}

bool AndroidWindowManager::ShouldCloseAll() const noexcept {
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

WindowAPI AndroidWindowManager::GetWindowAPI() const noexcept {
    return WindowAPI::eAndroidSurfaceWindow;
}

std::string AndroidWindowManager::GetPlatformInfo() const {
    return "Android / ANativeWindow";
}

bool AndroidWindowManager::IsFeatureSupported(const std::string& feature) const {
    return feature == "native_window" || feature == "resize";
}

std::string AndroidWindowManager::GetProperties() const {
    return properties_;
}

void AndroidWindowManager::SetProperties(const std::string& properties) {
    properties_ = properties;
}

uint64_t AndroidWindowManager::GetCurrentTime() const noexcept {
    using namespace std::chrono;
    return static_cast<uint64_t>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}

void AndroidWindowManager::Sleep(uint32_t milliseconds) const noexcept {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

double AndroidWindowManager::GetPlatformTime() const noexcept {
    using namespace std::chrono;
    return duration<double>(steady_clock::now().time_since_epoch()).count();
}

}  // namespace vne::xwin
