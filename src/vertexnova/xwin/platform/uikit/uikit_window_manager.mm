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

#include "uikit_main_sync.h"
#include "uikit_window.h"

#import <UIKit/UIKit.h>

#include <algorithm>
#include <chrono>
#include <thread>

namespace vne::xwin {

UIKitWindowManager::UIKitWindowManager() = default;

UIKitWindowManager::~UIKitWindowManager() {
    shutdown();
}

void UIKitWindowManager::notifyWindowEvent(IWindow* window, const WindowEventData& event) {
    if (callback_ && window) {
        callback_(window, event);
    }
}

bool UIKitWindowManager::initialize() {
    initialized_ = true;
    return true;
}

void UIKitWindowManager::shutdown() {
    destroyAllWindows();
    initialized_ = false;
}

bool UIKitWindowManager::isInitialized() const noexcept {
    return initialized_;
}

std::shared_ptr<IWindow> UIKitWindowManager::openWindow(const WindowDescriptor& descriptor) {
    if (!initialized_) {
        return nullptr;
    }
    __block std::shared_ptr<IWindow> out;
    uikitRunOnMainSync(^{
      auto w = std::make_shared<UIKitWindow>();
      w->setEventOwner(this);
      w->initialize(descriptor);
      if (!w->isOpen()) {
          out = nullptr;
          return;
      }
      windows_.push_back(w);
      if (!primary_) {
          primary_ = w;
      }
      focused_ = w;
      out = std::move(w);
    });
    return out;
}

std::shared_ptr<IWindow> UIKitWindowManager::openWindow(const std::string& title, uint32_t width, uint32_t height) {
    WindowDescriptor descriptor(title, width, height);
    return openWindow(descriptor);
}

void UIKitWindowManager::removeWindow(std::shared_ptr<IWindow> window) {
    if (!window) {
        return;
    }
    std::shared_ptr<IWindow> w = std::move(window);
    uikitRunOnMainSync(^{
      w->close();
      auto it = std::find(windows_.begin(), windows_.end(), w);
      if (it != windows_.end()) {
          windows_.erase(it);
      }
      if (primary_ == w) {
          primary_ = windows_.empty() ? nullptr : windows_.front();
      }
      if (focused_ == w) {
          focused_ = primary_;
      }
    });
}

void UIKitWindowManager::destroyAllWindows() {
    uikitRunOnMainSync(^{
      for (auto& w : windows_) {
          if (w) {
              w->close();
          }
      }
      windows_.clear();
      primary_.reset();
      focused_.reset();
    });
}

size_t UIKitWindowManager::getWindowCount() const noexcept {
    return windows_.size();
}

std::vector<std::shared_ptr<IWindow>> UIKitWindowManager::getWindows() const {
    return windows_;
}

std::shared_ptr<IWindow> UIKitWindowManager::getPrimaryWindow() const noexcept {
    return primary_;
}

std::shared_ptr<IWindow> UIKitWindowManager::getFocusedWindow() const noexcept {
    return focused_;
}

void UIKitWindowManager::setPrimaryWindow(std::shared_ptr<IWindow> window) {
    primary_ = std::move(window);
}

void UIKitWindowManager::focusWindow(std::shared_ptr<IWindow> window) {
    focused_ = std::move(window);
}

void UIKitWindowManager::processEvents() {}

void UIKitWindowManager::setEventCallback(const WindowManagerEventCallbackT& callback) {
    callback_ = callback;
}

void UIKitWindowManager::setEventBridgeCallbacks(EventBridgeCallbacks callbacks) {
    event_bridge_callbacks_ = std::move(callbacks);
}

bool UIKitWindowManager::shouldClose() const noexcept {
    for (const auto& w : windows_) {
        if (w && !w->isOpen()) {
            return true;
        }
    }
    return false;
}

bool UIKitWindowManager::shouldCloseAll() const noexcept {
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

WindowAPI UIKitWindowManager::getWindowAPI() const noexcept {
    return WindowAPI::eIosUikitWindow;
}

std::string UIKitWindowManager::getPlatformInfo() const {
    return "iOS / UIKit";
}

bool UIKitWindowManager::isFeatureSupported(const std::string& feature) const {
    return feature == "resize" || feature == "dpi" || feature == "uikit";
}

std::string UIKitWindowManager::getProperties() const {
    return properties_;
}

void UIKitWindowManager::setProperties(const std::string& properties) {
    properties_ = properties;
}

uint64_t UIKitWindowManager::getCurrentTime() const noexcept {
    using namespace std::chrono;
    return static_cast<uint64_t>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}

void UIKitWindowManager::sleep(uint32_t milliseconds) const noexcept {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

double UIKitWindowManager::getPlatformTime() const noexcept {
    using namespace std::chrono;
    return duration<double>(steady_clock::now().time_since_epoch()).count();
}

}  // namespace vne::xwin
