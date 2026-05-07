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

#include "wayland_window.h"

#include "wayland_window_manager.h"
#include "event_bridge.h"

#include <cstring>

#include <wayland-client.h>

extern "C" {
#include "xdg-shell-client-protocol.h"
}

namespace vne::xwin {

namespace {

void xdgSurfaceConfigureThunk(void* data, struct xdg_surface* xdg_surface, uint32_t serial) {
    (void)data;
    xdg_surface_ack_configure(xdg_surface, serial);
}

void xdgToplevelConfigureThunk(void* data, struct xdg_toplevel*, int32_t width, int32_t height, struct wl_array*) {
    auto* self = static_cast<WaylandWindow*>(data);
    if (width > 0 && height > 0) {
        self->applyToplevelConfigure(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    }
}

void xdgToplevelCloseThunk(void* data, struct xdg_toplevel*) {
    auto* self = static_cast<WaylandWindow*>(data);
    self->applyToplevelClose();
}

void xdgToplevelConfigureBoundsThunk(void* data, struct xdg_toplevel*, int32_t width, int32_t height) {
    (void)data;
    (void)width;
    (void)height;
}

/* Value-init then assign: avoids -Wmissing-field-initializers when protocols add fields
 * (designated-only lists still warn on Clang for omitted trailing members). */
const xdg_surface_listener kXdgSurfaceListener = [] {
    xdg_surface_listener l{};
    l.configure = xdgSurfaceConfigureThunk;
    return l;
}();

const xdg_toplevel_listener kXdgToplevelListener = [] {
    xdg_toplevel_listener l{};
    l.configure = xdgToplevelConfigureThunk;
    l.close = xdgToplevelCloseThunk;
#if defined(XDG_TOPLEVEL_CONFIGURE_BOUNDS_SINCE_VERSION)
    l.configure_bounds = xdgToplevelConfigureBoundsThunk;
#endif
    return l;
}();

}  // namespace

WaylandWindow::WaylandWindow() = default;

WaylandWindow::~WaylandWindow() {
    destroySurfaces();
}

void WaylandWindow::setEventOwner(WaylandWindowManager* owner) {
    owner_ = owner;
}

void WaylandWindow::destroySurfaces() {
    if (toplevel_) {
        xdg_toplevel_destroy(toplevel_);
        toplevel_ = nullptr;
    }
    if (xdg_surface_) {
        xdg_surface_destroy(xdg_surface_);
        xdg_surface_ = nullptr;
    }
    if (surface_) {
        wl_surface_destroy(surface_);
        surface_ = nullptr;
    }
    open_ = false;
}

void WaylandWindow::applyToplevelConfigure(uint32_t width, uint32_t height) {
    desc_.size.width = width;
    desc_.size.height = height;
    if (owner_) {
        const EventBridgeCallbacks& cb = owner_->eventBridgeCallbacks();
        eventBridgeWindowResize(this, desc_, cb, width, height);
        WindowEventData ev{};
        ev.type = WindowEventType::eResize;
        ev.size = desc_.size;
        owner_->notifyWindowEvent(this, ev);
    }
}

void WaylandWindow::applyToplevelClose() {
    open_ = false;
    if (owner_) {
        WindowEventData ev{};
        ev.type = WindowEventType::eClose;
        owner_->notifyWindowEvent(this, ev);
    }
}

void WaylandWindow::initialize(const WindowDescriptor& descriptor) {
    destroySurfaces();
    desc_ = descriptor;
    if (!owner_ || !owner_->nativeCompositor() || !owner_->nativeXdgWmBase()) {
        return;
    }
    surface_ = wl_compositor_create_surface(owner_->nativeCompositor());
    if (!surface_) {
        return;
    }
    xdg_surface_ = xdg_wm_base_get_xdg_surface(owner_->nativeXdgWmBase(), surface_);
    if (!xdg_surface_) {
        wl_surface_destroy(surface_);
        surface_ = nullptr;
        return;
    }
    xdg_surface_add_listener(xdg_surface_, &kXdgSurfaceListener, this);

    toplevel_ = xdg_surface_get_toplevel(xdg_surface_);
    if (!toplevel_) {
        destroySurfaces();
        return;
    }
    xdg_toplevel_add_listener(toplevel_, &kXdgToplevelListener, this);
    if (!desc_.title.empty()) {
        xdg_toplevel_set_title(toplevel_, desc_.title.c_str());
    }
    if (desc_.limits.has_min_size || desc_.limits.has_max_size) {
        setWindowLimits(desc_.limits);
    }
    wl_surface_commit(surface_);
    if (owner_->nativeDisplay()) {
        wl_display_roundtrip(owner_->nativeDisplay());
    }
    open_ = true;
}

void WaylandWindow::pollEvents() {
    if (owner_ && owner_->nativeDisplay()) {
        // Read from the Wayland socket and dispatch; dispatch_pending alone only drains the local queue.
        wl_display_dispatch(owner_->nativeDisplay());
    }
}

void WaylandWindow::swapBuffers() {}

void WaylandWindow::setTitle(const std::string& title) {
    desc_.title = title;
    if (toplevel_) {
        xdg_toplevel_set_title(toplevel_, title.c_str());
        if (surface_ && owner_ && owner_->nativeDisplay()) {
            wl_surface_commit(surface_);
            wl_display_flush(owner_->nativeDisplay());
        }
    }
}

void WaylandWindow::setWindowMode(WindowMode mode) {
    switch (mode) {
        case WindowMode::eFullscreen:
            setFullscreen(true);
            break;
        case WindowMode::eWindowed:
            setFullscreen(false);
            if (toplevel_) {
                xdg_toplevel_unset_maximized(toplevel_);
            }
            if (surface_ && owner_ && owner_->nativeDisplay()) {
                wl_surface_commit(surface_);
                wl_display_flush(owner_->nativeDisplay());
            }
            desc_.mode = WindowMode::eWindowed;
            desc_.state = WindowState::eNormal;
            break;
        case WindowMode::eMaximized:
            maximize();
            break;
        case WindowMode::eBorderless:
            setFullscreen(false);
            if (toplevel_) {
                xdg_toplevel_unset_maximized(toplevel_);
            }
            if (surface_ && owner_ && owner_->nativeDisplay()) {
                wl_surface_commit(surface_);
                wl_display_flush(owner_->nativeDisplay());
            }
            desc_.mode = WindowMode::eBorderless;
            desc_.state = WindowState::eNormal;
            break;
    }
}

WindowMode WaylandWindow::getWindowMode() const noexcept {
    return desc_.mode;
}

void WaylandWindow::setFullscreen(bool enabled) {
    if (!toplevel_) {
        return;
    }
    if (enabled) {
        xdg_toplevel_set_fullscreen(toplevel_, nullptr);
    } else {
        xdg_toplevel_unset_fullscreen(toplevel_);
    }
    if (surface_ && owner_ && owner_->nativeDisplay()) {
        wl_surface_commit(surface_);
        wl_display_flush(owner_->nativeDisplay());
    }
    fullscreen_ = enabled;
    if (enabled) {
        desc_.mode = WindowMode::eFullscreen;
        desc_.state = WindowState::eFullscreen;
    } else if (desc_.mode == WindowMode::eFullscreen) {
        desc_.mode = WindowMode::eWindowed;
        desc_.state = WindowState::eNormal;
    }
}

bool WaylandWindow::isFullscreen() const noexcept {
    return fullscreen_;
}

void WaylandWindow::setPosition(int x, int y) {
    desc_.position.x = x;
    desc_.position.y = y;
}

WindowPosition WaylandWindow::getPosition() const {
    return desc_.position;
}

void WaylandWindow::resize(uint32_t width, uint32_t height) {
    desc_.size.width = width;
    desc_.size.height = height;
    if (surface_ && owner_ && owner_->nativeDisplay()) {
        wl_surface_commit(surface_);
        wl_display_flush(owner_->nativeDisplay());
    }
}

void WaylandWindow::close() {
    destroySurfaces();
}

bool WaylandWindow::isOpen() const noexcept {
    return open_ && surface_ != nullptr;
}

NativeWindowHandle WaylandWindow::getNativeHandle() const noexcept {
    NativeWindowHandle handle{};
    handle.api = WindowAPI::eWaylandWindow;
    handle.wl_display = owner_ ? owner_->nativeDisplay() : nullptr;
    handle.wl_surface = surface_;
    return handle;
}

WindowAPI WaylandWindow::getWindowAPI() const noexcept {
    return WindowAPI::eWaylandWindow;
}

int WaylandWindow::getWidth() const noexcept {
    return static_cast<int>(desc_.size.width);
}

int WaylandWindow::getHeight() const noexcept {
    return static_cast<int>(desc_.size.height);
}

float WaylandWindow::getDpiScale() const noexcept {
    return owner_ ? owner_->outputScale() : 1.0F;
}

void WaylandWindow::minimize() {
    if (!toplevel_) {
        return;
    }
    xdg_toplevel_set_minimized(toplevel_);
    if (surface_ && owner_ && owner_->nativeDisplay()) {
        wl_surface_commit(surface_);
        wl_display_flush(owner_->nativeDisplay());
    }
    desc_.state = WindowState::eMinimized;
}

void WaylandWindow::maximize() {
    if (!toplevel_) {
        return;
    }
    xdg_toplevel_set_maximized(toplevel_);
    if (surface_ && owner_ && owner_->nativeDisplay()) {
        wl_surface_commit(surface_);
        wl_display_flush(owner_->nativeDisplay());
    }
    desc_.mode = WindowMode::eMaximized;
    desc_.state = WindowState::eMaximized;
}

void WaylandWindow::restore() {
    if (!toplevel_) {
        return;
    }
    if (fullscreen_) {
        xdg_toplevel_unset_fullscreen(toplevel_);
        fullscreen_ = false;
    }
    xdg_toplevel_unset_maximized(toplevel_);
    if (surface_ && owner_ && owner_->nativeDisplay()) {
        wl_surface_commit(surface_);
        wl_display_flush(owner_->nativeDisplay());
    }
    desc_.mode = WindowMode::eWindowed;
    desc_.state = WindowState::eNormal;
}

void WaylandWindow::setWindowLimits(const WindowLimits& limits) {
    desc_.limits = limits;
    if (!toplevel_) {
        return;
    }
    if (limits.has_min_size) {
        xdg_toplevel_set_min_size(toplevel_,
                                  static_cast<int32_t>(limits.min_size.width),
                                  static_cast<int32_t>(limits.min_size.height));
    } else {
        xdg_toplevel_set_min_size(toplevel_, 0, 0);
    }
    if (limits.has_max_size) {
        xdg_toplevel_set_max_size(toplevel_,
                                  static_cast<int32_t>(limits.max_size.width),
                                  static_cast<int32_t>(limits.max_size.height));
    } else {
        xdg_toplevel_set_max_size(toplevel_, 0, 0);
    }
    if (surface_ && owner_ && owner_->nativeDisplay()) {
        wl_surface_commit(surface_);
        wl_display_flush(owner_->nativeDisplay());
    }
}

void WaylandWindow::setCursor(WindowCursor cursor) {
    // Cursor images require wl_shm + wl_cursor; compositors may also offer cursor-shape-v1.
    // Until those globals are bound, this is a deliberate no-op.
    (void)cursor;
}

}  // namespace vne::xwin
