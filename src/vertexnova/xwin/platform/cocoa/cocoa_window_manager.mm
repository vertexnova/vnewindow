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

void CocoaWindowManager_C::NotifyWindowEvent(Window_I* window, const WindowEventData_C& event) {
    if (_callback && window) {
        _callback(window, event);
    }
}

bool CocoaWindowManager_C::Initialize() {
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    _initialized = true;
    return true;
}

void CocoaWindowManager_C::Shutdown() {
    DestroyAllWindows();
    _initialized = false;
}

bool CocoaWindowManager_C::IsInitialized() const {
    return _initialized;
}

std::shared_ptr<Window_I> CocoaWindowManager_C::CreateWindow(const WindowDescriptor_C& descriptor) {
    if (!_initialized) {
        return nullptr;
    }
    auto w = std::make_shared<CocoaWindow_C>();
    w->SetEventOwner(this);
    w->Initialize(descriptor);
    if (!w->IsOpen()) {
        return nullptr;
    }
    _windows.push_back(w);
    if (!_primary) {
        _primary = w;
    }
    _focused = w;
    return w;
}

std::shared_ptr<Window_I> CocoaWindowManager_C::CreateWindow(const std::string& title,
                                                             uint32_t width,
                                                             uint32_t height) {
    WindowDescriptor_C d(title, width, height);
    return CreateWindow(d);
}

void CocoaWindowManager_C::DestroyWindow(std::shared_ptr<Window_I> window) {
    if (!window) {
        return;
    }
    window->Close();
    auto it = std::find(_windows.begin(), _windows.end(), window);
    if (it != _windows.end()) {
        _windows.erase(it);
    }
    if (_primary == window) {
        _primary = _windows.empty() ? nullptr : _windows.front();
    }
    if (_focused == window) {
        _focused = _primary;
    }
}

void CocoaWindowManager_C::DestroyAllWindows() {
    for (auto& w : _windows) {
        if (w) {
            w->Close();
        }
    }
    _windows.clear();
    _primary.reset();
    _focused.reset();
}

size_t CocoaWindowManager_C::GetWindowCount() const {
    return _windows.size();
}

std::vector<std::shared_ptr<Window_I>> CocoaWindowManager_C::GetWindows() const {
    return _windows;
}

std::shared_ptr<Window_I> CocoaWindowManager_C::GetPrimaryWindow() const {
    return _primary;
}

std::shared_ptr<Window_I> CocoaWindowManager_C::GetFocusedWindow() const {
    return _focused;
}

void CocoaWindowManager_C::SetPrimaryWindow(std::shared_ptr<Window_I> window) {
    _primary = std::move(window);
}

void CocoaWindowManager_C::FocusWindow(std::shared_ptr<Window_I> window) {
    _focused = std::move(window);
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
    _callback = callback;
}

void CocoaWindowManager_C::setEventBridgeCallbacks(EventBridgeCallbacks callbacks) {
    _event_bridge_callbacks = std::move(callbacks);
}

bool CocoaWindowManager_C::ShouldClose() const {
    for (const auto& w : _windows) {
        if (w && !w->IsOpen()) {
            return true;
        }
    }
    return false;
}

bool CocoaWindowManager_C::ShouldCloseAll() const {
    if (_windows.empty()) {
        return false;
    }
    for (const auto& w : _windows) {
        if (w && w->IsOpen()) {
            return false;
        }
    }
    return true;
}

WindowAPI_TP CocoaWindowManager_C::GetWindowAPI() const {
    return WindowAPI_TP::COCOA_WINDOW;
}

std::string CocoaWindowManager_C::GetPlatformInfo() const {
    return "macOS / AppKit";
}

bool CocoaWindowManager_C::IsFeatureSupported(const std::string& feature) const {
    return feature == "resize" || feature == "dpi" || feature == "fullscreen";
}

std::string CocoaWindowManager_C::GetProperties() const {
    return _properties;
}

void CocoaWindowManager_C::SetProperties(const std::string& properties) {
    _properties = properties;
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
