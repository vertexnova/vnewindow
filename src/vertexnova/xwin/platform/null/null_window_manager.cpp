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
    shutdown();
}

bool NullWindowManager::initialize() {
    initialized_ = true;
    return true;
}

void NullWindowManager::shutdown() {
    destroyAllWindows();
    initialized_ = false;
}

bool NullWindowManager::isInitialized() const noexcept {
    return initialized_;
}

std::shared_ptr<IWindow> NullWindowManager::openWindow(const WindowDescriptor& descriptor) {
    if (!initialized_) {
        return nullptr;
    }
    auto w = std::make_shared<NullWindow>();
    w->initialize(descriptor);
    windows_.push_back(w);
    if (!primary_) {
        primary_ = w;
    }
    focused_ = w;
    return w;
}

std::shared_ptr<IWindow> NullWindowManager::openWindow(const std::string& title, uint32_t width, uint32_t height) {
    WindowDescriptor descriptor(title, width, height);
    return openWindow(descriptor);
}

void NullWindowManager::removeWindow(std::shared_ptr<IWindow> window) {
    if (!window) {
        return;
    }
    window->close();
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

void NullWindowManager::destroyAllWindows() {
    for (auto& w : windows_) {
        if (w) {
            w->close();
        }
    }
    windows_.clear();
    primary_.reset();
    focused_.reset();
}

size_t NullWindowManager::getWindowCount() const noexcept {
    return windows_.size();
}

std::vector<std::shared_ptr<IWindow>> NullWindowManager::getWindows() const {
    return windows_;
}

std::shared_ptr<IWindow> NullWindowManager::getPrimaryWindow() const noexcept {
    return primary_;
}

std::shared_ptr<IWindow> NullWindowManager::getFocusedWindow() const noexcept {
    return focused_;
}

void NullWindowManager::setPrimaryWindow(std::shared_ptr<IWindow> window) {
    primary_ = std::move(window);
}

void NullWindowManager::focusWindow(std::shared_ptr<IWindow> window) {
    focused_ = std::move(window);
}

void NullWindowManager::processEvents() {}

bool NullWindowManager::shouldClose() const noexcept {
    for (const auto& w : windows_) {
        if (w && !w->isOpen()) {
            return true;
        }
    }
    return false;
}

bool NullWindowManager::shouldCloseAll() const noexcept {
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

WindowAPI NullWindowManager::getWindowAPI() const noexcept {
    return WindowAPI::eNullWindow;
}

std::string NullWindowManager::getPlatformInfo() const {
    return "NullWindowManager (headless / tests)";
}

bool NullWindowManager::isFeatureSupported(const std::string& /*feature*/) const {
    return false;
}

std::string NullWindowManager::getProperties() const {
    return properties_;
}

void NullWindowManager::setProperties(const std::string& properties) {
    properties_ = properties;
}

uint64_t NullWindowManager::getCurrentTime() const noexcept {
    using namespace std::chrono;
    return static_cast<uint64_t>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}

void NullWindowManager::sleep(uint32_t milliseconds) const noexcept {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

double NullWindowManager::getPlatformTime() const noexcept {
    using namespace std::chrono;
    return duration<double>(steady_clock::now().time_since_epoch()).count();
}

}  // namespace vne::xwin
