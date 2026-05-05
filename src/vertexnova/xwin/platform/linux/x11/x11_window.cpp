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

#include "x11_window.h"

#include "x11_window_manager.h"
#include "xwin_map_key.h"
#include "event_bridge.h"

#include <vertexnova/events/types.h>

#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <cstdint>

namespace vne::xwin {

namespace {

vne::events::MouseButton x11ButtonToMouseButton(unsigned int button) {
    switch (button) {
        case 1U:
            return vne::events::MouseButton::eLeft;
        case 2U:
            return vne::events::MouseButton::eMiddle;
        case 3U:
            return vne::events::MouseButton::eRight;
        default:
            return vne::events::MouseButton::eLeft;
    }
}

}  // namespace

X11Window_C::X11Window_C() = default;

X11Window_C::~X11Window_C() {
    destroy();
}

void X11Window_C::SetEventOwner(X11WindowManager_C* owner) {
    _owner = owner;
}

void X11Window_C::SetDisplay(Display* display, int screen, ::Window root, void* xcb_connection) {
    _display = display;
    _screen = screen;
    _root = root;
    _xcb_connection = xcb_connection;
}

void X11Window_C::destroy() {
    if (_display && _window) {
        XDestroyWindow(_display, _window);
        _window = 0;
    }
    _open = false;
}

void X11Window_C::Initialize(const WindowDescriptor_C& descriptor) {
    destroy();
    if (!_display) {
        return;
    }
    _desc = descriptor;
    const unsigned long black = BlackPixel(_display, _screen);
    const unsigned long white = WhitePixel(_display, _screen);
    _window = XCreateSimpleWindow(_display,
                                  _root,
                                  _desc.position.x,
                                  _desc.position.y,
                                  static_cast<unsigned>(_desc.size.width),
                                  static_cast<unsigned>(_desc.size.height),
                                  0,
                                  black,
                                  white);
    if (!_window) {
        return;
    }
    XSelectInput(_display,
                 _window,
                 ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask
                     | StructureNotifyMask | FocusChangeMask);
    _wm_delete = XInternAtom(_display, "WM_DELETE_WINDOW", False);
    Atom protocols[] = {_wm_delete};
    XSetWMProtocols(_display, _window, protocols, 1);

    XStoreName(_display, _window, _desc.title.c_str());

    if (_desc.visible) {
        XMapWindow(_display, _window);
    }
    XFlush(_display);

    // Apply size limits from descriptor
    if (_desc.limits.has_min_size || _desc.limits.has_max_size) {
        SetWindowLimits(_desc.limits);
    }

    _open = true;
}

void X11Window_C::PollEvents() {
    if (!_display || !_window) {
        return;
    }
    const EventBridgeCallbacks empty_callbacks{};
    const EventBridgeCallbacks& cb = _owner ? _owner->eventBridgeCallbacks() : empty_callbacks;

    XEvent ev{};
    while (XPending(_display) > 0) {
        XNextEvent(_display, &ev);
        // SelectionRequest is addressed to us even when xany.window differs
        if (ev.type == SelectionRequest) {
            handle_selection_request(ev.xselectionrequest);
            continue;
        }
        if (ev.xany.window != _window) {
            continue;
        }
        if (ev.type == ClientMessage) {
            if (static_cast<Atom>(ev.xclient.data.l[0]) == _wm_delete) {
                eventBridgeWindowClose(this, _desc, cb);
                _open = false;
                if (_owner) {
                    WindowEventData_C data{};
                    data.type = WindowEventType_TP::CLOSE;
                    _owner->NotifyWindowEvent(this, data);
                }
            }
        } else if (ev.type == ConfigureNotify) {
            _desc.size.width = static_cast<uint32_t>(ev.xconfigure.width);
            _desc.size.height = static_cast<uint32_t>(ev.xconfigure.height);
            eventBridgeWindowResize(this, _desc, cb, _desc.size.width, _desc.size.height);
            if (_owner) {
                WindowEventData_C data{};
                data.type = WindowEventType_TP::RESIZE;
                data.size = _desc.size;
                _owner->NotifyWindowEvent(this, data);
            }
        } else if (ev.type == KeyPress) {
            const unsigned int kc = static_cast<unsigned int>(ev.xkey.keycode);
            const KeySym sym = XLookupKeysym(&ev.xkey, 0);
            const vne::events::KeyCode mapped = mapX11Keysym(sym);
            if (mapped != vne::events::KeyCode::eUnknown && kc < _keycode_down.size()) {
                const bool repeat = _keycode_down[kc];
                _keycode_down[kc] = true;
                const std::uint8_t mods = mapX11Modifiers(ev.xkey.state);
                eventBridgeKeyDown(this, _desc, cb, mapped, mods, repeat);
            }
            // Text input: decode printable characters via XLookupString
            if (_desc.enable_events || cb.onTextInput) {
                char buf[32] = {};
                const int n = XLookupString(&ev.xkey, buf, static_cast<int>(sizeof(buf) - 1), nullptr, nullptr);
                if (n > 0 && static_cast<unsigned char>(buf[0]) >= 0x20) {
                    buf[n] = '\0';
                    eventBridgeTextInput(this, _desc, cb, buf);
                }
            }
        } else if (ev.type == KeyRelease) {
            const unsigned int kc = static_cast<unsigned int>(ev.xkey.keycode);
            if (kc < _keycode_down.size()) {
                _keycode_down[kc] = false;
            }
            const KeySym sym = XLookupKeysym(&ev.xkey, 0);
            const vne::events::KeyCode mapped = mapX11Keysym(sym);
            if (mapped != vne::events::KeyCode::eUnknown) {
                const std::uint8_t mods = mapX11Modifiers(ev.xkey.state);
                eventBridgeKeyUp(this, _desc, cb, mapped, mods);
            }
        } else if (ev.type == ButtonPress) {
            const unsigned int b = static_cast<unsigned int>(ev.xbutton.button);
            if (b == 4U || b == 5U || b == 6U || b == 7U) {
                const float y = (b == 4U) ? 1.0F : (b == 5U) ? -1.0F : 0.0F;
                const float x = (b == 6U) ? 1.0F : (b == 7U) ? -1.0F : 0.0F;
                eventBridgeMouseScroll(this, _desc, cb, x, y);
            } else {
                const std::uint8_t mods = mapX11Modifiers(ev.xbutton.state);
                const vne::events::MouseButton mb = x11ButtonToMouseButton(b);
                eventBridgeMouseButton(this,
                                       _desc,
                                       cb,
                                       mb,
                                       true,
                                       static_cast<double>(ev.xbutton.x),
                                       static_cast<double>(ev.xbutton.y),
                                       mods);
            }
        } else if (ev.type == ButtonRelease) {
            const unsigned int b = static_cast<unsigned int>(ev.xbutton.button);
            if (b >= 4U && b <= 7U) {
                continue;
            }
            const std::uint8_t mods = mapX11Modifiers(ev.xbutton.state);
            const vne::events::MouseButton mb = x11ButtonToMouseButton(b);
            eventBridgeMouseButton(this,
                                   _desc,
                                   cb,
                                   mb,
                                   false,
                                   static_cast<double>(ev.xbutton.x),
                                   static_cast<double>(ev.xbutton.y),
                                   mods);
        } else if (ev.type == MotionNotify) {
            const std::uint8_t mods = mapX11Modifiers(ev.xmotion.state);
            eventBridgeMouseMove(this,
                                 _desc,
                                 cb,
                                 static_cast<double>(ev.xmotion.x),
                                 static_cast<double>(ev.xmotion.y),
                                 mods);
        } else if (ev.type == FocusIn) {
            eventBridgeWindowFocus(this, _desc, cb, true);
            if (_owner) {
                WindowEventData_C data{};
                data.type = WindowEventType_TP::FOCUS;
                data.focused = true;
                _owner->NotifyWindowEvent(this, data);
            }
        } else if (ev.type == FocusOut) {
            eventBridgeWindowFocus(this, _desc, cb, false);
            if (_owner) {
                WindowEventData_C data{};
                data.type = WindowEventType_TP::FOCUS;
                data.focused = false;
                _owner->NotifyWindowEvent(this, data);
            }
        }
    }
}

void X11Window_C::SwapBuffers() {}

void X11Window_C::SetTitle(const std::string& title) {
    _desc.title = title;
    if (_display && _window) {
        XStoreName(_display, _window, title.c_str());
        XFlush(_display);
    }
}

void X11Window_C::SetWindowMode(WindowMode_TP mode) {
    _desc.mode = mode;
    if (!_display || !_window) {
        return;
    }
    if (mode == WindowMode_TP::FULLSCREEN) {
        SetFullscreen(true);
        return;
    }
    if (_fullscreen) {
        SetFullscreen(false);
    }
    if (mode == WindowMode_TP::BORDERLESS) {
        struct MotifWmHints {
            uint32_t flags;
            uint32_t functions;
            uint32_t decorations;
            int32_t input_mode;
            uint32_t status;
        } hints{2U, 0U, 0U, 0, 0U};
        Atom hints_atom = XInternAtom(_display, "_MOTIF_WM_HINTS", False);
        XChangeProperty(_display,
                        _window,
                        hints_atom,
                        hints_atom,
                        32,
                        PropModeReplace,
                        reinterpret_cast<unsigned char*>(&hints),
                        5);
    }
}

WindowMode_TP X11Window_C::GetWindowMode() const {
    return _desc.mode;
}

void X11Window_C::send_ewmh_state(bool add, Atom atom1, Atom atom2) {
    if (!_display || !_window) {
        return;
    }
    XEvent ev{};
    ev.type = ClientMessage;
    ev.xclient.window = _window;
    ev.xclient.message_type = XInternAtom(_display, "_NET_WM_STATE", False);
    ev.xclient.format = 32;
    ev.xclient.data.l[0] = add ? 1 : 0;  // 1=add, 0=remove
    ev.xclient.data.l[1] = static_cast<long>(atom1);
    ev.xclient.data.l[2] = static_cast<long>(atom2);
    ev.xclient.data.l[3] = 1;  // source: application
    XSendEvent(_display, _root, False, SubstructureRedirectMask | SubstructureNotifyMask, &ev);
    XFlush(_display);
}

void X11Window_C::SetFullscreen(bool enabled) {
    if (!_display || !_window || enabled == _fullscreen) {
        return;
    }
    Atom fs = XInternAtom(_display, "_NET_WM_STATE_FULLSCREEN", False);
    send_ewmh_state(enabled, fs);
    _fullscreen = enabled;
}

bool X11Window_C::IsFullscreen() const {
    return _fullscreen;
}

void X11Window_C::Minimize() {
    if (_display && _window) {
        XIconifyWindow(_display, _window, _screen);
        XFlush(_display);
    }
}

void X11Window_C::Maximize() {
    if (!_display || !_window) {
        return;
    }
    Atom maxH = XInternAtom(_display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
    Atom maxV = XInternAtom(_display, "_NET_WM_STATE_MAXIMIZED_VERT", False);
    send_ewmh_state(true, maxH, maxV);
}

void X11Window_C::Restore() {
    if (_display && _window) {
        // Unset maximized states first, then map
        Atom maxH = XInternAtom(_display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
        Atom maxV = XInternAtom(_display, "_NET_WM_STATE_MAXIMIZED_VERT", False);
        send_ewmh_state(false, maxH, maxV);
        XMapWindow(_display, _window);
        XFlush(_display);
    }
}

void X11Window_C::SetWindowLimits(const WindowLimits_C& limits) {
    _desc.limits = limits;
    if (!_display || !_window) {
        return;
    }
    XSizeHints* hints = XAllocSizeHints();
    if (!hints) {
        return;
    }
    hints->flags = 0;
    if (limits.has_min_size) {
        hints->flags |= PMinSize;
        hints->min_width = static_cast<int>(limits.min_size.width);
        hints->min_height = static_cast<int>(limits.min_size.height);
    }
    if (limits.has_max_size) {
        hints->flags |= PMaxSize;
        hints->max_width = static_cast<int>(limits.max_size.width);
        hints->max_height = static_cast<int>(limits.max_size.height);
    }
    XSetWMNormalHints(_display, _window, hints);
    XFree(hints);
    XFlush(_display);
}

void X11Window_C::SetCursor(WindowCursor_TP cursor) {
    if (!_display || !_window) {
        return;
    }
    switch (cursor) {
        case WindowCursor_TP::HIDDEN:
        case WindowCursor_TP::DISABLED: {
            // Create an invisible cursor using a 1×1 blank pixmap
            if (_blank_cursor == None) {
                static const char kBlank = 0;
                Pixmap pix = XCreateBitmapFromData(_display, _window, &kBlank, 1, 1);
                XColor black{};
                _blank_cursor = XCreatePixmapCursor(_display, pix, pix, &black, &black, 0, 0);
                XFreePixmap(_display, pix);
            }
            XDefineCursor(_display, _window, _blank_cursor);
            break;
        }
        case WindowCursor_TP::NORMAL:
        default:
            XUndefineCursor(_display, _window);
            break;
    }
    XFlush(_display);
}

void X11Window_C::SetPosition(int x, int y) {
    _desc.position.x = x;
    _desc.position.y = y;
    if (_display && _window) {
        XMoveWindow(_display, _window, x, y);
        XFlush(_display);
    }
}

void X11Window_C::GetPosition(int& x, int& y) const {
    x = _desc.position.x;
    y = _desc.position.y;
}

void X11Window_C::Resize(uint32_t width, uint32_t height) {
    _desc.size.width = width;
    _desc.size.height = height;
    if (_display && _window) {
        XResizeWindow(_display, _window, static_cast<unsigned>(width), static_cast<unsigned>(height));
        XFlush(_display);
    }
}

void X11Window_C::Close() {
    destroy();
}

bool X11Window_C::IsOpen() const {
    return _open && _window != 0;
}

void* X11Window_C::GetNativeWindow() const {
    return reinterpret_cast<void*>(static_cast<uintptr_t>(_window));
}

NativeWindowHandle_C X11Window_C::GetNativeHandle() const {
    NativeWindowHandle_C handle{};
    handle.api = WindowAPI_TP::X11_WINDOW;
    handle.x11_display = _display;
    handle.x11_window_id = static_cast<uint32_t>(_window);
    handle.xcb_connection = _xcb_connection;
    handle.xcb_window_id = static_cast<uint32_t>(_window);
    return handle;
}

WindowAPI_TP X11Window_C::GetWindowAPI() const {
    return WindowAPI_TP::X11_WINDOW;
}

int X11Window_C::GetWidth() const {
    return static_cast<int>(_desc.size.width);
}

int X11Window_C::GetHeight() const {
    return static_cast<int>(_desc.size.height);
}

float X11Window_C::GetDPIScale() const {
    if (!_display) {
        return 1.0F;
    }
    const int width_px = DisplayWidth(_display, _screen);
    const int width_mm = DisplayWidthMM(_display, _screen);
    if (width_px <= 0 || width_mm <= 0) {
        return 1.0F;
    }
    const float dpi = (static_cast<float>(width_px) * 25.4F) / static_cast<float>(width_mm);
    return dpi / 96.0F;
}

}  // namespace vne::xwin
