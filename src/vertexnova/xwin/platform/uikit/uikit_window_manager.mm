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

void UIKitWindowManager_C::NotifyWindowEvent(IWindow* window, const WindowEventData& event) {
    if (callback_ && window) {
        callback_(window, event);
    }
}

bool UIKitWindowManager_C::Initialize() {
    initialized_ = true;
    return true;
}

void UIKitWindowManager_C::Shutdown() {
    DestroyAllWindows();
    initialized_ = false;
}

bool UIKitWindowManager_C::IsInitialized() const {
    return initialized_;
}

std::shared_ptr<IWindow> UIKitWindowManager_C::CreateWindow(const WindowDescriptor& descriptor) {
    if (!initialized_) {
        return nullptr;
    }
    auto w = std::make_shared<UIKitWindow_C>();
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

std::shared_ptr<IWindow> UIKitWindowManager_C::CreateWindow(const std::string& title, uint32_t width, uint32_t height) {
    WindowDescriptor d(title, width, height);
    return CreateWindow(d);
}

void UIKitWindowManager_C::DestroyWindow(std::shared_ptr<IWindow> window) {
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

void UIKitWindowManager_C::DestroyAllWindows() {
    for (auto& w : windows_) {
        if (w) {
            w->Close();
        }
    }
    windows_.clear();
    primary_.reset();
    focused_.reset();
}

size_t UIKitWindowManager_C::GetWindowCount() const {
    return windows_.size();
}

std::vector<std::shared_ptr<IWindow>> UIKitWindowManager_C::GetWindows() const {
    return windows_;
}

std::shared_ptr<IWindow> UIKitWindowManager_C::GetPrimaryWindow() const {
    return primary_;
}

std::shared_ptr<IWindow> UIKitWindowManager_C::GetFocusedWindow() const {
    return focused_;
}

void UIKitWindowManager_C::SetPrimaryWindow(std::shared_ptr<IWindow> window) {
    primary_ = std::move(window);
}

void UIKitWindowManager_C::FocusWindow(std::shared_ptr<IWindow> window) {
    focused_ = std::move(window);
}

void UIKitWindowManager_C::ProcessEvents() {}

void UIKitWindowManager_C::SetEventCallback(const WindowManagerEventCallback_T& callback) {
    callback_ = callback;
}

void UIKitWindowManager_C::setEventBridgeCallbacks(EventBridgeCallbacks callbacks) {
    event_bridge_callbacks_ = std::move(callbacks);
}

bool UIKitWindowManager_C::ShouldClose() const {
    for (const auto& w : windows_) {
        if (w && !w->IsOpen()) {
            return true;
        }
    }
    return false;
}

bool UIKitWindowManager_C::ShouldCloseAll() const {
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

WindowAPI UIKitWindowManager_C::GetWindowAPI() const {
    return WindowAPI::eIosUikitWindow;
}

std::string UIKitWindowManager_C::GetPlatformInfo() const {
    return "iOS / UIKit";
}

bool UIKitWindowManager_C::IsFeatureSupported(const std::string& feature) const {
    return feature == "resize" || feature == "dpi" || feature == "uikit";
}

std::string UIKitWindowManager_C::GetProperties() const {
    return properties_;
}

void UIKitWindowManager_C::SetProperties(const std::string& properties) {
    properties_ = properties;
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
