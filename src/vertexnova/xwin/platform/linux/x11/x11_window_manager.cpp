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

X11WindowManager_C::X11WindowManager_C() = default;

X11WindowManager_C::~X11WindowManager_C() {
    Shutdown();
}

void X11WindowManager_C::NotifyWindowEvent(Window_I* window, const WindowEventData_C& event) {
    if (_callback && window) {
        _callback(window, event);
    }
}

bool X11WindowManager_C::Initialize() {
    _display = XOpenDisplay(nullptr);
    if (!_display) {
        return false;
    }
    _screen = DefaultScreen(_display);
    _root = RootWindow(_display, _screen);
#if defined(VNE_X11_HAS_XLIB_XCB)
    _xcb_connection = XGetXCBConnection(_display);
#endif
    _initialized = true;
    return true;
}

void X11WindowManager_C::Shutdown() {
    DestroyAllWindows();
    if (_display) {
        XCloseDisplay(_display);
        _display = nullptr;
    }
    _xcb_connection = nullptr;
    _initialized = false;
}

bool X11WindowManager_C::IsInitialized() const {
    return _initialized;
}

std::shared_ptr<Window_I> X11WindowManager_C::CreateWindow(const WindowDescriptor_C& descriptor) {
    if (!_initialized || !_display) {
        return nullptr;
    }
    auto w = std::make_shared<X11Window_C>();
    w->SetEventOwner(this);
    w->SetDisplay(_display, _screen, _root, _xcb_connection);
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

std::shared_ptr<Window_I> X11WindowManager_C::CreateWindow(const std::string& title, uint32_t width, uint32_t height) {
    WindowDescriptor_C d(title, width, height);
    return CreateWindow(d);
}

void X11WindowManager_C::DestroyWindow(std::shared_ptr<Window_I> window) {
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

void X11WindowManager_C::DestroyAllWindows() {
    for (auto& w : _windows) {
        if (w) {
            w->Close();
        }
    }
    _windows.clear();
    _primary.reset();
    _focused.reset();
}

size_t X11WindowManager_C::GetWindowCount() const {
    return _windows.size();
}

std::vector<std::shared_ptr<Window_I>> X11WindowManager_C::GetWindows() const {
    return _windows;
}

std::shared_ptr<Window_I> X11WindowManager_C::GetPrimaryWindow() const {
    return _primary;
}

std::shared_ptr<Window_I> X11WindowManager_C::GetFocusedWindow() const {
    return _focused;
}

void X11WindowManager_C::SetPrimaryWindow(std::shared_ptr<Window_I> window) {
    _primary = std::move(window);
}

void X11WindowManager_C::FocusWindow(std::shared_ptr<Window_I> window) {
    _focused = std::move(window);
}

void X11WindowManager_C::ProcessEvents() {
    for (auto& w : _windows) {
        if (w) {
            w->PollEvents();
        }
    }
}

void X11WindowManager_C::SetEventCallback(const WindowManagerEventCallback_T& callback) {
    _callback = callback;
}

void X11WindowManager_C::setEventBridgeCallbacks(EventBridgeCallbacks callbacks) {
    _event_bridge_callbacks = std::move(callbacks);
}

bool X11WindowManager_C::ShouldClose() const {
    for (const auto& w : _windows) {
        if (w && !w->IsOpen()) {
            return true;
        }
    }
    return false;
}

bool X11WindowManager_C::ShouldCloseAll() const {
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

WindowAPI_TP X11WindowManager_C::GetWindowAPI() const {
    return WindowAPI_TP::X11_WINDOW;
}

std::string X11WindowManager_C::GetPlatformInfo() const {
    return "Linux X11 (Xlib)";
}

bool X11WindowManager_C::IsFeatureSupported(const std::string& feature) const {
    return feature == "resize" || feature == "decorated";
}

std::string X11WindowManager_C::GetProperties() const {
    return _properties;
}

void X11WindowManager_C::SetProperties(const std::string& properties) {
    _properties = properties;
}

uint64_t X11WindowManager_C::GetCurrentTime() const {
    using namespace std::chrono;
    return static_cast<uint64_t>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}

void X11WindowManager_C::Sleep(uint32_t milliseconds) const {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

double X11WindowManager_C::GetPlatformTime() const {
    using namespace std::chrono;
    return duration<double>(steady_clock::now().time_since_epoch()).count();
}

}  // namespace vne::xwin
