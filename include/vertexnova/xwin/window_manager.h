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

using WindowManagerEventCallbackT = std::function<void(IWindow*, const WindowEventData&)>;

/**
 * @brief Owns and dispatches a set of IWindow instances and platform-wide events.
 *
 * @warning Not thread-safe unless a platform implementation documents otherwise; use from the main/event thread.
 */
class IWindowManager {
   public:
    virtual ~IWindowManager() = default;

    [[nodiscard]] virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    [[nodiscard]] virtual bool isInitialized() const noexcept = 0;

    [[nodiscard]] virtual std::shared_ptr<IWindow> openWindow(const WindowDescriptor& descriptor) = 0;
    [[nodiscard]] virtual std::shared_ptr<IWindow> openWindow(const std::string& title,
                                                              uint32_t width,
                                                              uint32_t height) = 0;
    virtual void removeWindow(std::shared_ptr<IWindow> window) = 0;
    virtual void destroyAllWindows() = 0;

    [[nodiscard]] virtual size_t getWindowCount() const noexcept = 0;
    [[nodiscard]] virtual std::vector<std::shared_ptr<IWindow>> getWindows() const = 0;
    [[nodiscard]] virtual std::shared_ptr<IWindow> getPrimaryWindow() const noexcept = 0;
    [[nodiscard]] virtual std::shared_ptr<IWindow> getFocusedWindow() const noexcept = 0;
    virtual void setPrimaryWindow(std::shared_ptr<IWindow> window) = 0;
    virtual void focusWindow(std::shared_ptr<IWindow> window) = 0;

    virtual void processEvents() = 0;
    virtual void setEventCallback(const WindowManagerEventCallbackT& callback) = 0;
    /**
     * @brief Optional granular hooks after vne::events updates (see event_bridge_callbacks.h).
     *
     * After each frame: call vne::events::EventManager::instance().processEvents() if you use the
     * queued events, then vne::events::Input::nextFrame() once after your simulation step (see
     * vneevents input documentation). xwin does not call nextFrame() inside processEvents().
     */
    virtual void setEventBridgeCallbacks(EventBridgeCallbacks callbacks) = 0;
    [[nodiscard]] virtual bool shouldClose() const noexcept = 0;
    [[nodiscard]] virtual bool shouldCloseAll() const noexcept = 0;

    [[nodiscard]] virtual WindowAPI getWindowAPI() const noexcept = 0;
    [[nodiscard]] virtual std::string getPlatformInfo() const = 0;
    [[nodiscard]] virtual bool isFeatureSupported(const std::string& feature) const = 0;

    [[nodiscard]] virtual std::string getProperties() const = 0;
    virtual void setProperties(const std::string& properties) = 0;
    [[nodiscard]] virtual uint32_t getMonitorCount() const noexcept;
    [[nodiscard]] virtual MonitorInfo getMonitorInfo(uint32_t index) const;
    [[nodiscard]] virtual uint32_t getPrimaryMonitorIndex() const noexcept;

    [[nodiscard]] virtual uint64_t getCurrentTime() const noexcept = 0;
    virtual void sleep(uint32_t milliseconds) const noexcept = 0;
    [[nodiscard]] virtual double getPlatformTime() const noexcept = 0;
};

inline uint32_t IWindowManager::getMonitorCount() const noexcept {
    return 0;
}
inline MonitorInfo IWindowManager::getMonitorInfo(uint32_t index) const {
    (void)index;
    return {};
}
inline uint32_t IWindowManager::getPrimaryMonitorIndex() const noexcept {
    return 0;
}

}  // namespace vne::xwin
