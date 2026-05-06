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

class WaylandWindowManager;

class WaylandWindow final : public IWindow {
   public:
    WaylandWindow();
    ~WaylandWindow() override;

    void setEventOwner(WaylandWindowManager* owner);
    [[nodiscard]] const WindowDescriptor& descriptor() const noexcept { return desc_; }

    void Initialize(const WindowDescriptor& descriptor) override;
    void PollEvents() override;
    void SwapBuffers() override;
    void SetTitle(const std::string& title) override;
    void SetWindowMode(WindowMode mode) override;
    [[nodiscard]] WindowMode GetWindowMode() const noexcept override;
    void SetFullscreen(bool enabled) override;
    [[nodiscard]] bool IsFullscreen() const noexcept override;
    void Minimize() override;
    void Maximize() override;
    void Restore() override;
    void SetWindowLimits(const WindowLimits& limits) override;
    void SetCursor(WindowCursor cursor) override;
    void SetPosition(int x, int y) override;
    void GetPosition(int& x, int& y) const override;
    void Resize(uint32_t width, uint32_t height) override;
    void Close() override;
    [[nodiscard]] bool IsOpen() const noexcept override;
    [[nodiscard]] void* GetNativeWindow() const noexcept override;
    [[nodiscard]] NativeWindowHandle GetNativeHandle() const noexcept override;
    [[nodiscard]] WindowAPI GetWindowAPI() const noexcept override;
    [[nodiscard]] int GetWidth() const noexcept override;
    [[nodiscard]] int GetHeight() const noexcept override;
    [[nodiscard]] float GetDPIScale() const noexcept override;

    /** Called from Wayland listener thunks (xdg_toplevel_listener). */
    void applyToplevelConfigure(uint32_t width, uint32_t height);
    void applyToplevelClose();

    /** Same pointer registered with the compositor (keyboard/pointer focus mapping). */
    [[nodiscard]] wl_surface* nativeSurface() const noexcept { return surface_; }

   private:
    void destroySurfaces();

    WaylandWindowManager* owner_ = nullptr;
    WindowDescriptor desc_{};
    bool open_ = false;
    bool fullscreen_ = false;
    wl_surface* surface_ = nullptr;
    xdg_surface* xdg_surface_ = nullptr;
    xdg_toplevel* toplevel_ = nullptr;
};

}  // namespace vne::xwin
