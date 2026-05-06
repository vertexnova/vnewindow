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

UIKitWindowManager::UIKitWindowManager() = default;

UIKitWindowManager::~UIKitWindowManager() {
    Shutdown();
}

void UIKitWindowManager::notifyWindowEvent(IWindow* window, const WindowEventData& event) {
    if (callback_ && window) {
        callback_(window, event);
    }
}

bool UIKitWindowManager::Initialize() {
    initialized_ = true;
    return true;
}

void UIKitWindowManager::Shutdown() {
    DestroyAllWindows();
    initialized_ = false;
}

bool UIKitWindowManager::IsInitialized() const noexcept {
    return initialized_;
}

std::shared_ptr<IWindow> UIKitWindowManager::OpenWindow(const WindowDescriptor& descriptor) {
    if (!initialized_) {
        return nullptr;
    }
    auto w = std::make_shared<UIKitWindow>();
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

std::shared_ptr<IWindow> UIKitWindowManager::OpenWindow(const std::string& title, uint32_t width, uint32_t height) {
    WindowDescriptor descriptor(title, width, height);
    return OpenWindow(descriptor);
}

void UIKitWindowManager::RemoveWindow(std::shared_ptr<IWindow> window) {
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

void UIKitWindowManager::DestroyAllWindows() {
    for (auto& w : windows_) {
        if (w) {
            w->Close();
        }
    }
    windows_.clear();
    primary_.reset();
    focused_.reset();
}

size_t UIKitWindowManager::GetWindowCount() const noexcept {
    return windows_.size();
}

std::vector<std::shared_ptr<IWindow>> UIKitWindowManager::GetWindows() const {
    return windows_;
}

std::shared_ptr<IWindow> UIKitWindowManager::GetPrimaryWindow() const noexcept {
    return primary_;
}

std::shared_ptr<IWindow> UIKitWindowManager::GetFocusedWindow() const noexcept {
    return focused_;
}

void UIKitWindowManager::SetPrimaryWindow(std::shared_ptr<IWindow> window) {
    primary_ = std::move(window);
}

void UIKitWindowManager::FocusWindow(std::shared_ptr<IWindow> window) {
    focused_ = std::move(window);
}

void UIKitWindowManager::ProcessEvents() {}

void UIKitWindowManager::SetEventCallback(const WindowManagerEventCallback_T& callback) {
    callback_ = callback;
}

void UIKitWindowManager::setEventBridgeCallbacks(EventBridgeCallbacks callbacks) {
    event_bridge_callbacks_ = std::move(callbacks);
}

bool UIKitWindowManager::ShouldClose() const noexcept {
    for (const auto& w : windows_) {
        if (w && !w->IsOpen()) {
            return true;
        }
    }
    return false;
}

bool UIKitWindowManager::ShouldCloseAll() const noexcept {
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

WindowAPI UIKitWindowManager::GetWindowAPI() const noexcept {
    return WindowAPI::eIosUikitWindow;
}

std::string UIKitWindowManager::GetPlatformInfo() const {
    return "iOS / UIKit";
}

bool UIKitWindowManager::IsFeatureSupported(const std::string& feature) const {
    return feature == "resize" || feature == "dpi" || feature == "uikit";
}

std::string UIKitWindowManager::GetProperties() const {
    return properties_;
}

void UIKitWindowManager::SetProperties(const std::string& properties) {
    properties_ = properties;
}

uint64_t UIKitWindowManager::GetCurrentTime() const noexcept {
    using namespace std::chrono;
    return static_cast<uint64_t>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}

void UIKitWindowManager::Sleep(uint32_t milliseconds) const noexcept {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

double UIKitWindowManager::GetPlatformTime() const noexcept {
    using namespace std::chrono;
    return duration<double>(steady_clock::now().time_since_epoch()).count();
}

}  // namespace vne::xwin
