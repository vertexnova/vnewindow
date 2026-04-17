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

#include "vertexnova/xwin/window_descriptor.h"
#include "vertexnova/xwin/xwin_types.h"

#include <functional>
#include <memory>
#include <string>

namespace vne::xwin {

struct WindowEventData_C;

using WindowEventCallback_T = std::function<void(const WindowEventData_C&)>;

/**
 * @brief Abstract window; graphics presents externally — SwapBuffers may be a no-op.
 */
class Window_I {
   public:
    virtual ~Window_I() = default;

    virtual void Initialize(const WindowDescriptor_C& descriptor) = 0;
    virtual void PollEvents() = 0;
    /** @brief No-op unless a platform GL context is owned by this window. */
    virtual void SwapBuffers() = 0;

    virtual void SetTitle(const std::string& title) = 0;
    virtual void SetWindowMode(WindowMode_TP mode) = 0;
    virtual WindowMode_TP GetWindowMode() const = 0;
    virtual void SetFullscreen(bool enabled) = 0;
    virtual bool IsFullscreen() const = 0;
    virtual void SetPosition(int x, int y) = 0;
    virtual void GetPosition(int& x, int& y) const = 0;
    virtual void SetWindowLimits(const WindowLimits_C& limits);
    virtual void SetCursor(WindowCursor_TP cursor);
    virtual void SetMonitor(uint32_t monitor_index);
    virtual uint32_t GetMonitor() const;
    virtual float GetDPIScale() const;
    virtual uint32_t GetFramebufferWidth() const;
    virtual uint32_t GetFramebufferHeight() const;
    virtual void SetTransparent(bool enabled);
    virtual bool IsTransparent() const;
    virtual void SetVSync(bool enabled);
    virtual bool IsVSyncEnabled() const;
    virtual void Minimize();
    virtual void Maximize();
    virtual void Restore();
    virtual void Resize(uint32_t width, uint32_t height) = 0;
    virtual void Close() = 0;
    virtual bool IsOpen() const = 0;
    virtual void* GetNativeWindow() const = 0;
    virtual WindowAPI_TP GetWindowAPI() const = 0;
    virtual int GetWidth() const = 0;
    virtual int GetHeight() const = 0;

    static std::unique_ptr<Window_I> Create(const WindowDescriptor_C& descriptor);
};

inline void Window_I::SetWindowLimits(const WindowLimits_C& limits) {
    (void)limits;
}
inline void Window_I::SetCursor(WindowCursor_TP cursor) {
    (void)cursor;
}
inline void Window_I::SetMonitor(uint32_t monitor_index) {
    (void)monitor_index;
}
inline uint32_t Window_I::GetMonitor() const {
    return 0;
}
inline float Window_I::GetDPIScale() const {
    return 1.0F;
}
inline uint32_t Window_I::GetFramebufferWidth() const {
    return static_cast<uint32_t>(static_cast<float>(GetWidth()) * GetDPIScale());
}
inline uint32_t Window_I::GetFramebufferHeight() const {
    return static_cast<uint32_t>(static_cast<float>(GetHeight()) * GetDPIScale());
}
inline void Window_I::SetTransparent(bool enabled) {
    (void)enabled;
}
inline bool Window_I::IsTransparent() const {
    return false;
}
inline void Window_I::SetVSync(bool enabled) {
    (void)enabled;
}
inline bool Window_I::IsVSyncEnabled() const {
    return false;
}
inline void Window_I::Minimize() {}
inline void Window_I::Maximize() {}
inline void Window_I::Restore() {}

}  // namespace vne::xwin
