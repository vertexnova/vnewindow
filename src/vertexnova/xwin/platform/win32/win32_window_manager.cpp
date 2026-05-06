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

#include "win32_window_manager.h"

#include "win32_window.h"

#include <algorithm>
#include <chrono>
#include <thread>

namespace vne::xwin {

Win32WindowManager_C::Win32WindowManager_C() = default;

Win32WindowManager_C::~Win32WindowManager_C() {
    Shutdown();
}

void Win32WindowManager_C::NotifyWindowEvent(IWindow* window, const WindowEventData& event) {
    if (callback_ && window) {
        callback_(window, event);
    }
}

bool Win32WindowManager_C::Initialize() {
    initialized_ = true;
    return true;
}

void Win32WindowManager_C::Shutdown() {
    DestroyAllWindows();
    initialized_ = false;
}

bool Win32WindowManager_C::IsInitialized() const {
    return initialized_;
}

std::shared_ptr<IWindow> Win32WindowManager_C::OpenWindow(const WindowDescriptor& descriptor) {
    if (!initialized_) {
        return nullptr;
    }
    auto w = std::make_shared<Win32Window_C>();
    w->SetEventOwner(this);
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

std::shared_ptr<IWindow> Win32WindowManager_C::OpenWindow(const std::string& title, uint32_t width, uint32_t height) {
    WindowDescriptor d(title, width, height);
    return OpenWindow(d);
}

void Win32WindowManager_C::RemoveWindow(std::shared_ptr<IWindow> window) {
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

void Win32WindowManager_C::DestroyAllWindows() {
    for (auto& w : windows_) {
        if (w) {
            w->Close();
        }
    }
    windows_.clear();
    primary_.reset();
    focused_.reset();
}

size_t Win32WindowManager_C::GetWindowCount() const {
    return windows_.size();
}

std::vector<std::shared_ptr<IWindow>> Win32WindowManager_C::GetWindows() const {
    return windows_;
}

std::shared_ptr<IWindow> Win32WindowManager_C::GetPrimaryWindow() const {
    return primary_;
}

std::shared_ptr<IWindow> Win32WindowManager_C::GetFocusedWindow() const {
    return focused_;
}

void Win32WindowManager_C::SetPrimaryWindow(std::shared_ptr<IWindow> window) {
    primary_ = std::move(window);
}

void Win32WindowManager_C::FocusWindow(std::shared_ptr<IWindow> window) {
    focused_ = std::move(window);
}

void Win32WindowManager_C::ProcessEvents() {
    for (auto& w : windows_) {
        if (w) {
            w->PollEvents();
        }
    }
}

void Win32WindowManager_C::SetEventCallback(const WindowManagerEventCallback_T& callback) {
    callback_ = callback;
}

void Win32WindowManager_C::setEventBridgeCallbacks(EventBridgeCallbacks callbacks) {
    event_bridge_callbacks_ = std::move(callbacks);
}

bool Win32WindowManager_C::ShouldClose() const {
    for (const auto& w : windows_) {
        if (w && !w->IsOpen()) {
            return true;
        }
    }
    return false;
}

bool Win32WindowManager_C::ShouldCloseAll() const {
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

WindowAPI Win32WindowManager_C::GetWindowAPI() const {
    return WindowAPI::eWin32Window;
}

std::string Win32WindowManager_C::GetPlatformInfo() const {
    return "Microsoft Windows (Win32)";
}

bool Win32WindowManager_C::IsFeatureSupported(const std::string& feature) const {
    return feature == "resize" || feature == "dpi" || feature == "fullscreen";
}

std::string Win32WindowManager_C::GetProperties() const {
    return properties_;
}

void Win32WindowManager_C::SetProperties(const std::string& properties) {
    properties_ = properties;
}

uint64_t Win32WindowManager_C::GetCurrentTime() const {
    using namespace std::chrono;
    return static_cast<uint64_t>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}

void Win32WindowManager_C::Sleep(uint32_t milliseconds) const {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

double Win32WindowManager_C::GetPlatformTime() const {
    using namespace std::chrono;
    return duration<double>(steady_clock::now().time_since_epoch()).count();
}

}  // namespace vne::xwin
