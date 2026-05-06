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

/** @file window_manager.h Multi-window host (vne::xwin::IWindowManager). */

#include "vertexnova/xwin/window.h"
#include "vertexnova/xwin/window_descriptor.h"
#include "vertexnova/xwin/monitor_info.h"
#include "vertexnova/xwin/xwin_types.h"
#include "vertexnova/xwin/event_bridge_callbacks.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace vne::xwin {

using WindowManagerEventCallback_T = std::function<void(IWindow*, const WindowEventData&)>;

/**
 * @brief Owns and dispatches a set of IWindow instances and platform-wide events.
 *
 * @warning Not thread-safe unless a platform implementation documents otherwise; use from the main/event thread.
 */
class IWindowManager {
   public:
    virtual ~IWindowManager() = default;

    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;
    virtual bool IsInitialized() const = 0;

    virtual std::shared_ptr<IWindow> OpenWindow(const WindowDescriptor& descriptor) = 0;
    virtual std::shared_ptr<IWindow> OpenWindow(const std::string& title, uint32_t width, uint32_t height) = 0;
    virtual void RemoveWindow(std::shared_ptr<IWindow> window) = 0;
    virtual void DestroyAllWindows() = 0;

    virtual size_t GetWindowCount() const = 0;
    virtual std::vector<std::shared_ptr<IWindow>> GetWindows() const = 0;
    virtual std::shared_ptr<IWindow> GetPrimaryWindow() const = 0;
    virtual std::shared_ptr<IWindow> GetFocusedWindow() const = 0;
    virtual void SetPrimaryWindow(std::shared_ptr<IWindow> window) = 0;
    virtual void FocusWindow(std::shared_ptr<IWindow> window) = 0;

    virtual void ProcessEvents() = 0;
    virtual void SetEventCallback(const WindowManagerEventCallback_T& callback) = 0;
    /**
     * @brief Optional granular hooks after vne::events updates (see event_bridge_callbacks.h).
     *
     * After each frame: call vne::events::EventManager::instance().processEvents() if you use the
     * queued events, then vne::events::Input::nextFrame() once after your simulation step (see
     * vneevents input documentation). xwin does not call nextFrame() inside ProcessEvents().
     */
    virtual void setEventBridgeCallbacks(EventBridgeCallbacks callbacks) = 0;
    virtual bool ShouldClose() const = 0;
    virtual bool ShouldCloseAll() const = 0;

    virtual WindowAPI GetWindowAPI() const = 0;
    virtual std::string GetPlatformInfo() const = 0;
    virtual bool IsFeatureSupported(const std::string& feature) const = 0;

    virtual std::string GetProperties() const = 0;
    virtual void SetProperties(const std::string& properties) = 0;
    virtual uint32_t GetMonitorCount() const;
    virtual MonitorInfo GetMonitorInfo(uint32_t index) const;
    virtual uint32_t GetPrimaryMonitorIndex() const;

    virtual uint64_t GetCurrentTime() const = 0;
    virtual void Sleep(uint32_t milliseconds) const = 0;
    virtual double GetPlatformTime() const = 0;
};

inline uint32_t IWindowManager::GetMonitorCount() const {
    return 0;
}
inline MonitorInfo IWindowManager::GetMonitorInfo(uint32_t index) const {
    (void)index;
    return {};
}
inline uint32_t IWindowManager::GetPrimaryMonitorIndex() const {
    return 0;
}

}  // namespace vne::xwin
