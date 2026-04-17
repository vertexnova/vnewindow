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

class CocoaWindowManager_C final : public WindowManager_I {
   public:
    void NotifyWindowEvent(Window_I* window, const WindowEventData_C& event);
    const EventBridgeCallbacks& eventBridgeCallbacks() const { return _event_bridge_callbacks; }

    CocoaWindowManager_C();
    ~CocoaWindowManager_C() override;

    bool Initialize() override;
    void Shutdown() override;
    bool IsInitialized() const override;

    std::shared_ptr<Window_I> CreateWindow(const WindowDescriptor_C& descriptor) override;
    std::shared_ptr<Window_I> CreateWindow(const std::string& title, uint32_t width, uint32_t height) override;
    void DestroyWindow(std::shared_ptr<Window_I> window) override;
    void DestroyAllWindows() override;

    size_t GetWindowCount() const override;
    std::vector<std::shared_ptr<Window_I>> GetWindows() const override;
    std::shared_ptr<Window_I> GetPrimaryWindow() const override;
    std::shared_ptr<Window_I> GetFocusedWindow() const override;
    void SetPrimaryWindow(std::shared_ptr<Window_I> window) override;
    void FocusWindow(std::shared_ptr<Window_I> window) override;

    void ProcessEvents() override;
    void SetEventCallback(const WindowManagerEventCallback_T& callback) override;
    void setEventBridgeCallbacks(EventBridgeCallbacks callbacks) override;
    bool ShouldClose() const override;
    bool ShouldCloseAll() const override;

    WindowAPI_TP GetWindowAPI() const override;
    std::string GetPlatformInfo() const override;
    bool IsFeatureSupported(const std::string& feature) const override;
    std::string GetProperties() const override;
    void SetProperties(const std::string& properties) override;

    uint64_t GetCurrentTime() const override;
    void Sleep(uint32_t milliseconds) const override;
    double GetPlatformTime() const override;

   private:
    std::vector<std::shared_ptr<Window_I>> _windows;
    std::shared_ptr<Window_I> _primary;
    std::shared_ptr<Window_I> _focused;
    WindowManagerEventCallback_T _callback{};
    EventBridgeCallbacks _event_bridge_callbacks{};
    bool _initialized = false;
    std::string _properties;
};

}  // namespace vne::xwin
