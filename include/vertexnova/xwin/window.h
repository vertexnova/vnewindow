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
 * @brief Platform window abstraction; rendering presents externally — SwapBuffers may be a no-op when GL is external.
 *
 * @warning Most implementations are main-thread-only (especially AppKit/UIKit). Call Initialize, PollEvents,
 *          and accessors from the thread that owns the native event loop unless documented otherwise.
 */
class IWindow {
   public:
    virtual ~IWindow() = default;

    virtual void Initialize(const WindowDescriptor& descriptor) = 0;
    virtual void PollEvents() = 0;
    /** @brief No-op unless a platform GL context is owned by this window. */
    virtual void SwapBuffers() = 0;

    virtual void SetTitle(const std::string& title) = 0;
    virtual void SetWindowMode(WindowMode mode) = 0;
    [[nodiscard]] virtual WindowMode GetWindowMode() const noexcept = 0;
    virtual void SetFullscreen(bool enabled) = 0;
    [[nodiscard]] virtual bool IsFullscreen() const noexcept = 0;
    virtual void SetPosition(int x, int y) = 0;
    virtual void GetPosition(int& x, int& y) const = 0;
    virtual void SetWindowLimits(const WindowLimits& limits);
    virtual void SetCursor(WindowCursor cursor);
    virtual void SetMonitor(uint32_t monitor_index);
    [[nodiscard]] virtual uint32_t GetMonitor() const noexcept;
    [[nodiscard]] virtual float GetDPIScale() const noexcept;
    [[nodiscard]] virtual uint32_t GetFramebufferWidth() const noexcept;
    [[nodiscard]] virtual uint32_t GetFramebufferHeight() const noexcept;
    virtual void SetTransparent(bool enabled);
    [[nodiscard]] virtual bool IsTransparent() const noexcept;
    virtual void SetVSync(bool enabled);
    [[nodiscard]] virtual bool IsVSyncEnabled() const noexcept;
    virtual void Minimize();
    virtual void Maximize();
    virtual void Restore();
    virtual void Resize(uint32_t width, uint32_t height) = 0;
    virtual void Close() = 0;
    [[nodiscard]] virtual bool IsOpen() const noexcept = 0;
    [[nodiscard]] virtual void* GetNativeWindow() const noexcept = 0;
    [[nodiscard]] virtual NativeWindowHandle GetNativeHandle() const noexcept;
    [[nodiscard]] virtual WindowAPI GetWindowAPI() const noexcept = 0;
    [[nodiscard]] virtual int GetWidth() const noexcept = 0;
    [[nodiscard]] virtual int GetHeight() const noexcept = 0;
    [[nodiscard]] virtual std::string GetClipboardText() const;
    virtual void SetClipboardText(const std::string& text);
    virtual void SetWindowIcon(const uint8_t* rgba_pixels, uint32_t width, uint32_t height);

    [[nodiscard]] static std::unique_ptr<IWindow> Create(const WindowDescriptor& descriptor);
};

inline void IWindow::SetWindowLimits(const WindowLimits& limits) {
    (void)limits;
}
inline void IWindow::SetCursor(WindowCursor cursor) {
    (void)cursor;
}
inline void IWindow::SetMonitor(uint32_t monitor_index) {
    (void)monitor_index;
}
inline uint32_t IWindow::GetMonitor() const noexcept {
    return 0;
}
inline float IWindow::GetDPIScale() const noexcept {
    return 1.0F;
}
inline uint32_t IWindow::GetFramebufferWidth() const noexcept {
    return static_cast<uint32_t>(static_cast<float>(GetWidth()) * GetDPIScale());
}
inline uint32_t IWindow::GetFramebufferHeight() const noexcept {
    return static_cast<uint32_t>(static_cast<float>(GetHeight()) * GetDPIScale());
}
inline void IWindow::SetTransparent(bool enabled) {
    (void)enabled;
}
inline bool IWindow::IsTransparent() const noexcept {
    return false;
}
inline void IWindow::SetVSync(bool enabled) {
    (void)enabled;
}
inline bool IWindow::IsVSyncEnabled() const noexcept {
    return false;
}
inline void IWindow::Minimize() {}
inline void IWindow::Maximize() {}
inline void IWindow::Restore() {}
inline NativeWindowHandle IWindow::GetNativeHandle() const noexcept {
    return {};
}
inline std::string IWindow::GetClipboardText() const {
    return {};
}
inline void IWindow::SetClipboardText(const std::string& text) {
    (void)text;
}
inline void IWindow::SetWindowIcon(const uint8_t* rgba_pixels, uint32_t width, uint32_t height) {
    (void)rgba_pixels;
    (void)width;
    (void)height;
}

}  // namespace vne::xwin
