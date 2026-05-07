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

#include "android_window_manager.h"

#include "android_window.h"

#include <algorithm>
#include <chrono>
#include <thread>

namespace vne::xwin {

AndroidWindowManager::AndroidWindowManager() = default;

AndroidWindowManager::~AndroidWindowManager() {
    shutdown();
}

void AndroidWindowManager::notifyWindowEvent(IWindow* window, const WindowEventData& event) {
    if (callback_ && window) {
        callback_(window, event);
    }
}

bool AndroidWindowManager::initialize() {
    initialized_ = true;
    return true;
}

void AndroidWindowManager::shutdown() {
    destroyAllWindows();
    initialized_ = false;
}

bool AndroidWindowManager::isInitialized() const noexcept {
    return initialized_;
}

std::shared_ptr<IWindow> AndroidWindowManager::openWindow(const WindowDescriptor& descriptor) {
    if (!initialized_) {
        return nullptr;
    }
    auto w = std::make_shared<AndroidWindow>();
    w->initialize(descriptor);
    if (!w->isOpen()) {
        return nullptr;
    }
    windows_.push_back(w);
    if (!primary_) {
        primary_ = w;
    }
    focused_ = w;
    return w;
}

std::shared_ptr<IWindow> AndroidWindowManager::openWindow(const std::string& title, uint32_t width, uint32_t height) {
    WindowDescriptor descriptor(title, width, height);
    return openWindow(descriptor);
}

void AndroidWindowManager::removeWindow(std::shared_ptr<IWindow> window) {
    if (!window) {
        return;
    }
    auto it = std::find(windows_.begin(), windows_.end(), window);
    if (it == windows_.end()) {
        return;
    }
    (*it)->close();
    windows_.erase(it);
    if (primary_ == window) {
        primary_ = windows_.empty() ? nullptr : windows_.front();
    }
    if (focused_ == window) {
        focused_ = primary_;
    }
}

void AndroidWindowManager::destroyAllWindows() {
    for (auto& w : windows_) {
        if (w) {
            w->close();
        }
    }
    windows_.clear();
    primary_.reset();
    focused_.reset();
}

size_t AndroidWindowManager::getWindowCount() const noexcept {
    return windows_.size();
}

std::vector<std::shared_ptr<IWindow>> AndroidWindowManager::getWindows() const {
    return windows_;
}

std::shared_ptr<IWindow> AndroidWindowManager::getPrimaryWindow() const noexcept {
    return primary_;
}

std::shared_ptr<IWindow> AndroidWindowManager::getFocusedWindow() const noexcept {
    return focused_;
}

void AndroidWindowManager::setPrimaryWindow(std::shared_ptr<IWindow> window) {
    primary_ = std::move(window);
}

void AndroidWindowManager::focusWindow(std::shared_ptr<IWindow> window) {
    focused_ = std::move(window);
}

void AndroidWindowManager::processEvents() {}

void AndroidWindowManager::setEventCallback(const WindowManagerEventCallbackT& callback) {
    callback_ = callback;
}

void AndroidWindowManager::setEventBridgeCallbacks(EventBridgeCallbacks callbacks) {
    event_bridge_callbacks_ = std::move(callbacks);
}

bool AndroidWindowManager::shouldClose() const noexcept {
    for (const auto& w : windows_) {
        if (w && !w->isOpen()) {
            return true;
        }
    }
    return false;
}

bool AndroidWindowManager::shouldCloseAll() const noexcept {
    if (windows_.empty()) {
        return false;
    }
    for (const auto& w : windows_) {
        if (w && w->isOpen()) {
            return false;
        }
    }
    return true;
}

WindowAPI AndroidWindowManager::getWindowAPI() const noexcept {
    return WindowAPI::eAndroidSurfaceWindow;
}

std::string AndroidWindowManager::getPlatformInfo() const {
    return "Android / ANativeWindow";
}

bool AndroidWindowManager::isFeatureSupported(const std::string& feature) const {
    return feature == "native_window" || feature == "resize";
}

std::string AndroidWindowManager::getProperties() const {
    return properties_;
}

void AndroidWindowManager::setProperties(const std::string& properties) {
    properties_ = properties;
}

uint64_t AndroidWindowManager::getCurrentTime() const noexcept {
    using namespace std::chrono;
    return static_cast<uint64_t>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}

void AndroidWindowManager::sleep(uint32_t milliseconds) const noexcept {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

double AndroidWindowManager::getPlatformTime() const noexcept {
    using namespace std::chrono;
    return duration<double>(steady_clock::now().time_since_epoch()).count();
}

}  // namespace vne::xwin
