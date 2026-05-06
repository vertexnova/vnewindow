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

#include "x11_window_manager.h"

#include "x11_window.h"

#include <algorithm>
#include <chrono>
#include <thread>
#if __has_include(<X11/Xlib-xcb.h>)
#include <X11/Xlib-xcb.h>
#define VNE_X11_HAS_XLIB_XCB 1
#endif

namespace vne::xwin {

X11WindowManager::X11WindowManager() = default;

X11WindowManager::~X11WindowManager() {
    Shutdown();
}

void X11WindowManager::notifyWindowEvent(IWindow* window, const WindowEventData& event) {
    if (callback_ && window) {
        callback_(window, event);
    }
}

bool X11WindowManager::Initialize() {
    display_ = XOpenDisplay(nullptr);
    if (!display_) {
        return false;
    }
    screen_ = DefaultScreen(display_);
    root_ = RootWindow(display_, screen_);
#if defined(VNE_X11_HAS_XLIB_XCB)
    xcb_connection_ = XGetXCBConnection(display_);
#endif
    initialized_ = true;
    return true;
}

void X11WindowManager::Shutdown() {
    DestroyAllWindows();
    if (display_) {
        XCloseDisplay(display_);
        display_ = nullptr;
    }
    xcb_connection_ = nullptr;
    initialized_ = false;
}

bool X11WindowManager::IsInitialized() const noexcept {
    return initialized_;
}

std::shared_ptr<IWindow> X11WindowManager::OpenWindow(const WindowDescriptor& descriptor) {
    if (!initialized_ || !display_) {
        return nullptr;
    }
    auto w = std::make_shared<X11Window>();
    w->setEventOwner(this);
    w->setDisplay(display_, screen_, root_, xcb_connection_);
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

std::shared_ptr<IWindow> X11WindowManager::OpenWindow(const std::string& title, uint32_t width, uint32_t height) {
    WindowDescriptor descriptor(title, width, height);
    return OpenWindow(descriptor);
}

void X11WindowManager::RemoveWindow(std::shared_ptr<IWindow> window) {
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

void X11WindowManager::DestroyAllWindows() {
    for (auto& w : windows_) {
        if (w) {
            w->Close();
        }
    }
    windows_.clear();
    primary_.reset();
    focused_.reset();
}

size_t X11WindowManager::GetWindowCount() const noexcept {
    return windows_.size();
}

std::vector<std::shared_ptr<IWindow>> X11WindowManager::GetWindows() const {
    return windows_;
}

std::shared_ptr<IWindow> X11WindowManager::GetPrimaryWindow() const noexcept {
    return primary_;
}

std::shared_ptr<IWindow> X11WindowManager::GetFocusedWindow() const noexcept {
    return focused_;
}

void X11WindowManager::SetPrimaryWindow(std::shared_ptr<IWindow> window) {
    primary_ = std::move(window);
}

void X11WindowManager::FocusWindow(std::shared_ptr<IWindow> window) {
    focused_ = std::move(window);
}

void X11WindowManager::ProcessEvents() {
    for (auto& w : windows_) {
        if (w) {
            w->PollEvents();
        }
    }
}

void X11WindowManager::SetEventCallback(const WindowManagerEventCallback_T& callback) {
    callback_ = callback;
}

void X11WindowManager::setEventBridgeCallbacks(EventBridgeCallbacks callbacks) {
    event_bridge_callbacks_ = std::move(callbacks);
}

bool X11WindowManager::ShouldClose() const noexcept {
    for (const auto& w : windows_) {
        if (w && !w->IsOpen()) {
            return true;
        }
    }
    return false;
}

bool X11WindowManager::ShouldCloseAll() const noexcept {
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

WindowAPI X11WindowManager::GetWindowAPI() const noexcept {
    return WindowAPI::eX11Window;
}

std::string X11WindowManager::GetPlatformInfo() const {
    return "Linux X11 (Xlib)";
}

bool X11WindowManager::IsFeatureSupported(const std::string& feature) const {
    return feature == "resize" || feature == "decorated";
}

std::string X11WindowManager::GetProperties() const {
    return properties_;
}

void X11WindowManager::SetProperties(const std::string& properties) {
    properties_ = properties;
}

uint64_t X11WindowManager::GetCurrentTime() const noexcept {
    using namespace std::chrono;
    return static_cast<uint64_t>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}

void X11WindowManager::Sleep(uint32_t milliseconds) const noexcept {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

double X11WindowManager::GetPlatformTime() const noexcept {
    using namespace std::chrono;
    return duration<double>(steady_clock::now().time_since_epoch()).count();
}

}  // namespace vne::xwin
