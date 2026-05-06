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

#include "vertexnova/xwin/window.h"

#include <string>

struct wl_surface;
struct xdg_surface;
struct xdg_toplevel;

namespace vne::xwin {

class WaylandWindowManager_C;

class WaylandWindow_C final : public IWindow {
   public:
    WaylandWindow_C();
    ~WaylandWindow_C() override;

    void SetOwner(WaylandWindowManager_C* owner);
    const WindowDescriptor& descriptor() const { return desc_; }

    void Initialize(const WindowDescriptor& descriptor) override;
    void PollEvents() override;
    void SwapBuffers() override;
    void SetTitle(const std::string& title) override;
    void SetWindowMode(WindowMode mode) override;
    WindowMode GetWindowMode() const override;
    void SetFullscreen(bool enabled) override;
    bool IsFullscreen() const override;
    void Minimize() override;
    void Maximize() override;
    void Restore() override;
    void SetWindowLimits(const WindowLimits& limits) override;
    void SetCursor(WindowCursor cursor) override;
    void SetPosition(int x, int y) override;
    void GetPosition(int& x, int& y) const override;
    void Resize(uint32_t width, uint32_t height) override;
    void Close() override;
    bool IsOpen() const override;
    void* GetNativeWindow() const override;
    NativeWindowHandle GetNativeHandle() const override;
    WindowAPI GetWindowAPI() const override;
    int GetWidth() const override;
    int GetHeight() const override;
    float GetDPIScale() const override;

    /** Called from Wayland listener thunks (xdg_toplevel_listener). */
    void apply_toplevel_configure(uint32_t width, uint32_t height);
    void apply_toplevel_close();

    /** Same pointer registered with the compositor (keyboard/pointer focus mapping). */
    wl_surface* native_surface() const { return surface_; }

   private:
    void destroy_surfaces();

    WaylandWindowManager_C* owner_ = nullptr;
    WindowDescriptor desc_{};
    bool open_ = false;
    bool fullscreen_ = false;
    wl_surface* surface_ = nullptr;
    xdg_surface* xdg_surface_ = nullptr;
    xdg_toplevel* toplevel_ = nullptr;
};

}  // namespace vne::xwin
