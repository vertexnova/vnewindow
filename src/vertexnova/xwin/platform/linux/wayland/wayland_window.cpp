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
#include "xwin_vne_events_bridge.h"

#include <cstring>

#include <wayland-client.h>

extern "C" {
#include "xdg-shell-client-protocol.h"
}

namespace vne::xwin {

namespace {

void xdg_surface_configure_thunk(void* data, struct xdg_surface* xdg_surface, uint32_t serial) {
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
    xdg_surface_configure_thunk,
};

const xdg_toplevel_listener kXdgToplevelListener = {
    xdg_toplevel_configure_thunk,
    xdg_toplevel_close_thunk,
};

}  // namespace

WaylandWindow_C::WaylandWindow_C() = default;

WaylandWindow_C::~WaylandWindow_C() {
    destroy_surfaces();
}

void WaylandWindow_C::SetOwner(WaylandWindowManager_C* owner) {
    _owner = owner;
}

void WaylandWindow_C::destroy_surfaces() {
    if (_toplevel) {
        xdg_toplevel_destroy(_toplevel);
        _toplevel = nullptr;
    }
    if (_xdg_surface) {
        xdg_surface_destroy(_xdg_surface);
        _xdg_surface = nullptr;
    }
    if (_surface) {
        wl_surface_destroy(_surface);
        _surface = nullptr;
    }
    _open = false;
}

void WaylandWindow_C::apply_toplevel_configure(uint32_t width, uint32_t height) {
    _desc.size.width = width;
    _desc.size.height = height;
    if (_owner) {
        const XWinVneEventCallbacks_C& cb = _owner->vneEventCallbacks();
        xwinVneBridgeWindowResize(this, _desc, cb, width, height);
        WindowEventData_C ev{};
        ev.type = WindowEventType_TP::RESIZE;
        ev.size = _desc.size;
        _owner->NotifyWindowEvent(this, ev);
    }
}

void WaylandWindow_C::apply_toplevel_close() {
    _open = false;
    if (_owner) {
        WindowEventData_C ev{};
        ev.type = WindowEventType_TP::CLOSE;
        _owner->NotifyWindowEvent(this, ev);
    }
}

void WaylandWindow_C::Initialize(const WindowDescriptor_C& descriptor) {
    destroy_surfaces();
    _desc = descriptor;
    if (!_owner || !_owner->NativeCompositor() || !_owner->NativeXdgWmBase()) {
        return;
    }
    _surface = wl_compositor_create_surface(_owner->NativeCompositor());
    if (!_surface) {
        return;
    }
    _xdg_surface = xdg_wm_base_get_xdg_surface(_owner->NativeXdgWmBase(), _surface);
    if (!_xdg_surface) {
        wl_surface_destroy(_surface);
        _surface = nullptr;
        return;
    }
    xdg_surface_add_listener(_xdg_surface, &kXdgSurfaceListener, this);

    _toplevel = xdg_surface_get_toplevel(_xdg_surface);
    if (!_toplevel) {
        destroy_surfaces();
        return;
    }
    xdg_toplevel_add_listener(_toplevel, &kXdgToplevelListener, this);
    if (!_desc.title.empty()) {
        xdg_toplevel_set_title(_toplevel, _desc.title.c_str());
    }
    if (_desc.limits.has_min_size || _desc.limits.has_max_size) {
        SetWindowLimits(_desc.limits);
    }
    wl_surface_commit(_surface);
    if (_owner->NativeDisplay()) {
        wl_display_roundtrip(_owner->NativeDisplay());
    }
    _open = true;
}

void WaylandWindow_C::PollEvents() {
    if (_owner && _owner->NativeDisplay()) {
        wl_display_dispatch_pending(_owner->NativeDisplay());
    }
}

void WaylandWindow_C::SwapBuffers() {}

void WaylandWindow_C::SetTitle(const std::string& title) {
    _desc.title = title;
    if (_toplevel) {
        xdg_toplevel_set_title(_toplevel, title.c_str());
        if (_surface && _owner && _owner->NativeDisplay()) {
            wl_surface_commit(_surface);
            wl_display_flush(_owner->NativeDisplay());
        }
    }
}

void WaylandWindow_C::SetWindowMode(WindowMode_TP mode) {
    _desc.mode = mode;
}

WindowMode_TP WaylandWindow_C::GetWindowMode() const {
    return _desc.mode;
}

void WaylandWindow_C::SetFullscreen(bool enabled) {
    if (!_toplevel) { return; }
    if (enabled) {
        xdg_toplevel_set_fullscreen(_toplevel, nullptr);
    } else {
        xdg_toplevel_unset_fullscreen(_toplevel);
    }
    if (_surface && _owner && _owner->NativeDisplay()) {
        wl_surface_commit(_surface);
        wl_display_flush(_owner->NativeDisplay());
    }
    _fullscreen = enabled;
}

bool WaylandWindow_C::IsFullscreen() const {
    return _fullscreen;
}

void WaylandWindow_C::SetPosition(int x, int y) {
    _desc.position.x = x;
    _desc.position.y = y;
}

void WaylandWindow_C::GetPosition(int& x, int& y) const {
    x = _desc.position.x;
    y = _desc.position.y;
}

void WaylandWindow_C::Resize(uint32_t width, uint32_t height) {
    _desc.size.width = width;
    _desc.size.height = height;
    if (_surface && _owner && _owner->NativeDisplay()) {
        wl_surface_commit(_surface);
        wl_display_flush(_owner->NativeDisplay());
    }
}

void WaylandWindow_C::Close() {
    destroy_surfaces();
}

bool WaylandWindow_C::IsOpen() const {
    return _open && _surface != nullptr;
}

void* WaylandWindow_C::GetNativeWindow() const {
    return _surface;
}

WindowAPI_TP WaylandWindow_C::GetWindowAPI() const {
    return WindowAPI_TP::WAYLAND_WINDOW;
}

int WaylandWindow_C::GetWidth() const {
    return static_cast<int>(_desc.size.width);
}

int WaylandWindow_C::GetHeight() const {
    return static_cast<int>(_desc.size.height);
}

void WaylandWindow_C::Minimize() {
    if (!_toplevel) { return; }
    xdg_toplevel_set_minimized(_toplevel);
    if (_surface && _owner && _owner->NativeDisplay()) {
        wl_surface_commit(_surface);
        wl_display_flush(_owner->NativeDisplay());
    }
}

void WaylandWindow_C::Maximize() {
    if (!_toplevel) { return; }
    xdg_toplevel_set_maximized(_toplevel);
    if (_surface && _owner && _owner->NativeDisplay()) {
        wl_surface_commit(_surface);
        wl_display_flush(_owner->NativeDisplay());
    }
}

void WaylandWindow_C::Restore() {
    if (!_toplevel) { return; }
    if (_fullscreen) {
        xdg_toplevel_unset_fullscreen(_toplevel);
        _fullscreen = false;
    }
    xdg_toplevel_unset_maximized(_toplevel);
    if (_surface && _owner && _owner->NativeDisplay()) {
        wl_surface_commit(_surface);
        wl_display_flush(_owner->NativeDisplay());
    }
}

void WaylandWindow_C::SetWindowLimits(const WindowLimits_C& limits) {
    _desc.limits = limits;
    if (!_toplevel) { return; }
    if (limits.has_min_size) {
        xdg_toplevel_set_min_size(_toplevel,
                                    static_cast<int32_t>(limits.min_size.width),
                                    static_cast<int32_t>(limits.min_size.height));
    } else {
        xdg_toplevel_set_min_size(_toplevel, 0, 0);
    }
    if (limits.has_max_size) {
        xdg_toplevel_set_max_size(_toplevel,
                                  static_cast<int32_t>(limits.max_size.width),
                                  static_cast<int32_t>(limits.max_size.height));
    } else {
        xdg_toplevel_set_max_size(_toplevel, 0, 0);
    }
    if (_surface && _owner && _owner->NativeDisplay()) {
        wl_surface_commit(_surface);
        wl_display_flush(_owner->NativeDisplay());
    }
}

void WaylandWindow_C::SetCursor(WindowCursor_TP cursor) {
    // Cursor images require wl_shm + wl_cursor; compositors may also offer cursor-shape-v1.
    // Until those globals are bound, this is a deliberate no-op.
    (void)cursor;
}

}  // namespace vne::xwin
