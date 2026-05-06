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

class AndroidWindowManager final : public IWindowManager {
   public:
    void notifyWindowEvent(IWindow* window, const WindowEventData& event);

    AndroidWindowManager();
    ~AndroidWindowManager() override;

    [[nodiscard]] bool Initialize() override;
    void Shutdown() override;
    [[nodiscard]] bool IsInitialized() const noexcept override;

    [[nodiscard]] std::shared_ptr<IWindow> OpenWindow(const WindowDescriptor& descriptor) override;
    [[nodiscard]] std::shared_ptr<IWindow> OpenWindow(const std::string& title,
                                                      uint32_t width,
                                                      uint32_t height) override;
    void RemoveWindow(std::shared_ptr<IWindow> window) override;
    void DestroyAllWindows() override;

    [[nodiscard]] size_t GetWindowCount() const noexcept override;
    [[nodiscard]] std::vector<std::shared_ptr<IWindow>> GetWindows() const override;
    [[nodiscard]] std::shared_ptr<IWindow> GetPrimaryWindow() const noexcept override;
    [[nodiscard]] std::shared_ptr<IWindow> GetFocusedWindow() const noexcept override;
    void SetPrimaryWindow(std::shared_ptr<IWindow> window) override;
    void FocusWindow(std::shared_ptr<IWindow> window) override;

    void ProcessEvents() override;
    void SetEventCallback(const WindowManagerEventCallback_T& callback) override;
    void setEventBridgeCallbacks(EventBridgeCallbacks callbacks) override;
    [[nodiscard]] bool ShouldClose() const noexcept override;
    [[nodiscard]] bool ShouldCloseAll() const noexcept override;

    [[nodiscard]] WindowAPI GetWindowAPI() const noexcept override;
    [[nodiscard]] std::string GetPlatformInfo() const override;
    [[nodiscard]] bool IsFeatureSupported(const std::string& feature) const override;
    [[nodiscard]] std::string GetProperties() const override;
    void SetProperties(const std::string& properties) override;

    [[nodiscard]] uint64_t GetCurrentTime() const noexcept override;
    void Sleep(uint32_t milliseconds) const noexcept override;
    [[nodiscard]] double GetPlatformTime() const noexcept override;

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
