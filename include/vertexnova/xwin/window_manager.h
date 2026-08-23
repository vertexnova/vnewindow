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

#include <memory>
#include <string>
#include <vector>

namespace vne::xwin {

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
    /**
     * @brief Resolves the window an event came from, via IWindow::getId().
     * @return The matching window, or nullptr if it has since been removed.
     */
    [[nodiscard]] virtual std::shared_ptr<IWindow> findWindow(vne::events::WindowId id) const;
    virtual void setPrimaryWindow(std::shared_ptr<IWindow> window) = 0;
    virtual void focusWindow(std::shared_ptr<IWindow> window) = 0;

    /**
     * @brief Pumps the platform event loop, emitting vne::events events for whatever arrived.
     *
     * This only fills the queue. Each frame the application then calls
     * vne::events::EventManager::instance().processEvents() to dispatch it, and
     * vne::events::Input::nextFrame() once after its simulation step. xwin calls neither.
     */
    virtual void processEvents() = 0;

    /**
     * @brief Emits a process-scoped application lifecycle event.
     *
     * Called by the platform host, which is the only thing that sees these transitions: the iOS
     * scene delegate on resign-active / enter-background, the Android host on APP_CMD_PAUSE and
     * friends, the browser on visibilitychange. Not window-scoped, so the resulting event carries
     * no window id.
     *
     * On eLowMemory, release what you can — mobile systems terminate apps that ignore it.
     */
    virtual void notifyApplicationLifecycle(ApplicationLifecycle transition);
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

inline std::shared_ptr<IWindow> IWindowManager::findWindow(vne::events::WindowId id) const {
    if (id == vne::events::kInvalidWindowId) {
        return nullptr;
    }
    for (const auto& window : getWindows()) {
        if (window && window->getId() == id) {
            return window;
        }
    }
    return nullptr;
}

}  // namespace vne::xwin
