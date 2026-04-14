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

#include <X11/Xutil.h>

namespace vne::xwin {

X11Window_C::X11Window_C() = default;

X11Window_C::~X11Window_C() {
    destroy();
}

void X11Window_C::SetEventOwner(X11WindowManager_C* owner) {
    _owner = owner;
}

void X11Window_C::SetDisplay(Display* display, int screen, ::Window root) {
    _display = display;
    _screen = screen;
    _root = root;
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
    XSelectInput(_display, _window, ExposureMask | KeyPressMask | StructureNotifyMask);
    _wm_delete = XInternAtom(_display, "WM_DELETE_WINDOW", False);
    Atom protocols[] = {_wm_delete};
    XSetWMProtocols(_display, _window, protocols, 1);

    XStoreName(_display, _window, _desc.title.c_str());

    if (_desc.visible) {
        XMapWindow(_display, _window);
    }
    XFlush(_display);
    _open = true;
}

void X11Window_C::PollEvents() {
    if (!_display || !_window) {
        return;
    }
    XEvent ev{};
    while (XPending(_display) > 0) {
        XNextEvent(_display, &ev);
        if (ev.xany.window != _window) {
            continue;
        }
        if (ev.type == ClientMessage) {
            if (static_cast<Atom>(ev.xclient.data.l[0]) == _wm_delete) {
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
            if (_owner) {
                WindowEventData_C data{};
                data.type = WindowEventType_TP::RESIZE;
                data.size = _desc.size;
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
}

WindowMode_TP X11Window_C::GetWindowMode() const {
    return _desc.mode;
}

void X11Window_C::SetFullscreen(bool enabled) {
    (void)enabled;
}

bool X11Window_C::IsFullscreen() const {
    return false;
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

WindowAPI_TP X11Window_C::GetWindowAPI() const {
    return WindowAPI_TP::X11_WINDOW;
}

int X11Window_C::GetWidth() const {
    return static_cast<int>(_desc.size.width);
}

int X11Window_C::GetHeight() const {
    return static_cast<int>(_desc.size.height);
}

}  // namespace vne::xwin
