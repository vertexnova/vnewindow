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

class WaylandWindow_C final : public Window_I {
   public:
    WaylandWindow_C();
    ~WaylandWindow_C() override;

    void SetOwner(WaylandWindowManager_C* owner);
    const WindowDescriptor_C& descriptor() const { return _desc; }

    void Initialize(const WindowDescriptor_C& descriptor) override;
    void PollEvents() override;
    void SwapBuffers() override;
    void SetTitle(const std::string& title) override;
    void SetWindowMode(WindowMode_TP mode) override;
    WindowMode_TP GetWindowMode() const override;
    void SetFullscreen(bool enabled) override;
    bool IsFullscreen() const override;
    void Minimize() override;
    void Maximize() override;
    void Restore() override;
    void SetWindowLimits(const WindowLimits_C& limits) override;
    void SetCursor(WindowCursor_TP cursor) override;
    void SetPosition(int x, int y) override;
    void GetPosition(int& x, int& y) const override;
    void Resize(uint32_t width, uint32_t height) override;
    void Close() override;
    bool IsOpen() const override;
    void* GetNativeWindow() const override;
    WindowAPI_TP GetWindowAPI() const override;
    int GetWidth() const override;
    int GetHeight() const override;

   private:
    void apply_toplevel_configure(uint32_t width, uint32_t height);
    void apply_toplevel_close();
    void destroy_surfaces();

    /** @brief Same pointer registered with the compositor (keyboard/pointer focus mapping). */
    wl_surface* native_surface() const { return _surface; }

    WaylandWindowManager_C* _owner = nullptr;
    WindowDescriptor_C _desc{};
    bool _open = false;
    bool _fullscreen = false;
    wl_surface* _surface = nullptr;
    xdg_surface* _xdg_surface = nullptr;
    xdg_toplevel* _toplevel = nullptr;
};

}  // namespace vne::xwin
