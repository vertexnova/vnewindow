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

    void initialize(const WindowDescriptor& descriptor) override;
    void pollEvents() override;
    void swapBuffers() override;
    void setTitle(const std::string& title) override;
    void setWindowMode(WindowMode mode) override;
    [[nodiscard]] WindowMode getWindowMode() const noexcept override;
    void setFullscreen(bool enabled) override;
    [[nodiscard]] bool isFullscreen() const noexcept override;
    void minimize() override;
    void maximize() override;
    void restore() override;
    void setWindowLimits(const WindowLimits& limits) override;
    void setCursor(WindowCursor cursor) override;
    void setPosition(int x, int y) override;
    void getPosition(int& x, int& y) const override;
    void resize(uint32_t width, uint32_t height) override;
    void close() override;
    [[nodiscard]] bool isOpen() const noexcept override;
    [[nodiscard]] NativeWindowHandle getNativeHandle() const noexcept override;
    [[nodiscard]] WindowAPI getWindowAPI() const noexcept override;
    [[nodiscard]] int getWidth() const noexcept override;
    [[nodiscard]] int getHeight() const noexcept override;
    [[nodiscard]] float getDpiScale() const noexcept override;

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
