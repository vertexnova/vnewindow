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

/* <windows.h> defines GetCurrentTime as a macro (GetTickCount); break it before defining our method. */
#ifdef GetCurrentTime
#undef GetCurrentTime
#endif

namespace vne::xwin {

Win32WindowManager::Win32WindowManager() = default;

Win32WindowManager::~Win32WindowManager() {
    Shutdown();
}

void Win32WindowManager::notifyWindowEvent(IWindow* window, const WindowEventData& event) {
    if (callback_ && window) {
        callback_(window, event);
    }
}

bool Win32WindowManager::Initialize() {
    initialized_ = true;
    return true;
}

void Win32WindowManager::Shutdown() {
    DestroyAllWindows();
    initialized_ = false;
}

bool Win32WindowManager::IsInitialized() const noexcept {
    return initialized_;
}

std::shared_ptr<IWindow> Win32WindowManager::OpenWindow(const WindowDescriptor& descriptor) {
    if (!initialized_) {
        return nullptr;
    }
    auto w = std::make_shared<Win32Window>();
    w->setEventOwner(this);
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

std::shared_ptr<IWindow> Win32WindowManager::OpenWindow(const std::string& title, uint32_t width, uint32_t height) {
    WindowDescriptor descriptor(title, width, height);
    return OpenWindow(descriptor);
}

void Win32WindowManager::RemoveWindow(std::shared_ptr<IWindow> window) {
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

void Win32WindowManager::DestroyAllWindows() {
    for (auto& w : windows_) {
        if (w) {
            w->Close();
        }
    }
    windows_.clear();
    primary_.reset();
    focused_.reset();
}

size_t Win32WindowManager::GetWindowCount() const noexcept {
    return windows_.size();
}

std::vector<std::shared_ptr<IWindow>> Win32WindowManager::GetWindows() const {
    return windows_;
}

std::shared_ptr<IWindow> Win32WindowManager::GetPrimaryWindow() const noexcept {
    return primary_;
}

std::shared_ptr<IWindow> Win32WindowManager::GetFocusedWindow() const noexcept {
    return focused_;
}

void Win32WindowManager::SetPrimaryWindow(std::shared_ptr<IWindow> window) {
    primary_ = std::move(window);
}

void Win32WindowManager::FocusWindow(std::shared_ptr<IWindow> window) {
    focused_ = std::move(window);
}

void Win32WindowManager::ProcessEvents() {
    for (auto& w : windows_) {
        if (w) {
            w->PollEvents();
        }
    }
}

void Win32WindowManager::SetEventCallback(const WindowManagerEventCallback_T& callback) {
    callback_ = callback;
}

void Win32WindowManager::setEventBridgeCallbacks(EventBridgeCallbacks callbacks) {
    event_bridge_callbacks_ = std::move(callbacks);
}

bool Win32WindowManager::ShouldClose() const noexcept {
    for (const auto& w : windows_) {
        if (w && !w->IsOpen()) {
            return true;
        }
    }
    return false;
}

bool Win32WindowManager::ShouldCloseAll() const noexcept {
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

WindowAPI Win32WindowManager::GetWindowAPI() const noexcept {
    return WindowAPI::eWin32Window;
}

std::string Win32WindowManager::GetPlatformInfo() const {
    return "Microsoft Windows (Win32)";
}

bool Win32WindowManager::IsFeatureSupported(const std::string& feature) const {
    return feature == "resize" || feature == "dpi" || feature == "fullscreen";
}

std::string Win32WindowManager::GetProperties() const {
    return properties_;
}

void Win32WindowManager::SetProperties(const std::string& properties) {
    properties_ = properties;
}

uint64_t Win32WindowManager::GetCurrentTime() const noexcept {
    using namespace std::chrono;
    return static_cast<uint64_t>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}

void Win32WindowManager::Sleep(uint32_t milliseconds) const noexcept {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

double Win32WindowManager::GetPlatformTime() const noexcept {
    using namespace std::chrono;
    return duration<double>(steady_clock::now().time_since_epoch()).count();
}

}  // namespace vne::xwin
