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

#include "uikit_window_manager.h"

#include "uikit_window.h"

#import <UIKit/UIKit.h>

#include <algorithm>
#include <chrono>
#include <thread>

namespace vne::xwin {

UIKitWindowManager_C::UIKitWindowManager_C() = default;

UIKitWindowManager_C::~UIKitWindowManager_C() {
    Shutdown();
}

void UIKitWindowManager_C::NotifyWindowEvent(Window_I* window, const WindowEventData_C& event) {
    if (_callback && window) {
        _callback(window, event);
    }
}

bool UIKitWindowManager_C::Initialize() {
    _initialized = true;
    return true;
}

void UIKitWindowManager_C::Shutdown() {
    DestroyAllWindows();
    _initialized = false;
}

bool UIKitWindowManager_C::IsInitialized() const {
    return _initialized;
}

std::shared_ptr<Window_I> UIKitWindowManager_C::CreateWindow(const WindowDescriptor_C& descriptor) {
    if (!_initialized) {
        return nullptr;
    }
    auto w = std::make_shared<UIKitWindow_C>();
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

std::shared_ptr<Window_I> UIKitWindowManager_C::CreateWindow(const std::string& title,
                                                             uint32_t width,
                                                             uint32_t height) {
    WindowDescriptor_C d(title, width, height);
    return CreateWindow(d);
}

void UIKitWindowManager_C::DestroyWindow(std::shared_ptr<Window_I> window) {
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

void UIKitWindowManager_C::DestroyAllWindows() {
    for (auto& w : _windows) {
        if (w) {
            w->Close();
        }
    }
    _windows.clear();
    _primary.reset();
    _focused.reset();
}

size_t UIKitWindowManager_C::GetWindowCount() const {
    return _windows.size();
}

std::vector<std::shared_ptr<Window_I>> UIKitWindowManager_C::GetWindows() const {
    return _windows;
}

std::shared_ptr<Window_I> UIKitWindowManager_C::GetPrimaryWindow() const {
    return _primary;
}

std::shared_ptr<Window_I> UIKitWindowManager_C::GetFocusedWindow() const {
    return _focused;
}

void UIKitWindowManager_C::SetPrimaryWindow(std::shared_ptr<Window_I> window) {
    _primary = std::move(window);
}

void UIKitWindowManager_C::FocusWindow(std::shared_ptr<Window_I> window) {
    _focused = std::move(window);
}

void UIKitWindowManager_C::ProcessEvents() {}

void UIKitWindowManager_C::SetEventCallback(const WindowManagerEventCallback_T& callback) {
    _callback = callback;
}

void UIKitWindowManager_C::setEventBridgeCallbacks(EventBridgeCallbacks_C callbacks) {
    _event_bridge_callbacks = std::move(callbacks);
}

bool UIKitWindowManager_C::ShouldClose() const {
    for (const auto& w : _windows) {
        if (w && !w->IsOpen()) {
            return true;
        }
    }
    return false;
}

bool UIKitWindowManager_C::ShouldCloseAll() const {
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

WindowAPI_TP UIKitWindowManager_C::GetWindowAPI() const {
    return WindowAPI_TP::IOS_UIKIT_WINDOW;
}

std::string UIKitWindowManager_C::GetPlatformInfo() const {
    return "iOS / UIKit";
}

bool UIKitWindowManager_C::IsFeatureSupported(const std::string& feature) const {
    return feature == "resize" || feature == "dpi" || feature == "uikit";
}

std::string UIKitWindowManager_C::GetProperties() const {
    return _properties;
}

void UIKitWindowManager_C::SetProperties(const std::string& properties) {
    _properties = properties;
}

uint64_t UIKitWindowManager_C::GetCurrentTime() const {
    using namespace std::chrono;
    return static_cast<uint64_t>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}

void UIKitWindowManager_C::Sleep(uint32_t milliseconds) const {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

double UIKitWindowManager_C::GetPlatformTime() const {
    using namespace std::chrono;
    return duration<double>(steady_clock::now().time_since_epoch()).count();
}

}  // namespace vne::xwin
