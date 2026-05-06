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

#include "cocoa_window_manager.h"

#include "cocoa_window.h"

#import <Cocoa/Cocoa.h>

#include <algorithm>
#include <chrono>
#include <thread>

namespace vne::xwin {

CocoaWindowManager_C::CocoaWindowManager_C() = default;

CocoaWindowManager_C::~CocoaWindowManager_C() {
    Shutdown();
}

void CocoaWindowManager_C::NotifyWindowEvent(IWindow* window, const WindowEventData& event) {
    if (callback_ && window) {
        callback_(window, event);
    }
}

bool CocoaWindowManager_C::Initialize() {
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    initialized_ = true;
    return true;
}

void CocoaWindowManager_C::Shutdown() {
    DestroyAllWindows();
    initialized_ = false;
}

bool CocoaWindowManager_C::IsInitialized() const {
    return initialized_;
}

std::shared_ptr<IWindow> CocoaWindowManager_C::OpenWindow(const WindowDescriptor& descriptor) {
    if (!initialized_) {
        return nullptr;
    }
    auto w = std::make_shared<CocoaWindow_C>();
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

std::shared_ptr<IWindow> CocoaWindowManager_C::OpenWindow(const std::string& title, uint32_t width, uint32_t height) {
    WindowDescriptor d(title, width, height);
    return OpenWindow(d);
}

void CocoaWindowManager_C::RemoveWindow(std::shared_ptr<IWindow> window) {
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

void CocoaWindowManager_C::DestroyAllWindows() {
    for (auto& w : windows_) {
        if (w) {
            w->Close();
        }
    }
    windows_.clear();
    primary_.reset();
    focused_.reset();
}

size_t CocoaWindowManager_C::GetWindowCount() const {
    return windows_.size();
}

std::vector<std::shared_ptr<IWindow>> CocoaWindowManager_C::GetWindows() const {
    return windows_;
}

std::shared_ptr<IWindow> CocoaWindowManager_C::GetPrimaryWindow() const {
    return primary_;
}

std::shared_ptr<IWindow> CocoaWindowManager_C::GetFocusedWindow() const {
    return focused_;
}

void CocoaWindowManager_C::SetPrimaryWindow(std::shared_ptr<IWindow> window) {
    primary_ = std::move(window);
}

void CocoaWindowManager_C::FocusWindow(std::shared_ptr<IWindow> window) {
    focused_ = std::move(window);
}

void CocoaWindowManager_C::ProcessEvents() {
    for (;;) {
        NSEvent* ev = [NSApp nextEventMatchingMask:NSEventMaskAny
                                         untilDate:[NSDate distantPast]
                                            inMode:NSDefaultRunLoopMode
                                           dequeue:YES];
        if (!ev) {
            break;
        }
        [NSApp sendEvent:ev];
    }
}

void CocoaWindowManager_C::SetEventCallback(const WindowManagerEventCallback_T& callback) {
    callback_ = callback;
}

void CocoaWindowManager_C::setEventBridgeCallbacks(EventBridgeCallbacks callbacks) {
    event_bridge_callbacks_ = std::move(callbacks);
}

bool CocoaWindowManager_C::ShouldClose() const {
    for (const auto& w : windows_) {
        if (w && !w->IsOpen()) {
            return true;
        }
    }
    return false;
}

bool CocoaWindowManager_C::ShouldCloseAll() const {
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

WindowAPI CocoaWindowManager_C::GetWindowAPI() const {
    return WindowAPI::eCocoaWindow;
}

std::string CocoaWindowManager_C::GetPlatformInfo() const {
    return "macOS / AppKit";
}

bool CocoaWindowManager_C::IsFeatureSupported(const std::string& feature) const {
    return feature == "resize" || feature == "dpi" || feature == "fullscreen";
}

std::string CocoaWindowManager_C::GetProperties() const {
    return properties_;
}

void CocoaWindowManager_C::SetProperties(const std::string& properties) {
    properties_ = properties;
}

uint64_t CocoaWindowManager_C::GetCurrentTime() const {
    using namespace std::chrono;
    return static_cast<uint64_t>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}

void CocoaWindowManager_C::Sleep(uint32_t milliseconds) const {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

double CocoaWindowManager_C::GetPlatformTime() const {
    using namespace std::chrono;
    return duration<double>(steady_clock::now().time_since_epoch()).count();
}

}  // namespace vne::xwin
