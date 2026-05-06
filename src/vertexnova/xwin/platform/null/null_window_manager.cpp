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

#include "null_window_manager.h"

#include "null_window.h"

#include <algorithm>
#include <thread>

namespace vne::xwin {

NullWindowManager::~NullWindowManager() {
    Shutdown();
}

bool NullWindowManager::Initialize() {
    initialized_ = true;
    return true;
}

void NullWindowManager::Shutdown() {
    DestroyAllWindows();
    initialized_ = false;
}

bool NullWindowManager::IsInitialized() const noexcept {
    return initialized_;
}

std::shared_ptr<IWindow> NullWindowManager::OpenWindow(const WindowDescriptor& descriptor) {
    if (!initialized_) {
        return nullptr;
    }
    auto w = std::make_shared<NullWindow>();
    w->Initialize(descriptor);
    windows_.push_back(w);
    if (!primary_) {
        primary_ = w;
    }
    focused_ = w;
    return w;
}

std::shared_ptr<IWindow> NullWindowManager::OpenWindow(const std::string& title, uint32_t width, uint32_t height) {
    WindowDescriptor descriptor(title, width, height);
    return OpenWindow(descriptor);
}

void NullWindowManager::RemoveWindow(std::shared_ptr<IWindow> window) {
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

void NullWindowManager::DestroyAllWindows() {
    for (auto& w : windows_) {
        if (w) {
            w->Close();
        }
    }
    windows_.clear();
    primary_.reset();
    focused_.reset();
}

size_t NullWindowManager::GetWindowCount() const noexcept {
    return windows_.size();
}

std::vector<std::shared_ptr<IWindow>> NullWindowManager::GetWindows() const {
    return windows_;
}

std::shared_ptr<IWindow> NullWindowManager::GetPrimaryWindow() const noexcept {
    return primary_;
}

std::shared_ptr<IWindow> NullWindowManager::GetFocusedWindow() const noexcept {
    return focused_;
}

void NullWindowManager::SetPrimaryWindow(std::shared_ptr<IWindow> window) {
    primary_ = std::move(window);
}

void NullWindowManager::FocusWindow(std::shared_ptr<IWindow> window) {
    focused_ = std::move(window);
}

void NullWindowManager::ProcessEvents() {}

void NullWindowManager::SetEventCallback(const WindowManagerEventCallback_T& callback) {
    callback_ = callback;
}

void NullWindowManager::setEventBridgeCallbacks(EventBridgeCallbacks callbacks) {
    event_bridge_callbacks_ = std::move(callbacks);
}

bool NullWindowManager::ShouldClose() const noexcept {
    for (const auto& w : windows_) {
        if (w && !w->IsOpen()) {
            return true;
        }
    }
    return false;
}

bool NullWindowManager::ShouldCloseAll() const noexcept {
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

WindowAPI NullWindowManager::GetWindowAPI() const noexcept {
    return WindowAPI::eNullWindow;
}

std::string NullWindowManager::GetPlatformInfo() const {
    return "NullWindowManager (headless / tests)";
}

bool NullWindowManager::IsFeatureSupported(const std::string& /*feature*/) const {
    return false;
}

std::string NullWindowManager::GetProperties() const {
    return properties_;
}

void NullWindowManager::SetProperties(const std::string& properties) {
    properties_ = properties;
}

uint64_t NullWindowManager::GetCurrentTime() const noexcept {
    using namespace std::chrono;
    return static_cast<uint64_t>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}

void NullWindowManager::Sleep(uint32_t milliseconds) const noexcept {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

double NullWindowManager::GetPlatformTime() const noexcept {
    using namespace std::chrono;
    return duration<double>(steady_clock::now().time_since_epoch()).count();
}

}  // namespace vne::xwin
