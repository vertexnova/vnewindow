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

/** @file window.h Abstraction for platform windows (vne::xwin::IWindow). */

#include "vertexnova/xwin/window_descriptor.h"
#include "vertexnova/xwin/native_window_handle.h"
#include "vertexnova/xwin/xwin_types.h"

#include <functional>
#include <memory>
#include <string>

namespace vne::xwin {

struct WindowEventData;

using WindowEventCallback_T = std::function<void(const WindowEventData&)>;

/**
 * @brief Platform window abstraction; rendering presents externally — swapBuffers may be a no-op when GL is external.
 *
 * @warning Most implementations are main-thread-only (especially AppKit/UIKit). Call initialize, pollEvents,
 *          and accessors from the thread that owns the native event loop unless documented otherwise.
 */
class IWindow {
   public:
    virtual ~IWindow() = default;

    virtual void initialize(const WindowDescriptor& descriptor) = 0;
    virtual void pollEvents() = 0;
    /** @brief No-op unless a platform GL context is owned by this window. */
    virtual void swapBuffers() = 0;

    virtual void setTitle(const std::string& title) = 0;
    virtual void setWindowMode(WindowMode mode) = 0;
    [[nodiscard]] virtual WindowMode getWindowMode() const noexcept = 0;
    virtual void setFullscreen(bool enabled) = 0;
    [[nodiscard]] virtual bool isFullscreen() const noexcept = 0;
    virtual void setPosition(int x, int y) = 0;
    virtual void getPosition(int& x, int& y) const = 0;
    virtual void setWindowLimits(const WindowLimits& limits);
    virtual void setCursor(WindowCursor cursor);
    virtual void setMonitor(uint32_t monitor_index);
    [[nodiscard]] virtual uint32_t getMonitor() const noexcept;
    [[nodiscard]] virtual float getDpiScale() const noexcept;
    [[nodiscard]] virtual uint32_t getFramebufferWidth() const noexcept;
    [[nodiscard]] virtual uint32_t getFramebufferHeight() const noexcept;
    virtual void setTransparent(bool enabled);
    [[nodiscard]] virtual bool isTransparent() const noexcept;
    virtual void setVSync(bool enabled);
    [[nodiscard]] virtual bool isVSyncEnabled() const noexcept;
    virtual void minimize();
    virtual void maximize();
    virtual void restore();
    virtual void resize(uint32_t width, uint32_t height) = 0;
    virtual void close() = 0;
    [[nodiscard]] virtual bool isOpen() const noexcept = 0;
    [[nodiscard]] virtual void* getNativeWindow() const noexcept = 0;
    [[nodiscard]] virtual NativeWindowHandle getNativeHandle() const noexcept;
    [[nodiscard]] virtual WindowAPI getWindowAPI() const noexcept = 0;
    [[nodiscard]] virtual int getWidth() const noexcept = 0;
    [[nodiscard]] virtual int getHeight() const noexcept = 0;
    [[nodiscard]] virtual std::string getClipboardText() const;
    virtual void setClipboardText(const std::string& text);
    virtual void setWindowIcon(const uint8_t* rgba_pixels, uint32_t width, uint32_t height);

    [[nodiscard]] static std::unique_ptr<IWindow> create(const WindowDescriptor& descriptor);
};

inline void IWindow::setWindowLimits(const WindowLimits& limits) {
    (void)limits;
}
inline void IWindow::setCursor(WindowCursor cursor) {
    (void)cursor;
}
inline void IWindow::setMonitor(uint32_t monitor_index) {
    (void)monitor_index;
}
inline uint32_t IWindow::getMonitor() const noexcept {
    return 0;
}
inline float IWindow::getDpiScale() const noexcept {
    return 1.0F;
}
inline uint32_t IWindow::getFramebufferWidth() const noexcept {
    return static_cast<uint32_t>(static_cast<float>(getWidth()) * getDpiScale());
}
inline uint32_t IWindow::getFramebufferHeight() const noexcept {
    return static_cast<uint32_t>(static_cast<float>(getHeight()) * getDpiScale());
}
inline void IWindow::setTransparent(bool enabled) {
    (void)enabled;
}
inline bool IWindow::isTransparent() const noexcept {
    return false;
}
inline void IWindow::setVSync(bool enabled) {
    (void)enabled;
}
inline bool IWindow::isVSyncEnabled() const noexcept {
    return false;
}
inline void IWindow::minimize() {}
inline void IWindow::maximize() {}
inline void IWindow::restore() {}
inline NativeWindowHandle IWindow::getNativeHandle() const noexcept {
    return {};
}
inline std::string IWindow::getClipboardText() const {
    return {};
}
inline void IWindow::setClipboardText(const std::string& text) {
    (void)text;
}
inline void IWindow::setWindowIcon(const uint8_t* rgba_pixels, uint32_t width, uint32_t height) {
    (void)rgba_pixels;
    (void)width;
    (void)height;
}

}  // namespace vne::xwin
