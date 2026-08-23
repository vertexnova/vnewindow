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

CocoaWindowManager::CocoaWindowManager() = default;

CocoaWindowManager::~CocoaWindowManager() {
    shutdown();
}

bool CocoaWindowManager::initialize() {
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    initialized_ = true;
    return true;
}

void CocoaWindowManager::shutdown() {
    destroyAllWindows();
    initialized_ = false;
}

bool CocoaWindowManager::isInitialized() const noexcept {
    return initialized_;
}

std::shared_ptr<IWindow> CocoaWindowManager::openWindow(const WindowDescriptor& descriptor) {
    if (!initialized_) {
        return nullptr;
    }
    auto w = std::make_shared<CocoaWindow>();
    w->setEventOwner(this);
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

std::shared_ptr<IWindow> CocoaWindowManager::openWindow(const std::string& title, uint32_t width, uint32_t height) {
    WindowDescriptor descriptor(title, width, height);
    return openWindow(descriptor);
}

void CocoaWindowManager::removeWindow(std::shared_ptr<IWindow> window) {
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

void CocoaWindowManager::destroyAllWindows() {
    for (auto& w : windows_) {
        if (w) {
            w->close();
        }
    }
    windows_.clear();
    primary_.reset();
    focused_.reset();
}

size_t CocoaWindowManager::getWindowCount() const noexcept {
    return windows_.size();
}

std::vector<std::shared_ptr<IWindow>> CocoaWindowManager::getWindows() const {
    return windows_;
}

std::shared_ptr<IWindow> CocoaWindowManager::getPrimaryWindow() const noexcept {
    return primary_;
}

std::shared_ptr<IWindow> CocoaWindowManager::getFocusedWindow() const noexcept {
    return focused_;
}

void CocoaWindowManager::setPrimaryWindow(std::shared_ptr<IWindow> window) {
    primary_ = std::move(window);
}

void CocoaWindowManager::focusWindow(std::shared_ptr<IWindow> window) {
    focused_ = std::move(window);
}

void CocoaWindowManager::processEvents() {
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

bool CocoaWindowManager::shouldClose() const noexcept {
    for (const auto& w : windows_) {
        if (w && !w->isOpen()) {
            return true;
        }
    }
    return false;
}

bool CocoaWindowManager::shouldCloseAll() const noexcept {
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

WindowAPI CocoaWindowManager::getWindowAPI() const noexcept {
    return WindowAPI::eCocoaWindow;
}

std::string CocoaWindowManager::getPlatformInfo() const {
    return "macOS / AppKit";
}

bool CocoaWindowManager::isFeatureSupported(const std::string& feature) const {
    return feature == "resize" || feature == "dpi" || feature == "fullscreen";
}

std::string CocoaWindowManager::getProperties() const {
    return properties_;
}

void CocoaWindowManager::setProperties(const std::string& properties) {
    properties_ = properties;
}

uint64_t CocoaWindowManager::getCurrentTime() const noexcept {
    using namespace std::chrono;
    return static_cast<uint64_t>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}

void CocoaWindowManager::sleep(uint32_t milliseconds) const noexcept {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

double CocoaWindowManager::getPlatformTime() const noexcept {
    using namespace std::chrono;
    return duration<double>(steady_clock::now().time_since_epoch()).count();
}

}  // namespace vne::xwin
