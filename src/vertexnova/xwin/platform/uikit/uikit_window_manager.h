#pragma once
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

#include "vertexnova/xwin/window_manager.h"

#include <chrono>
#include <memory>
#include <string>
#include <vector>

namespace vne::xwin {

class UIKitWindowManager_C final : public IWindowManager {
   public:
    void NotifyWindowEvent(IWindow* window, const WindowEventData& event);
    const EventBridgeCallbacks& eventBridgeCallbacks() const { return event_bridge_callbacks_; }

    UIKitWindowManager_C();
    ~UIKitWindowManager_C() override;

    bool Initialize() override;
    void Shutdown() override;
    bool IsInitialized() const override;

    std::shared_ptr<IWindow> OpenWindow(const WindowDescriptor& descriptor) override;
    std::shared_ptr<IWindow> OpenWindow(const std::string& title, uint32_t width, uint32_t height) override;
    void RemoveWindow(std::shared_ptr<IWindow> window) override;
    void DestroyAllWindows() override;

    size_t GetWindowCount() const override;
    std::vector<std::shared_ptr<IWindow>> GetWindows() const override;
    std::shared_ptr<IWindow> GetPrimaryWindow() const override;
    std::shared_ptr<IWindow> GetFocusedWindow() const override;
    void SetPrimaryWindow(std::shared_ptr<IWindow> window) override;
    void FocusWindow(std::shared_ptr<IWindow> window) override;

    void ProcessEvents() override;
    void SetEventCallback(const WindowManagerEventCallback_T& callback) override;
    void setEventBridgeCallbacks(EventBridgeCallbacks callbacks) override;
    bool ShouldClose() const override;
    bool ShouldCloseAll() const override;

    WindowAPI GetWindowAPI() const override;
    std::string GetPlatformInfo() const override;
    bool IsFeatureSupported(const std::string& feature) const override;
    std::string GetProperties() const override;
    void SetProperties(const std::string& properties) override;

    uint64_t GetCurrentTime() const override;
    void Sleep(uint32_t milliseconds) const override;
    double GetPlatformTime() const override;

   private:
    std::vector<std::shared_ptr<IWindow>> windows_;
    std::shared_ptr<IWindow> primary_;
    std::shared_ptr<IWindow> focused_;
    WindowManagerEventCallback_T callback_{};
    EventBridgeCallbacks event_bridge_callbacks_{};
    bool initialized_ = false;
    std::string properties_;
};

}  // namespace vne::xwin
