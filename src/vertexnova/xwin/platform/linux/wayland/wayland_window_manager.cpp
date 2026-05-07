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

#include "wayland_window_manager.h"

#include "wayland_map_key.h"
#include "wayland_window.h"
#include "event_bridge.h"

#include <vertexnova/xwin/input_mapping.h>

#include <algorithm>
#include <chrono>
#include <cstring>
#include <sys/mman.h>
#include <thread>
#include <unistd.h>

#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>

extern "C" {
#include "xdg-shell-client-protocol.h"
}

namespace vne::xwin {

// ---------------------------------------------------------------------------
// Static C thunks for Wayland listeners
// ---------------------------------------------------------------------------

namespace {

// ---- Registry ----

void registry_global(void* data, struct wl_registry* reg, uint32_t name, const char* iface, uint32_t ver) {
    static_cast<WaylandWindowManager*>(data)->onRegistryGlobal(reg, name, iface, ver);
}
void registry_global_remove(void* data, struct wl_registry*, uint32_t name) {
    static_cast<WaylandWindowManager*>(data)->onRegistryGlobalRemove(name);
}

const wl_registry_listener kRegistryListener = [] {
    wl_registry_listener l{};
    l.global = registry_global;
    l.global_remove = registry_global_remove;
    return l;
}();

void output_geometry(
    void*, struct wl_output*, int32_t, int32_t, int32_t, int32_t, int32_t, const char*, const char*, int32_t) {}
void output_mode(void*, struct wl_output*, uint32_t, int32_t, int32_t, int32_t) {}
void output_done(void*, struct wl_output*) {}
void output_scale(void* data, struct wl_output* output, int32_t factor) {
    auto* self = static_cast<WaylandWindowManager*>(data);
    self->onOutputScale(output, factor);
}
const wl_output_listener kOutputListener = [] {
    wl_output_listener l{};
    l.geometry = output_geometry;
    l.mode = output_mode;
    l.done = output_done;
    l.scale = output_scale;
    return l;
}();

// ---- xdg_wm_base ping ----

void xdg_ping(void*, struct xdg_wm_base* base, uint32_t serial) {
    xdg_wm_base_pong(base, serial);
}
const xdg_wm_base_listener kXdgWmBaseListener = [] {
    xdg_wm_base_listener l{};
    l.ping = xdg_ping;
    return l;
}();

// ---- wl_seat ----

void seat_capabilities(void* data, struct wl_seat* seat, uint32_t caps) {
    static_cast<WaylandWindowManager*>(data)->onSeatCapabilities(seat, caps);
}
void seat_name(void*, struct wl_seat*, const char*) {}

const wl_seat_listener kSeatListener = [] {
    wl_seat_listener l{};
    l.capabilities = seat_capabilities;
    l.name = seat_name;
    return l;
}();

// ---- wl_keyboard ----

void kb_keymap(void* data, struct wl_keyboard*, uint32_t format, int32_t fd, uint32_t size) {
    static_cast<WaylandWindowManager*>(data)->onKeyboardKeymap(format, fd, size);
}

void kb_enter(
    void* data, struct wl_keyboard*, uint32_t /*serial*/, struct wl_surface* surface, struct wl_array* /*keys*/) {
    static_cast<WaylandWindowManager*>(data)->onKeyboardEnter(surface);
}

void kb_leave(void* data, struct wl_keyboard*, uint32_t /*serial*/, struct wl_surface* surface) {
    static_cast<WaylandWindowManager*>(data)->onKeyboardLeave(surface);
}

void kb_key(void* data, struct wl_keyboard*, uint32_t /*serial*/, uint32_t /*time*/, uint32_t key, uint32_t state) {
    static_cast<WaylandWindowManager*>(data)->onKey(key, state, 0);
}

void kb_modifiers(void* data,
                  struct wl_keyboard*,
                  uint32_t /*serial*/,
                  uint32_t depressed,
                  uint32_t latched,
                  uint32_t locked,
                  uint32_t group) {
    static_cast<WaylandWindowManager*>(data)->onModifiers(depressed, latched, locked, group);
}

void kb_repeat_info(void*, struct wl_keyboard*, int32_t, int32_t) {}

const wl_keyboard_listener kKeyboardListener = [] {
    wl_keyboard_listener l{};
    l.keymap = kb_keymap;
    l.enter = kb_enter;
    l.leave = kb_leave;
    l.key = kb_key;
    l.modifiers = kb_modifiers;
    l.repeat_info = kb_repeat_info;
    return l;
}();

// ---- wl_pointer ----

void ptr_enter(void*, struct wl_pointer*, uint32_t, struct wl_surface*, wl_fixed_t sx, wl_fixed_t sy) {
    (void)sx;
    (void)sy;
}
void ptr_leave(void*, struct wl_pointer*, uint32_t, struct wl_surface*) {}

void ptr_motion(void* data, struct wl_pointer*, uint32_t /*time*/, wl_fixed_t sx, wl_fixed_t sy) {
    static_cast<WaylandWindowManager*>(data)->onPointerMotion(wl_fixed_to_double(sx), wl_fixed_to_double(sy));
}

void ptr_button(
    void* data, struct wl_pointer*, uint32_t /*serial*/, uint32_t /*time*/, uint32_t button, uint32_t state) {
    // We don't have separate coords in the button event; use last known position
    // stored by onPointerMotion. Pass 0 here; manager will use cached coords.
    static_cast<WaylandWindowManager*>(data)->onPointerButton(button, state, -1.0, -1.0);
}

void ptr_axis(void* data, struct wl_pointer*, uint32_t /*time*/, uint32_t axis, wl_fixed_t value) {
    // axis 0 = vertical, axis 1 = horizontal
    const double v = wl_fixed_to_double(value) / -10.0;  // normalise to scroll steps
    if (axis == 0) {
        static_cast<WaylandWindowManager*>(data)->onPointerAxis(0.0, v);
    } else {
        static_cast<WaylandWindowManager*>(data)->onPointerAxis(v, 0.0);
    }
}

void ptr_frame(void*, struct wl_pointer*) {}
void ptr_axis_source(void*, struct wl_pointer*, uint32_t) {}
void ptr_axis_stop(void*, struct wl_pointer*, uint32_t, uint32_t) {}
void ptr_axis_discrete(void*, struct wl_pointer*, uint32_t, int32_t) {}

const wl_pointer_listener kPointerListener = [] {
    wl_pointer_listener l{};
    l.enter = ptr_enter;
    l.leave = ptr_leave;
    l.motion = ptr_motion;
    l.button = ptr_button;
    l.axis = ptr_axis;
    l.frame = ptr_frame;
    l.axis_source = ptr_axis_source;
    l.axis_stop = ptr_axis_stop;
    l.axis_discrete = ptr_axis_discrete;
    return l;
}();

// ---- wl_touch ----

void touch_down(void* data,
                struct wl_touch*,
                uint32_t /*serial*/,
                uint32_t /*time*/,
                struct wl_surface*,
                int32_t id,
                wl_fixed_t x,
                wl_fixed_t y) {
    static_cast<WaylandWindowManager*>(data)->onTouchDown(static_cast<uint32_t>(id),
                                                          wl_fixed_to_double(x),
                                                          wl_fixed_to_double(y));
}
void touch_up(void* data, struct wl_touch*, uint32_t /*serial*/, uint32_t /*time*/, int32_t id) {
    static_cast<WaylandWindowManager*>(data)->onTouchUp(static_cast<uint32_t>(id), 0.0, 0.0);
}
void touch_motion(void* data, struct wl_touch*, uint32_t /*time*/, int32_t id, wl_fixed_t x, wl_fixed_t y) {
    static_cast<WaylandWindowManager*>(data)->onTouchMotion(static_cast<uint32_t>(id),
                                                            wl_fixed_to_double(x),
                                                            wl_fixed_to_double(y));
}
void touch_frame(void*, struct wl_touch*) {}
void touch_cancel(void*, struct wl_touch*) {}
void touch_shape(void*, struct wl_touch*, int32_t, wl_fixed_t, wl_fixed_t) {}
void touch_orientation(void*, struct wl_touch*, int32_t, wl_fixed_t) {}

const wl_touch_listener kTouchListener = [] {
    wl_touch_listener l{};
    l.down = touch_down;
    l.up = touch_up;
    l.motion = touch_motion;
    l.frame = touch_frame;
    l.cancel = touch_cancel;
    l.shape = touch_shape;
    l.orientation = touch_orientation;
    return l;
}();

}  // namespace

// ---------------------------------------------------------------------------
// WaylandWindowManager — input dispatch helpers
// ---------------------------------------------------------------------------

WaylandWindow* WaylandWindowManager::focusedWindow() const {
    if (kbd_focus_surface_) {
        WaylandWindow* w = windowForSurface(kbd_focus_surface_);
        if (w) {
            return w;
        }
    }
    if (focused_) {
        return dynamic_cast<WaylandWindow*>(focused_.get());
    }
    if (primary_) {
        return dynamic_cast<WaylandWindow*>(primary_.get());
    }
    return nullptr;
}

WaylandWindow* WaylandWindowManager::windowForSurface(wl_surface* surface) const {
    if (!surface) {
        return nullptr;
    }
    for (const auto& w : windows_) {
        auto* wl = dynamic_cast<WaylandWindow*>(w.get());
        if (wl && wl->nativeSurface() == surface) {
            return wl;
        }
    }
    return nullptr;
}

void WaylandWindowManager::onKeyboardEnter(wl_surface* surface) {
    if (!surface) {
        return;
    }
    if (kbd_focus_surface_ == surface) {
        return;
    }
    if (kbd_focus_surface_ && kbd_focus_surface_ != surface) {
        WaylandWindow* prev = windowForSurface(kbd_focus_surface_);
        if (prev) {
            notifyWindowFocus(prev, false);
        }
    }
    kbd_focus_surface_ = surface;
    WaylandWindow* win = windowForSurface(surface);
    if (win) {
        for (auto& w : windows_) {
            if (w.get() == win) {
                focused_ = w;
                break;
            }
        }
        notifyWindowFocus(win, true);
    }
}

void WaylandWindowManager::onKeyboardLeave(wl_surface* surface) {
    if (!surface) {
        return;
    }
    if (kbd_focus_surface_ != surface) {
        return;
    }
    WaylandWindow* win = windowForSurface(surface);
    kbd_focus_surface_ = nullptr;
    if (win) {
        notifyWindowFocus(win, false);
    }
}

void WaylandWindowManager::notifyWindowFocus(WaylandWindow* win, bool focused) {
    if (!win) {
        return;
    }
    eventBridgeWindowFocus(win, win->descriptor(), event_bridge_callbacks_, focused);
    WindowEventData ev{};
    ev.type = WindowEventType::eFocus;
    ev.focused = focused;
    notifyWindowEvent(win, ev);
}

void WaylandWindowManager::onKeyboardKeymap(uint32_t format, int32_t fd, uint32_t size) {
    if (fd < 0) {
        return;
    }
    if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1 || size == 0U) {
        close(fd);
        return;
    }

    void* mapped = mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    if (mapped == MAP_FAILED) {
        return;
    }

    auto* keymap_str = static_cast<const char*>(mapped);
    if (!xkb_context_) {
        xkb_context_ = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    }
    if (!xkb_context_) {
        munmap(mapped, size);
        return;
    }

    xkb_keymap* new_keymap =
        xkb_keymap_new_from_string(xkb_context_, keymap_str, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
    munmap(mapped, size);
    if (!new_keymap) {
        return;
    }

    xkb_state* new_state = xkb_state_new(new_keymap);
    if (!new_state) {
        xkb_keymap_unref(new_keymap);
        return;
    }

    if (xkb_state_) {
        xkb_state_unref(xkb_state_);
    }
    if (xkb_keymap_) {
        xkb_keymap_unref(xkb_keymap_);
    }
    xkb_keymap_ = new_keymap;
    xkb_state_ = new_state;
    xkb_state_update_mask(xkb_state_, mod_depressed_, mod_latched_, mod_locked_, 0U, 0U, mod_group_);
}

void WaylandWindowManager::onKey(uint32_t linux_key, uint32_t state, uint32_t /*time*/) {
    WaylandWindow* win = focusedWindow();
    if (!win) {
        return;
    }
    if (!xkb_state_) {
        return;
    }
    const xkb_keycode_t xkb_keycode = static_cast<xkb_keycode_t>(linux_key + 8U);
    const xkb_keysym_t sym = xkb_state_key_get_one_sym(xkb_state_, xkb_keycode);
    if (sym == XKB_KEY_NoSymbol) {
        return;
    }
    const WindowDescriptor& desc = win->descriptor();
    const vne::events::KeyCode kc = mapNativeKeyToEvents(WindowAPI::eWaylandWindow,
                                                         packXkbNativeKey(static_cast<uint64_t>(sym)),
                                                         desc.input_mapping);
    const uint8_t base_mods = mapWaylandModifiers(mod_depressed_, mod_latched_, mod_locked_);
    const uint8_t mods =
        mapNativeModifiersToEvents(WindowAPI::eWaylandWindow, static_cast<uint64_t>(base_mods), desc.input_mapping);
    const bool pressed = (state == WL_KEYBOARD_KEY_STATE_PRESSED);
    if (pressed) {
        eventBridgeKeyDown(win, desc, event_bridge_callbacks_, kc, mods, false);
    } else {
        eventBridgeKeyUp(win, desc, event_bridge_callbacks_, kc, mods);
    }
}

void WaylandWindowManager::onModifiers(uint32_t dep, uint32_t lat, uint32_t lock, uint32_t group) {
    mod_depressed_ = dep;
    mod_latched_ = lat;
    mod_locked_ = lock;
    mod_group_ = group;
    if (xkb_state_) {
        xkb_state_update_mask(xkb_state_, dep, lat, lock, 0U, 0U, group);
    }
}

void WaylandWindowManager::onPointerMotion(double x, double y) {
    ptr_x_ = x;
    ptr_y_ = y;
    WaylandWindow* win = focusedWindow();
    if (!win) {
        return;
    }
    const WindowDescriptor& desc = win->descriptor();
    const uint8_t base_mods = mapWaylandModifiers(mod_depressed_, mod_latched_, mod_locked_);
    const uint8_t mods =
        mapNativeModifiersToEvents(WindowAPI::eWaylandWindow, static_cast<uint64_t>(base_mods), desc.input_mapping);
    eventBridgeMouseMove(win, desc, event_bridge_callbacks_, x, y, mods);
}

void WaylandWindowManager::onPointerButton(uint32_t button, uint32_t state, double x, double y) {
    WaylandWindow* win = focusedWindow();
    if (!win) {
        return;
    }
    // Use cached position if caller passed sentinel -1
    const double px = (x < 0.0) ? ptr_x_ : x;
    const double py = (y < 0.0) ? ptr_y_ : y;
    const WindowDescriptor& desc = win->descriptor();
    const vne::events::MouseButton mb =
        mapNativeMouseToEvents(WindowAPI::eWaylandWindow, packWaylandNativeMouse(button), desc.input_mapping);
    const bool pressed = (state == WL_POINTER_BUTTON_STATE_PRESSED);
    const uint8_t base_mods = mapWaylandModifiers(mod_depressed_, mod_latched_, mod_locked_);
    const uint8_t mods =
        mapNativeModifiersToEvents(WindowAPI::eWaylandWindow, static_cast<uint64_t>(base_mods), desc.input_mapping);
    eventBridgeMouseButton(win, desc, event_bridge_callbacks_, mb, pressed, px, py, mods);
}

void WaylandWindowManager::onPointerAxis(double x_off, double y_off) {
    WaylandWindow* win = focusedWindow();
    if (!win) {
        return;
    }
    eventBridgeMouseScroll(win,
                           win->descriptor(),
                           event_bridge_callbacks_,
                           static_cast<float>(x_off),
                           static_cast<float>(y_off));
}

void WaylandWindowManager::onOutputScale(struct wl_output* output, int32_t factor) {
    if (!output || factor <= 0) {
        return;
    }
    for (auto& [_, info] : outputs_) {
        if (info.output == output) {
            info.scale = factor;
            recomputeOutputScale();
            return;
        }
    }
}

void WaylandWindowManager::onTouchDown(uint32_t id, double x, double y) {
    WaylandWindow* win = focusedWindow();
    if (!win) {
        return;
    }
    eventBridgeTouch(win, win->descriptor(), event_bridge_callbacks_, id, x, y, EventBridgeTouchPhase::eDown);
}

void WaylandWindowManager::onTouchUp(uint32_t id, double x, double y) {
    WaylandWindow* win = focusedWindow();
    if (!win) {
        return;
    }
    eventBridgeTouch(win, win->descriptor(), event_bridge_callbacks_, id, x, y, EventBridgeTouchPhase::eUp);
}

void WaylandWindowManager::onTouchMotion(uint32_t id, double x, double y) {
    WaylandWindow* win = focusedWindow();
    if (!win) {
        return;
    }
    eventBridgeTouch(win, win->descriptor(), event_bridge_callbacks_, id, x, y, EventBridgeTouchPhase::eMove);
}

void WaylandWindowManager::onSeatCapabilities(struct wl_seat* seat, uint32_t caps) {
    const bool has_kb = (caps & WL_SEAT_CAPABILITY_KEYBOARD) != 0U;
    const bool has_ptr = (caps & WL_SEAT_CAPABILITY_POINTER) != 0U;
    const bool has_tch = (caps & WL_SEAT_CAPABILITY_TOUCH) != 0U;

    if (has_kb && !keyboard_) {
        keyboard_ = wl_seat_get_keyboard(seat);
        if (keyboard_) {
            wl_keyboard_add_listener(keyboard_, &kKeyboardListener, this);
        }
    } else if (!has_kb && keyboard_) {
        wl_keyboard_destroy(keyboard_);
        keyboard_ = nullptr;
        if (xkb_state_) {
            xkb_state_unref(xkb_state_);
            xkb_state_ = nullptr;
        }
        if (xkb_keymap_) {
            xkb_keymap_unref(xkb_keymap_);
            xkb_keymap_ = nullptr;
        }
    }

    if (has_ptr && !pointer_) {
        pointer_ = wl_seat_get_pointer(seat);
        if (pointer_) {
            wl_pointer_add_listener(pointer_, &kPointerListener, this);
        }
    } else if (!has_ptr && pointer_) {
        wl_pointer_destroy(pointer_);
        pointer_ = nullptr;
    }

    if (has_tch && !wl_touch_) {
        wl_touch_ = wl_seat_get_touch(seat);
        if (wl_touch_) {
            wl_touch_add_listener(wl_touch_, &kTouchListener, this);
        }
    } else if (!has_tch && wl_touch_) {
        wl_touch_destroy(wl_touch_);
        wl_touch_ = nullptr;
    }
}

// ---------------------------------------------------------------------------
// Registry / globals
// ---------------------------------------------------------------------------

void WaylandWindowManager::onRegistryGlobal(struct wl_registry* registry,
                                            uint32_t name,
                                            const char* interface,
                                            uint32_t version) {
    if (std::strcmp(interface, wl_compositor_interface.name) == 0) {
        bindCompositor(registry, name, version);
    } else if (std::strcmp(interface, xdg_wm_base_interface.name) == 0) {
        bindXdgWmBase(registry, name, version);
    } else if (std::strcmp(interface, wl_seat_interface.name) == 0) {
        bindSeat(registry, name, version);
    } else if (std::strcmp(interface, wl_output_interface.name) == 0) {
        const uint32_t ver = version < 2U ? version : 2U;
        wl_output* output = static_cast<wl_output*>(wl_registry_bind(registry, name, &wl_output_interface, ver));
        if (output) {
            wl_output_add_listener(output, &kOutputListener, this);
            outputs_[name] = OutputInfo{output, 1};
            recomputeOutputScale();
        }
    }
}

void WaylandWindowManager::onRegistryGlobalRemove(uint32_t name) {
    auto it = outputs_.find(name);
    if (it == outputs_.end()) {
        return;
    }
    if (it->second.output) {
        wl_output_destroy(it->second.output);
    }
    outputs_.erase(it);
    recomputeOutputScale();
}

void WaylandWindowManager::bindCompositor(struct wl_registry* registry, uint32_t name, uint32_t version) {
    (void)version;
    compositor_ = static_cast<wl_compositor*>(wl_registry_bind(registry, name, &wl_compositor_interface, 4));
}

void WaylandWindowManager::bindXdgWmBase(struct wl_registry* registry, uint32_t name, uint32_t version) {
    const uint32_t ver = version < 4U ? version : 4U;
    xdg_wm_base_ = static_cast<xdg_wm_base*>(wl_registry_bind(registry, name, &xdg_wm_base_interface, ver));
    if (xdg_wm_base_) {
        xdg_wm_base_add_listener(xdg_wm_base_, &kXdgWmBaseListener, nullptr);
    }
}

void WaylandWindowManager::bindSeat(struct wl_registry* registry, uint32_t name, uint32_t version) {
    const uint32_t ver = version < 5U ? version : 5U;
    seat_ = static_cast<wl_seat*>(wl_registry_bind(registry, name, &wl_seat_interface, ver));
    if (seat_) {
        wl_seat_add_listener(seat_, &kSeatListener, this);
    }
}

// ---------------------------------------------------------------------------
// Standard IWindowManager lifecycle (unchanged logic, only teardown extended)
// ---------------------------------------------------------------------------

WaylandWindowManager::WaylandWindowManager() = default;

WaylandWindowManager::~WaylandWindowManager() {
    shutdown();
}

void WaylandWindowManager::notifyWindowEvent(IWindow* window, const WindowEventData& event) {
    if (callback_ && window) {
        callback_(window, event);
    }
}

bool WaylandWindowManager::initialize() {
    display_ = wl_display_connect(nullptr);
    if (!display_) {
        return false;
    }
    registry_ = wl_display_get_registry(display_);
    if (!registry_) {
        wl_display_disconnect(display_);
        display_ = nullptr;
        return false;
    }
    wl_registry_add_listener(registry_, &kRegistryListener, this);
    if (wl_display_roundtrip(display_) < 0) {
        shutdown();
        return false;
    }
    if (!compositor_ || !xdg_wm_base_) {
        shutdown();
        return false;
    }
    // Second roundtrip picks up seat capabilities
    if (wl_display_roundtrip(display_) < 0) {
        shutdown();
        return false;
    }
    initialized_ = true;
    return true;
}

void WaylandWindowManager::teardownGlobals() {
    kbd_focus_surface_ = nullptr;
    if (keyboard_) {
        wl_keyboard_destroy(keyboard_);
        keyboard_ = nullptr;
    }
    if (xkb_state_) {
        xkb_state_unref(xkb_state_);
        xkb_state_ = nullptr;
    }
    if (xkb_keymap_) {
        xkb_keymap_unref(xkb_keymap_);
        xkb_keymap_ = nullptr;
    }
    if (xkb_context_) {
        xkb_context_unref(xkb_context_);
        xkb_context_ = nullptr;
    }
    if (pointer_) {
        wl_pointer_destroy(pointer_);
        pointer_ = nullptr;
    }
    if (wl_touch_) {
        wl_touch_destroy(wl_touch_);
        wl_touch_ = nullptr;
    }
    for (auto& [_, info] : outputs_) {
        if (info.output) {
            wl_output_destroy(info.output);
        }
    }
    outputs_.clear();
    output_scale_ = 1;
    if (seat_) {
        wl_seat_destroy(seat_);
        seat_ = nullptr;
    }
    if (xdg_wm_base_) {
        xdg_wm_base_destroy(xdg_wm_base_);
        xdg_wm_base_ = nullptr;
    }
    if (compositor_) {
        wl_compositor_destroy(compositor_);
        compositor_ = nullptr;
    }
    if (registry_) {
        wl_registry_destroy(registry_);
        registry_ = nullptr;
    }
    if (display_) {
        wl_display_disconnect(display_);
        display_ = nullptr;
    }
}

void WaylandWindowManager::recomputeOutputScale() noexcept {
    int32_t scale = 1;
    for (const auto& [_, info] : outputs_) {
        if (info.scale > scale) {
            scale = info.scale;
        }
    }
    output_scale_ = scale;
}

void WaylandWindowManager::shutdown() {
    destroyAllWindows();
    teardownGlobals();
    initialized_ = false;
}

bool WaylandWindowManager::isInitialized() const noexcept {
    return initialized_;
}

std::shared_ptr<IWindow> WaylandWindowManager::openWindow(const WindowDescriptor& descriptor) {
    if (!initialized_ || !display_ || !compositor_ || !xdg_wm_base_) {
        return nullptr;
    }
    auto w = std::make_shared<WaylandWindow>();
    w->setEventOwner(this);
    w->initialize(descriptor);
    if (!w->isOpen()) {
        return nullptr;
    }
    windows_.push_back(w);
    if (!primary_) {
        primary_ = w;
    }
    focused_ = w;
    return w;
}

std::shared_ptr<IWindow> WaylandWindowManager::openWindow(const std::string& title, uint32_t width, uint32_t height) {
    return openWindow(WindowDescriptor(title, width, height));
}

void WaylandWindowManager::removeWindow(std::shared_ptr<IWindow> window) {
    if (!window) {
        return;
    }
    window->close();
    auto it = std::find(windows_.begin(), windows_.end(), window);
    if (it != windows_.end()) {
        windows_.erase(it);
    }
    if (primary_ == window) {
        primary_ = windows_.empty() ? nullptr : windows_.front();
    }
    if (focused_ == window) {
        focused_ = primary_;
    }
}

void WaylandWindowManager::destroyAllWindows() {
    for (auto& w : windows_) {
        if (w) {
            w->close();
        }
    }
    windows_.clear();
    primary_.reset();
    focused_.reset();
}

size_t WaylandWindowManager::getWindowCount() const noexcept {
    return windows_.size();
}
std::vector<std::shared_ptr<IWindow>> WaylandWindowManager::getWindows() const {
    return windows_;
}
std::shared_ptr<IWindow> WaylandWindowManager::getPrimaryWindow() const noexcept {
    return primary_;
}
std::shared_ptr<IWindow> WaylandWindowManager::getFocusedWindow() const noexcept {
    return focused_;
}
void WaylandWindowManager::setPrimaryWindow(std::shared_ptr<IWindow> w) {
    primary_ = std::move(w);
}
void WaylandWindowManager::focusWindow(std::shared_ptr<IWindow> w) {
    focused_ = std::move(w);
}

void WaylandWindowManager::processEvents() {
    if (display_) {
        wl_display_dispatch(display_);
    }
}

void WaylandWindowManager::setEventCallback(const WindowManagerEventCallbackT& cb) {
    callback_ = cb;
}
void WaylandWindowManager::setEventBridgeCallbacks(EventBridgeCallbacks cbs) {
    event_bridge_callbacks_ = std::move(cbs);
}

bool WaylandWindowManager::shouldClose() const noexcept {
    for (const auto& w : windows_) {
        if (w && !w->isOpen()) {
            return true;
        }
    }
    return false;
}

bool WaylandWindowManager::shouldCloseAll() const noexcept {
    if (windows_.empty()) {
        return false;
    }
    for (const auto& w : windows_) {
        if (w && w->isOpen()) {
            return false;
        }
    }
    return true;
}

WindowAPI WaylandWindowManager::getWindowAPI() const noexcept {
    return WindowAPI::eWaylandWindow;
}
std::string WaylandWindowManager::getPlatformInfo() const {
    return "Linux / Wayland (xdg-shell)";
}
bool WaylandWindowManager::isFeatureSupported(const std::string& f) const {
    return f == "resize" || f == "close" || f == "wayland" || f == "touch";
}
std::string WaylandWindowManager::getProperties() const {
    return properties_;
}
void WaylandWindowManager::setProperties(const std::string& p) {
    properties_ = p;
}

uint64_t WaylandWindowManager::getCurrentTime() const noexcept {
    using namespace std::chrono;
    return static_cast<uint64_t>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}
void WaylandWindowManager::sleep(uint32_t ms) const noexcept {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
double WaylandWindowManager::getPlatformTime() const noexcept {
    using namespace std::chrono;
    return duration<double>(steady_clock::now().time_since_epoch()).count();
}

}  // namespace vne::xwin
