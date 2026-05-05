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

#include <algorithm>
#include <chrono>
#include <cstring>
#include <thread>

#include <wayland-client.h>
#include <linux/input-event-codes.h>

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
    static_cast<WaylandWindowManager_C*>(data)->on_registry_global(reg, name, iface, ver);
}
void registry_global_remove(void*, struct wl_registry*, uint32_t) {}

const wl_registry_listener kRegistryListener = {
    registry_global,
    registry_global_remove,
};

void output_geometry(
    void*, struct wl_output*, int32_t, int32_t, int32_t, int32_t, int32_t, const char*, const char*, int32_t) {}
void output_mode(void*, struct wl_output*, uint32_t, int32_t, int32_t, int32_t) {}
void output_done(void*, struct wl_output*) {}
void output_scale(void* data, struct wl_output*, int32_t factor) {
    auto* self = static_cast<WaylandWindowManager_C*>(data);
    self->on_output_scale(factor);
}
const wl_output_listener kOutputListener = {output_geometry, output_mode, output_done, output_scale};

// ---- xdg_wm_base ping ----

void xdg_ping(void*, struct xdg_wm_base* base, uint32_t serial) {
    xdg_wm_base_pong(base, serial);
}
const xdg_wm_base_listener kXdgWmBaseListener = {xdg_ping};

// ---- wl_seat ----

void seat_capabilities(void* data, struct wl_seat* seat, uint32_t caps) {
    static_cast<WaylandWindowManager_C*>(data)->on_seat_capabilities(seat, caps);
}
void seat_name(void*, struct wl_seat*, const char*) {}

const wl_seat_listener kSeatListener = {seat_capabilities, seat_name};

// ---- wl_keyboard ----

void kb_keymap(void*, struct wl_keyboard*, uint32_t, int32_t, uint32_t) {}

void kb_enter(
    void* data, struct wl_keyboard*, uint32_t /*serial*/, struct wl_surface* surface, struct wl_array* /*keys*/) {
    static_cast<WaylandWindowManager_C*>(data)->on_keyboard_enter(surface);
}

void kb_leave(void* data, struct wl_keyboard*, uint32_t /*serial*/, struct wl_surface* surface) {
    static_cast<WaylandWindowManager_C*>(data)->on_keyboard_leave(surface);
}

void kb_key(void* data, struct wl_keyboard*, uint32_t /*serial*/, uint32_t /*time*/, uint32_t key, uint32_t state) {
    static_cast<WaylandWindowManager_C*>(data)->on_key(key, state, 0);
}

void kb_modifiers(void* data,
                  struct wl_keyboard*,
                  uint32_t /*serial*/,
                  uint32_t depressed,
                  uint32_t latched,
                  uint32_t locked,
                  uint32_t /*group*/) {
    static_cast<WaylandWindowManager_C*>(data)->on_modifiers(depressed, latched, locked);
}

void kb_repeat_info(void*, struct wl_keyboard*, int32_t, int32_t) {}

const wl_keyboard_listener kKeyboardListener = {kb_keymap, kb_enter, kb_leave, kb_key, kb_modifiers, kb_repeat_info};

// ---- wl_pointer ----

void ptr_enter(void*, struct wl_pointer*, uint32_t, struct wl_surface*, wl_fixed_t sx, wl_fixed_t sy) {
    (void)sx;
    (void)sy;
}
void ptr_leave(void*, struct wl_pointer*, uint32_t, struct wl_surface*) {}

void ptr_motion(void* data, struct wl_pointer*, uint32_t /*time*/, wl_fixed_t sx, wl_fixed_t sy) {
    static_cast<WaylandWindowManager_C*>(data)->on_pointer_motion(wl_fixed_to_double(sx), wl_fixed_to_double(sy));
}

void ptr_button(
    void* data, struct wl_pointer*, uint32_t /*serial*/, uint32_t /*time*/, uint32_t button, uint32_t state) {
    // We don't have separate coords in the button event; use last known position
    // stored by on_pointer_motion. Pass 0 here; manager will use cached coords.
    static_cast<WaylandWindowManager_C*>(data)->on_pointer_button(button, state, -1.0, -1.0);
}

void ptr_axis(void* data, struct wl_pointer*, uint32_t /*time*/, uint32_t axis, wl_fixed_t value) {
    // axis 0 = vertical, axis 1 = horizontal
    const double v = wl_fixed_to_double(value) / -10.0;  // normalise to scroll steps
    if (axis == 0) {
        static_cast<WaylandWindowManager_C*>(data)->on_pointer_axis(0.0, v);
    } else {
        static_cast<WaylandWindowManager_C*>(data)->on_pointer_axis(v, 0.0);
    }
}

void ptr_frame(void*, struct wl_pointer*) {}
void ptr_axis_source(void*, struct wl_pointer*, uint32_t) {}
void ptr_axis_stop(void*, struct wl_pointer*, uint32_t, uint32_t) {}
void ptr_axis_discrete(void*, struct wl_pointer*, uint32_t, int32_t) {}

const wl_pointer_listener kPointerListener = {ptr_enter,
                                              ptr_leave,
                                              ptr_motion,
                                              ptr_button,
                                              ptr_axis,
                                              ptr_frame,
                                              ptr_axis_source,
                                              ptr_axis_stop,
                                              ptr_axis_discrete};

// ---- wl_touch ----

void touch_down(void* data,
                struct wl_touch*,
                uint32_t /*serial*/,
                uint32_t /*time*/,
                struct wl_surface*,
                int32_t id,
                wl_fixed_t x,
                wl_fixed_t y) {
    static_cast<WaylandWindowManager_C*>(data)->on_touch_down(static_cast<uint32_t>(id),
                                                              wl_fixed_to_double(x),
                                                              wl_fixed_to_double(y));
}
void touch_up(void* data, struct wl_touch*, uint32_t /*serial*/, uint32_t /*time*/, int32_t id) {
    static_cast<WaylandWindowManager_C*>(data)->on_touch_up(static_cast<uint32_t>(id), 0.0, 0.0);
}
void touch_motion(void* data, struct wl_touch*, uint32_t /*time*/, int32_t id, wl_fixed_t x, wl_fixed_t y) {
    static_cast<WaylandWindowManager_C*>(data)->on_touch_motion(static_cast<uint32_t>(id),
                                                                wl_fixed_to_double(x),
                                                                wl_fixed_to_double(y));
}
void touch_frame(void*, struct wl_touch*) {}
void touch_cancel(void*, struct wl_touch*) {}
void touch_shape(void*, struct wl_touch*, int32_t, wl_fixed_t, wl_fixed_t) {}
void touch_orientation(void*, struct wl_touch*, int32_t, wl_fixed_t) {}

const wl_touch_listener kTouchListener = {
    touch_down, touch_up, touch_motion, touch_frame, touch_cancel, touch_shape, touch_orientation};

// Map Linux kernel button codes to vne MouseButton
vne::events::MouseButton linuxButtonToMouse(uint32_t btn) {
    switch (btn) {
        case BTN_LEFT:
            return vne::events::MouseButton::eLeft;
        case BTN_RIGHT:
            return vne::events::MouseButton::eRight;
        case BTN_MIDDLE:
            return vne::events::MouseButton::eMiddle;
        default:
            return vne::events::MouseButton::eLeft;
    }
}

}  // namespace

// ---------------------------------------------------------------------------
// WaylandWindowManager_C — input dispatch helpers
// ---------------------------------------------------------------------------

WaylandWindow_C* WaylandWindowManager_C::focused_window() const {
    if (_kbd_focus_surface) {
        WaylandWindow_C* w = window_for_surface(_kbd_focus_surface);
        if (w) {
            return w;
        }
    }
    if (_focused) {
        return dynamic_cast<WaylandWindow_C*>(_focused.get());
    }
    if (_primary) {
        return dynamic_cast<WaylandWindow_C*>(_primary.get());
    }
    return nullptr;
}

WaylandWindow_C* WaylandWindowManager_C::window_for_surface(wl_surface* surface) const {
    if (!surface) {
        return nullptr;
    }
    for (const auto& w : _windows) {
        auto* wl = dynamic_cast<WaylandWindow_C*>(w.get());
        if (wl && wl->native_surface() == surface) {
            return wl;
        }
    }
    return nullptr;
}

void WaylandWindowManager_C::on_keyboard_enter(wl_surface* surface) {
    if (!surface) {
        return;
    }
    if (_kbd_focus_surface == surface) {
        return;
    }
    if (_kbd_focus_surface && _kbd_focus_surface != surface) {
        WaylandWindow_C* prev = window_for_surface(_kbd_focus_surface);
        if (prev) {
            notify_window_focus(prev, false);
        }
    }
    _kbd_focus_surface = surface;
    WaylandWindow_C* win = window_for_surface(surface);
    if (win) {
        for (auto& w : _windows) {
            if (w.get() == win) {
                _focused = w;
                break;
            }
        }
        notify_window_focus(win, true);
    }
}

void WaylandWindowManager_C::on_keyboard_leave(wl_surface* surface) {
    if (!surface) {
        return;
    }
    if (_kbd_focus_surface != surface) {
        return;
    }
    WaylandWindow_C* win = window_for_surface(surface);
    _kbd_focus_surface = nullptr;
    if (win) {
        notify_window_focus(win, false);
    }
}

void WaylandWindowManager_C::notify_window_focus(WaylandWindow_C* win, bool focused) {
    if (!win) {
        return;
    }
    eventBridgeWindowFocus(win, win->descriptor(), _event_bridge_callbacks, focused);
    WindowEventData_C ev{};
    ev.type = WindowEventType_TP::FOCUS;
    ev.focused = focused;
    NotifyWindowEvent(win, ev);
}

void WaylandWindowManager_C::on_key(uint32_t linux_key, uint32_t state, uint32_t /*time*/) {
    WaylandWindow_C* win = focused_window();
    if (!win) {
        return;
    }

    // Wayland sends Linux evdev scan codes; convert to XKB keysym via offset
    // (Linux keycodes are XKB scancode - 8; we need to get the keysym another way)
    // Without a full xkb_state, map the scan code to a known keysym using the
    // standard QWERTY assumption. For full layout support xkb_state_key_get_one_sym
    // would be used; we call mapWaylandKeysym with the XKB keysym.
    // Here we apply the standard evdev→XKB keysym table subset.
    // For simplicity we convert scan codes to XKB keysyms using the offset rule:
    //   xkb_keycode = linux_keycode + 8
    // then look up via a compact inline table.

    // Compact evdev scancode → XKB keysym mapping (US QWERTY layout subset)
    static const uint32_t kEvdevToXkb[] = {// 0-9
                                           0,
                                           0,
                                           0,
                                           0,
                                           0,
                                           0,
                                           0,
                                           0,
                                           0,
                                           0,
                                           // 10: KEY_Q..KEY_P row
                                           0x71,
                                           0x77,
                                           0x65,
                                           0x72,
                                           0x74,
                                           0x79,
                                           0x75,
                                           0x69,
                                           0x6f,
                                           0x70,  // q w e r t y u i o p
                                                  // 20: brackets, enter, ctrl, home row
                                           0x5b,
                                           0x5d,
                                           0xff0d,
                                           0,
                                           0x61,
                                           0x73,
                                           0x64,
                                           0x66,
                                           0x67,
                                           0x68,  // [ ] enter ctrl a s d f g h
                                                  // 30: j k l ; ' ` lshift \ z x
                                           0x6a,
                                           0x6b,
                                           0x6c,
                                           0x3b,
                                           0x27,
                                           0x60,
                                           0,
                                           0x5c,
                                           0x7a,
                                           0x78,  // j k l ; ' ` lshift \ z x
                                                  // 40: c v b n m , . / rshift kp*
                                           0x63,
                                           0x76,
                                           0x62,
                                           0x6e,
                                           0x6d,
                                           0x2c,
                                           0x2e,
                                           0x2f,
                                           0,
                                           0,  // c v b n m , . / rshift kp*
                                               // 50: alt space caps f1-f10
                                           0,
                                           0x20,
                                           0xffe5,
                                           0xffbe,
                                           0xffbf,
                                           0xffc0,
                                           0xffc1,
                                           0xffc2,
                                           0xffc3,
                                           0xffc4,  // alt sp caps f1-f9
                                                    // 60: f10 numlock scroll 7 8 9 kp- 4 5 6
                                           0xffc9,
                                           0xff7f,
                                           0xff14,
                                           0xffb7,
                                           0xffb8,
                                           0xffb9,
                                           0xffad,
                                           0xffb4,
                                           0xffb5,
                                           0xffb6,
                                           // 70: kp+ 1 2 3 0 kpdot f11 f12
                                           0xffab,
                                           0xffb1,
                                           0xffb2,
                                           0xffb3,
                                           0xffb0,
                                           0xffae,
                                           0xffc0,
                                           0xffc1,
                                           0,
                                           0,
                                           // 80-89 gaps
                                           0,
                                           0,
                                           0,
                                           0,
                                           0,
                                           0,
                                           0,
                                           0,
                                           0,
                                           0,
                                           // 90-99 kpenter kpctrl kp/ sysrq ralt
                                           0xffb0,
                                           0,
                                           0xffaf,
                                           0,
                                           0,
                                           0,
                                           0,
                                           0,
                                           0,
                                           0,
                                           // 100-109 home up pgup left right end down pgdn ins del
                                           0xff50,
                                           0xff52,
                                           0xff55,
                                           0xff51,
                                           0xff53,
                                           0xff57,
                                           0xff54,
                                           0xff56,
                                           0xff63,
                                           0xffff,
                                           // 110-119 esc numlock caps scroll kpequal kppmn 0 kpdot
                                           0xff1b,
                                           0xff7f,
                                           0xffe5,
                                           0xff14,
                                           0xffbd,
                                           0,
                                           0,
                                           0,
                                           0,
                                           0,
                                           // 120-125: super menu
                                           0,
                                           0,
                                           0,
                                           0,
                                           0,
                                           0};
    constexpr uint32_t kTableSize = sizeof(kEvdevToXkb) / sizeof(kEvdevToXkb[0]);

    uint32_t sym = 0;
    // Digit row: KEY_1(2)..KEY_0(11)
    if (linux_key >= 2 && linux_key <= 11) {
        static const uint32_t digits[] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30};
        sym = digits[linux_key - 2];
    } else if (linux_key < kTableSize) {
        sym = kEvdevToXkb[linux_key];
    }

    // Override some well-known scan codes
    switch (linux_key) {
        case 103:
            sym = 0xff52;
            break;  // KEY_UP
        case 108:
            sym = 0xff54;
            break;  // KEY_DOWN
        case 105:
            sym = 0xff51;
            break;  // KEY_LEFT
        case 106:
            sym = 0xff53;
            break;  // KEY_RIGHT
        case 104:
            sym = 0xff55;
            break;  // KEY_PAGEUP
        case 109:
            sym = 0xff56;
            break;  // KEY_PAGEDOWN
        case 102:
            sym = 0xff50;
            break;  // KEY_HOME
        case 107:
            sym = 0xff57;
            break;  // KEY_END
        case 42:
            sym = 0xffe1;
            break;  // KEY_LEFTSHIFT
        case 54:
            sym = 0xffe2;
            break;  // KEY_RIGHTSHIFT
        case 29:
            sym = 0xffe3;
            break;  // KEY_LEFTCTRL
        case 97:
            sym = 0xffe4;
            break;  // KEY_RIGHTCTRL
        case 56:
            sym = 0xffe9;
            break;  // KEY_LEFTALT
        case 100:
            sym = 0xffea;
            break;  // KEY_RIGHTALT
        case 125:
            sym = 0xffeb;
            break;  // KEY_LEFTMETA
        case 126:
            sym = 0xffec;
            break;  // KEY_RIGHTMETA
        default:
            break;
    }

    if (sym == 0) {
        return;
    }
    const vne::events::KeyCode kc = mapWaylandKeysym(sym);
    const uint8_t mods = mapWaylandModifiers(_mod_depressed, _mod_latched, _mod_locked);
    const bool pressed = (state == WL_KEYBOARD_KEY_STATE_PRESSED);

    const WindowDescriptor_C& desc = win->descriptor();
    if (pressed) {
        eventBridgeKeyDown(win, desc, _event_bridge_callbacks, kc, mods, false);
    } else {
        eventBridgeKeyUp(win, desc, _event_bridge_callbacks, kc, mods);
    }
}

void WaylandWindowManager_C::on_modifiers(uint32_t dep, uint32_t lat, uint32_t lock) {
    _mod_depressed = dep;
    _mod_latched = lat;
    _mod_locked = lock;
}

void WaylandWindowManager_C::on_pointer_motion(double x, double y) {
    _ptr_x = x;
    _ptr_y = y;
    WaylandWindow_C* win = focused_window();
    if (!win) {
        return;
    }
    const uint8_t mods = mapWaylandModifiers(_mod_depressed, _mod_latched, _mod_locked);
    eventBridgeMouseMove(win, win->descriptor(), _event_bridge_callbacks, x, y, mods);
}

void WaylandWindowManager_C::on_pointer_button(uint32_t button, uint32_t state, double x, double y) {
    WaylandWindow_C* win = focused_window();
    if (!win) {
        return;
    }
    // Use cached position if caller passed sentinel -1
    const double px = (x < 0.0) ? _ptr_x : x;
    const double py = (y < 0.0) ? _ptr_y : y;
    const vne::events::MouseButton mb = linuxButtonToMouse(button);
    const bool pressed = (state == WL_POINTER_BUTTON_STATE_PRESSED);
    const uint8_t mods = mapWaylandModifiers(_mod_depressed, _mod_latched, _mod_locked);
    eventBridgeMouseButton(win, win->descriptor(), _event_bridge_callbacks, mb, pressed, px, py, mods);
}

void WaylandWindowManager_C::on_pointer_axis(double x_off, double y_off) {
    WaylandWindow_C* win = focused_window();
    if (!win) {
        return;
    }
    eventBridgeMouseScroll(win,
                           win->descriptor(),
                           _event_bridge_callbacks,
                           static_cast<float>(x_off),
                           static_cast<float>(y_off));
}

void WaylandWindowManager_C::on_output_scale(int32_t factor) {
    if (factor > 0) {
        _output_scale = factor;
    }
}

void WaylandWindowManager_C::on_touch_down(uint32_t id, double x, double y) {
    WaylandWindow_C* win = focused_window();
    if (!win) {
        return;
    }
    eventBridgeTouch(win, win->descriptor(), _event_bridge_callbacks, id, x, y, EventBridgeTouchPhase::eDown);
}

void WaylandWindowManager_C::on_touch_up(uint32_t id, double x, double y) {
    WaylandWindow_C* win = focused_window();
    if (!win) {
        return;
    }
    eventBridgeTouch(win, win->descriptor(), _event_bridge_callbacks, id, x, y, EventBridgeTouchPhase::eUp);
}

void WaylandWindowManager_C::on_touch_motion(uint32_t id, double x, double y) {
    WaylandWindow_C* win = focused_window();
    if (!win) {
        return;
    }
    eventBridgeTouch(win, win->descriptor(), _event_bridge_callbacks, id, x, y, EventBridgeTouchPhase::eMove);
}

void WaylandWindowManager_C::on_seat_capabilities(struct wl_seat* seat, uint32_t caps) {
    const bool has_kb = (caps & WL_SEAT_CAPABILITY_KEYBOARD) != 0U;
    const bool has_ptr = (caps & WL_SEAT_CAPABILITY_POINTER) != 0U;
    const bool has_tch = (caps & WL_SEAT_CAPABILITY_TOUCH) != 0U;

    if (has_kb && !_keyboard) {
        _keyboard = wl_seat_get_keyboard(seat);
        if (_keyboard) {
            wl_keyboard_add_listener(_keyboard, &kKeyboardListener, this);
        }
    } else if (!has_kb && _keyboard) {
        wl_keyboard_destroy(_keyboard);
        _keyboard = nullptr;
    }

    if (has_ptr && !_pointer) {
        _pointer = wl_seat_get_pointer(seat);
        if (_pointer) {
            wl_pointer_add_listener(_pointer, &kPointerListener, this);
        }
    } else if (!has_ptr && _pointer) {
        wl_pointer_destroy(_pointer);
        _pointer = nullptr;
    }

    if (has_tch && !_wl_touch) {
        _wl_touch = wl_seat_get_touch(seat);
        if (_wl_touch) {
            wl_touch_add_listener(_wl_touch, &kTouchListener, this);
        }
    } else if (!has_tch && _wl_touch) {
        wl_touch_destroy(_wl_touch);
        _wl_touch = nullptr;
    }
}

// ---------------------------------------------------------------------------
// Registry / globals
// ---------------------------------------------------------------------------

void WaylandWindowManager_C::on_registry_global(struct wl_registry* registry,
                                                uint32_t name,
                                                const char* interface,
                                                uint32_t version) {
    if (std::strcmp(interface, wl_compositor_interface.name) == 0) {
        bind_compositor(registry, name, version);
    } else if (std::strcmp(interface, xdg_wm_base_interface.name) == 0) {
        bind_xdg_wm_base(registry, name, version);
    } else if (std::strcmp(interface, wl_seat_interface.name) == 0) {
        bind_seat(registry, name, version);
    } else if (std::strcmp(interface, wl_output_interface.name) == 0 && !_output) {
        const uint32_t ver = version < 2U ? version : 2U;
        _output = static_cast<wl_output*>(wl_registry_bind(registry, name, &wl_output_interface, ver));
        if (_output) {
            wl_output_add_listener(_output, &kOutputListener, this);
        }
    }
}

void WaylandWindowManager_C::bind_compositor(struct wl_registry* registry, uint32_t name, uint32_t version) {
    (void)version;
    _compositor = static_cast<wl_compositor*>(wl_registry_bind(registry, name, &wl_compositor_interface, 4));
}

void WaylandWindowManager_C::bind_xdg_wm_base(struct wl_registry* registry, uint32_t name, uint32_t version) {
    const uint32_t ver = version < 4U ? version : 4U;
    _xdg_wm_base = static_cast<xdg_wm_base*>(wl_registry_bind(registry, name, &xdg_wm_base_interface, ver));
    if (_xdg_wm_base) {
        xdg_wm_base_add_listener(_xdg_wm_base, &kXdgWmBaseListener, nullptr);
    }
}

void WaylandWindowManager_C::bind_seat(struct wl_registry* registry, uint32_t name, uint32_t version) {
    const uint32_t ver = version < 5U ? version : 5U;
    _seat = static_cast<wl_seat*>(wl_registry_bind(registry, name, &wl_seat_interface, ver));
    if (_seat) {
        wl_seat_add_listener(_seat, &kSeatListener, this);
    }
}

// ---------------------------------------------------------------------------
// Standard WindowManager_I lifecycle (unchanged logic, only teardown extended)
// ---------------------------------------------------------------------------

WaylandWindowManager_C::WaylandWindowManager_C() = default;

WaylandWindowManager_C::~WaylandWindowManager_C() {
    Shutdown();
}

void WaylandWindowManager_C::NotifyWindowEvent(Window_I* window, const WindowEventData_C& event) {
    if (_callback && window) {
        _callback(window, event);
    }
}

bool WaylandWindowManager_C::Initialize() {
    _display = wl_display_connect(nullptr);
    if (!_display) {
        return false;
    }
    _registry = wl_display_get_registry(_display);
    if (!_registry) {
        wl_display_disconnect(_display);
        _display = nullptr;
        return false;
    }
    wl_registry_add_listener(_registry, &kRegistryListener, this);
    if (wl_display_roundtrip(_display) < 0) {
        Shutdown();
        return false;
    }
    if (!_compositor || !_xdg_wm_base) {
        Shutdown();
        return false;
    }
    // Second roundtrip picks up seat capabilities
    wl_display_roundtrip(_display);
    _initialized = true;
    return true;
}

void WaylandWindowManager_C::teardown_globals() {
    _kbd_focus_surface = nullptr;
    if (_keyboard) {
        wl_keyboard_destroy(_keyboard);
        _keyboard = nullptr;
    }
    if (_pointer) {
        wl_pointer_destroy(_pointer);
        _pointer = nullptr;
    }
    if (_wl_touch) {
        wl_touch_destroy(_wl_touch);
        _wl_touch = nullptr;
    }
    if (_output) {
        wl_output_destroy(_output);
        _output = nullptr;
    }
    if (_seat) {
        wl_seat_destroy(_seat);
        _seat = nullptr;
    }
    if (_xdg_wm_base) {
        xdg_wm_base_destroy(_xdg_wm_base);
        _xdg_wm_base = nullptr;
    }
    if (_compositor) {
        wl_compositor_destroy(_compositor);
        _compositor = nullptr;
    }
    if (_registry) {
        wl_registry_destroy(_registry);
        _registry = nullptr;
    }
    if (_display) {
        wl_display_disconnect(_display);
        _display = nullptr;
    }
}

void WaylandWindowManager_C::Shutdown() {
    DestroyAllWindows();
    teardown_globals();
    _initialized = false;
}

bool WaylandWindowManager_C::IsInitialized() const {
    return _initialized;
}

std::shared_ptr<Window_I> WaylandWindowManager_C::CreateWindow(const WindowDescriptor_C& descriptor) {
    if (!_initialized || !_display || !_compositor || !_xdg_wm_base) {
        return nullptr;
    }
    auto w = std::make_shared<WaylandWindow_C>();
    w->SetOwner(this);
    w->Initialize(descriptor);
    if (!w->IsOpen()) {
        return nullptr;
    }
    _windows.push_back(w);
    if (!_primary) {
        _primary = w;
    }
    _focused = w;
    return w;
}

std::shared_ptr<Window_I> WaylandWindowManager_C::CreateWindow(const std::string& title,
                                                               uint32_t width,
                                                               uint32_t height) {
    return CreateWindow(WindowDescriptor_C(title, width, height));
}

void WaylandWindowManager_C::DestroyWindow(std::shared_ptr<Window_I> window) {
    if (!window) {
        return;
    }
    window->Close();
    auto it = std::find(_windows.begin(), _windows.end(), window);
    if (it != _windows.end()) {
        _windows.erase(it);
    }
    if (_primary == window) {
        _primary = _windows.empty() ? nullptr : _windows.front();
    }
    if (_focused == window) {
        _focused = _primary;
    }
}

void WaylandWindowManager_C::DestroyAllWindows() {
    for (auto& w : _windows) {
        if (w) {
            w->Close();
        }
    }
    _windows.clear();
    _primary.reset();
    _focused.reset();
}

size_t WaylandWindowManager_C::GetWindowCount() const {
    return _windows.size();
}
std::vector<std::shared_ptr<Window_I>> WaylandWindowManager_C::GetWindows() const {
    return _windows;
}
std::shared_ptr<Window_I> WaylandWindowManager_C::GetPrimaryWindow() const {
    return _primary;
}
std::shared_ptr<Window_I> WaylandWindowManager_C::GetFocusedWindow() const {
    return _focused;
}
void WaylandWindowManager_C::SetPrimaryWindow(std::shared_ptr<Window_I> w) {
    _primary = std::move(w);
}
void WaylandWindowManager_C::FocusWindow(std::shared_ptr<Window_I> w) {
    _focused = std::move(w);
}

void WaylandWindowManager_C::ProcessEvents() {
    if (_display) {
        wl_display_dispatch_pending(_display);
    }
}

void WaylandWindowManager_C::SetEventCallback(const WindowManagerEventCallback_T& cb) {
    _callback = cb;
}
void WaylandWindowManager_C::setEventBridgeCallbacks(EventBridgeCallbacks cbs) {
    _event_bridge_callbacks = std::move(cbs);
}

bool WaylandWindowManager_C::ShouldClose() const {
    for (const auto& w : _windows) {
        if (w && !w->IsOpen()) {
            return true;
        }
    }
    return false;
}

bool WaylandWindowManager_C::ShouldCloseAll() const {
    if (_windows.empty()) {
        return false;
    }
    for (const auto& w : _windows) {
        if (w && w->IsOpen()) {
            return false;
        }
    }
    return true;
}

WindowAPI_TP WaylandWindowManager_C::GetWindowAPI() const {
    return WindowAPI_TP::WAYLAND_WINDOW;
}
std::string WaylandWindowManager_C::GetPlatformInfo() const {
    return "Linux / Wayland (xdg-shell)";
}
bool WaylandWindowManager_C::IsFeatureSupported(const std::string& f) const {
    return f == "resize" || f == "close" || f == "wayland" || f == "touch";
}
std::string WaylandWindowManager_C::GetProperties() const {
    return _properties;
}
void WaylandWindowManager_C::SetProperties(const std::string& p) {
    _properties = p;
}

uint64_t WaylandWindowManager_C::GetCurrentTime() const {
    using namespace std::chrono;
    return static_cast<uint64_t>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}
void WaylandWindowManager_C::Sleep(uint32_t ms) const {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}
double WaylandWindowManager_C::GetPlatformTime() const {
    using namespace std::chrono;
    return duration<double>(steady_clock::now().time_since_epoch()).count();
}

}  // namespace vne::xwin
