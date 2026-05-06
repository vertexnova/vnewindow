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

void xdg_surface_configure_thunk(void* data, struct xdg_surface* xdg_surface, uint32_t serial) {
    (void)data;
    xdg_surface_ack_configure(xdg_surface, serial);
}

void xdg_toplevel_configure_thunk(void* data, struct xdg_toplevel*, int32_t width, int32_t height, struct wl_array*) {
    auto* self = static_cast<WaylandWindow_C*>(data);
    if (width > 0 && height > 0) {
        self->apply_toplevel_configure(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
    }
}

void xdg_toplevel_close_thunk(void* data, struct xdg_toplevel*) {
    auto* self = static_cast<WaylandWindow_C*>(data);
    self->apply_toplevel_close();
}

const xdg_surface_listener kXdgSurfaceListener = {
    .configure = xdg_surface_configure_thunk,
};

const xdg_toplevel_listener kXdgToplevelListener = {
    .configure = xdg_toplevel_configure_thunk,
    .close = xdg_toplevel_close_thunk,
};

}  // namespace

WaylandWindow_C::WaylandWindow_C() = default;

WaylandWindow_C::~WaylandWindow_C() {
    destroy_surfaces();
}

void WaylandWindow_C::SetOwner(WaylandWindowManager_C* owner) {
    owner_ = owner;
}

void WaylandWindow_C::destroy_surfaces() {
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

void WaylandWindow_C::apply_toplevel_configure(uint32_t width, uint32_t height) {
    desc_.size.width = width;
    desc_.size.height = height;
    if (owner_) {
        const EventBridgeCallbacks& cb = owner_->eventBridgeCallbacks();
        eventBridgeWindowResize(this, desc_, cb, width, height);
        WindowEventData ev{};
        ev.type = WindowEventType::eResize;
        ev.size = desc_.size;
        owner_->NotifyWindowEvent(this, ev);
    }
}

void WaylandWindow_C::apply_toplevel_close() {
    open_ = false;
    if (owner_) {
        WindowEventData ev{};
        ev.type = WindowEventType::eClose;
        owner_->NotifyWindowEvent(this, ev);
    }
}

void WaylandWindow_C::Initialize(const WindowDescriptor& descriptor) {
    destroy_surfaces();
    desc_ = descriptor;
    if (!owner_ || !owner_->NativeCompositor() || !owner_->NativeXdgWmBase()) {
        return;
    }
    surface_ = wl_compositor_create_surface(owner_->NativeCompositor());
    if (!surface_) {
        return;
    }
    xdg_surface_ = xdg_wm_base_get_xdg_surface(owner_->NativeXdgWmBase(), surface_);
    if (!xdg_surface_) {
        wl_surface_destroy(surface_);
        surface_ = nullptr;
        return;
    }
    xdg_surface_add_listener(xdg_surface_, &kXdgSurfaceListener, this);

    toplevel_ = xdg_surface_get_toplevel(xdg_surface_);
    if (!toplevel_) {
        destroy_surfaces();
        return;
    }
    xdg_toplevel_add_listener(toplevel_, &kXdgToplevelListener, this);
    if (!desc_.title.empty()) {
        xdg_toplevel_set_title(toplevel_, desc_.title.c_str());
    }
    if (desc_.limits.has_min_size || desc_.limits.has_max_size) {
        SetWindowLimits(desc_.limits);
    }
    wl_surface_commit(surface_);
    if (owner_->NativeDisplay()) {
        wl_display_roundtrip(owner_->NativeDisplay());
    }
    open_ = true;
}

void WaylandWindow_C::PollEvents() {
    if (owner_ && owner_->NativeDisplay()) {
        wl_display_dispatch_pending(owner_->NativeDisplay());
    }
}

void WaylandWindow_C::SwapBuffers() {}

void WaylandWindow_C::SetTitle(const std::string& title) {
    desc_.title = title;
    if (toplevel_) {
        xdg_toplevel_set_title(toplevel_, title.c_str());
        if (surface_ && owner_ && owner_->NativeDisplay()) {
            wl_surface_commit(surface_);
            wl_display_flush(owner_->NativeDisplay());
        }
    }
}

void WaylandWindow_C::SetWindowMode(WindowMode mode) {
    desc_.mode = mode;
}

WindowMode WaylandWindow_C::GetWindowMode() const {
    return desc_.mode;
}

void WaylandWindow_C::SetFullscreen(bool enabled) {
    if (!toplevel_) {
        return;
    }
    if (enabled) {
        xdg_toplevel_set_fullscreen(toplevel_, nullptr);
    } else {
        xdg_toplevel_unset_fullscreen(toplevel_);
    }
    if (surface_ && owner_ && owner_->NativeDisplay()) {
        wl_surface_commit(surface_);
        wl_display_flush(owner_->NativeDisplay());
    }
    fullscreen_ = enabled;
}

bool WaylandWindow_C::IsFullscreen() const {
    return fullscreen_;
}

void WaylandWindow_C::SetPosition(int x, int y) {
    desc_.position.x = x;
    desc_.position.y = y;
}

void WaylandWindow_C::GetPosition(int& x, int& y) const {
    x = desc_.position.x;
    y = desc_.position.y;
}

void WaylandWindow_C::Resize(uint32_t width, uint32_t height) {
    desc_.size.width = width;
    desc_.size.height = height;
    if (surface_ && owner_ && owner_->NativeDisplay()) {
        wl_surface_commit(surface_);
        wl_display_flush(owner_->NativeDisplay());
    }
}

void WaylandWindow_C::Close() {
    destroy_surfaces();
}

bool WaylandWindow_C::IsOpen() const {
    return open_ && surface_ != nullptr;
}

void* WaylandWindow_C::GetNativeWindow() const {
    return surface_;
}

NativeWindowHandle WaylandWindow_C::GetNativeHandle() const {
    NativeWindowHandle handle{};
    handle.api = WindowAPI::eWaylandWindow;
    handle.wl_display = owner_ ? owner_->NativeDisplay() : nullptr;
    handle.wl_surface = surface_;
    return handle;
}

WindowAPI WaylandWindow_C::GetWindowAPI() const {
    return WindowAPI::eWaylandWindow;
}

int WaylandWindow_C::GetWidth() const {
    return static_cast<int>(desc_.size.width);
}

int WaylandWindow_C::GetHeight() const {
    return static_cast<int>(desc_.size.height);
}

float WaylandWindow_C::GetDPIScale() const {
    return owner_ ? owner_->OutputScale() : 1.0F;
}

void WaylandWindow_C::Minimize() {
    if (!toplevel_) {
        return;
    }
    xdg_toplevel_set_minimized(toplevel_);
    if (surface_ && owner_ && owner_->NativeDisplay()) {
        wl_surface_commit(surface_);
        wl_display_flush(owner_->NativeDisplay());
    }
}

void WaylandWindow_C::Maximize() {
    if (!toplevel_) {
        return;
    }
    xdg_toplevel_set_maximized(toplevel_);
    if (surface_ && owner_ && owner_->NativeDisplay()) {
        wl_surface_commit(surface_);
        wl_display_flush(owner_->NativeDisplay());
    }
}

void WaylandWindow_C::Restore() {
    if (!toplevel_) {
        return;
    }
    if (fullscreen_) {
        xdg_toplevel_unset_fullscreen(toplevel_);
        fullscreen_ = false;
    }
    xdg_toplevel_unset_maximized(toplevel_);
    if (surface_ && owner_ && owner_->NativeDisplay()) {
        wl_surface_commit(surface_);
        wl_display_flush(owner_->NativeDisplay());
    }
}

void WaylandWindow_C::SetWindowLimits(const WindowLimits& limits) {
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
    if (surface_ && owner_ && owner_->NativeDisplay()) {
        wl_surface_commit(surface_);
        wl_display_flush(owner_->NativeDisplay());
    }
}

void WaylandWindow_C::SetCursor(WindowCursor cursor) {
    // Cursor images require wl_shm + wl_cursor; compositors may also offer cursor-shape-v1.
    // Until those globals are bound, this is a deliberate no-op.
    (void)cursor;
}

}  // namespace vne::xwin
