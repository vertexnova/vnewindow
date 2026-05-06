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
#include "event_bridge.h"

#include <vertexnova/xwin/input_mapping.h>

#include <vertexnova/events/types.h>

#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <cstdint>

namespace vne::xwin {

X11Window_C::X11Window_C() = default;

X11Window_C::~X11Window_C() {
    destroy();
}

void X11Window_C::SetEventOwner(X11WindowManager_C* owner) {
    owner_ = owner;
}

void X11Window_C::SetDisplay(Display* display, int screen, ::Window root, void* xcb_connection) {
    display_ = display;
    screen_ = screen;
    root_ = root;
    xcb_connection_ = xcb_connection;
}

void X11Window_C::destroy() {
    if (display_ && window_) {
        XDestroyWindow(display_, window_);
        window_ = 0;
    }
    open_ = false;
}

void X11Window_C::Initialize(const WindowDescriptor& descriptor) {
    destroy();
    if (!display_) {
        return;
    }
    desc_ = descriptor;
    const unsigned long black = BlackPixel(display_, screen_);
    const unsigned long white = WhitePixel(display_, screen_);
    window_ = XCreateSimpleWindow(display_,
                                  root_,
                                  desc_.position.x,
                                  desc_.position.y,
                                  static_cast<unsigned>(desc_.size.width),
                                  static_cast<unsigned>(desc_.size.height),
                                  0,
                                  black,
                                  white);
    if (!window_) {
        return;
    }
    XSelectInput(display_,
                 window_,
                 ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask
                     | StructureNotifyMask | FocusChangeMask);
    wm_delete_ = XInternAtom(display_, "WM_DELETE_WINDOW", False);
    Atom protocols[] = {wm_delete_};
    XSetWMProtocols(display_, window_, protocols, 1);

    XStoreName(display_, window_, desc_.title.c_str());

    if (desc_.visible) {
        XMapWindow(display_, window_);
    }
    XFlush(display_);

    // Apply size limits from descriptor
    if (desc_.limits.has_min_size || desc_.limits.has_max_size) {
        SetWindowLimits(desc_.limits);
    }

    open_ = true;
}

void X11Window_C::PollEvents() {
    if (!display_ || !window_) {
        return;
    }
    const EventBridgeCallbacks empty_callbacks{};
    const EventBridgeCallbacks& cb = owner_ ? owner_->eventBridgeCallbacks() : empty_callbacks;

    XEvent ev{};
    while (XPending(display_) > 0) {
        XNextEvent(display_, &ev);
        // SelectionRequest is addressed to us even when xany.window differs
        if (ev.type == SelectionRequest) {
            handle_selection_request(ev.xselectionrequest);
            continue;
        }
        if (ev.xany.window != window_) {
            continue;
        }
        if (ev.type == ClientMessage) {
            if (static_cast<Atom>(ev.xclient.data.l[0]) == wm_delete_) {
                eventBridgeWindowClose(this, desc_, cb);
                open_ = false;
                if (owner_) {
                    WindowEventData data{};
                    data.type = WindowEventType::eClose;
                    owner_->NotifyWindowEvent(this, data);
                }
            }
        } else if (ev.type == ConfigureNotify) {
            desc_.size.width = static_cast<uint32_t>(ev.xconfigure.width);
            desc_.size.height = static_cast<uint32_t>(ev.xconfigure.height);
            eventBridgeWindowResize(this, desc_, cb, desc_.size.width, desc_.size.height);
            if (owner_) {
                WindowEventData data{};
                data.type = WindowEventType::eResize;
                data.size = desc_.size;
                owner_->NotifyWindowEvent(this, data);
            }
        } else if (ev.type == KeyPress) {
            const unsigned int kc = static_cast<unsigned int>(ev.xkey.keycode);
            const KeySym sym = XLookupKeysym(&ev.xkey, 0);
            const vne::events::KeyCode mapped = mapNativeKeyToEvents(WindowAPI::eX11Window,
                                                                     packXkbNativeKey(static_cast<std::uint32_t>(sym)),
                                                                     desc_.input_mapping);
            if (mapped != vne::events::KeyCode::eUnknown && kc < keycode_down_.size()) {
                const bool repeat = keycode_down_[kc];
                keycode_down_[kc] = true;
                const std::uint8_t mods = mapNativeModifiersToEvents(WindowAPI::eX11Window,
                                                                     static_cast<std::uint64_t>(ev.xkey.state),
                                                                     desc_.input_mapping);
                eventBridgeKeyDown(this, desc_, cb, mapped, mods, repeat);
            }
            // Text input: decode printable characters via XLookupString
            if (desc_.enable_events || cb.onTextInput) {
                char buf[32] = {};
                const int n = XLookupString(&ev.xkey, buf, static_cast<int>(sizeof(buf) - 1), nullptr, nullptr);
                if (n > 0 && static_cast<unsigned char>(buf[0]) >= 0x20) {
                    buf[n] = '\0';
                    eventBridgeTextInput(this, desc_, cb, buf);
                }
            }
        } else if (ev.type == KeyRelease) {
            const unsigned int kc = static_cast<unsigned int>(ev.xkey.keycode);
            if (kc < keycode_down_.size()) {
                keycode_down_[kc] = false;
            }
            const KeySym sym = XLookupKeysym(&ev.xkey, 0);
            const vne::events::KeyCode mapped = mapNativeKeyToEvents(WindowAPI::eX11Window,
                                                                     packXkbNativeKey(static_cast<std::uint32_t>(sym)),
                                                                     desc_.input_mapping);
            if (mapped != vne::events::KeyCode::eUnknown) {
                const std::uint8_t mods = mapNativeModifiersToEvents(WindowAPI::eX11Window,
                                                                     static_cast<std::uint64_t>(ev.xkey.state),
                                                                     desc_.input_mapping);
                eventBridgeKeyUp(this, desc_, cb, mapped, mods);
            }
        } else if (ev.type == ButtonPress) {
            const unsigned int b = static_cast<unsigned int>(ev.xbutton.button);
            if (b == 4U || b == 5U || b == 6U || b == 7U) {
                const float y = (b == 4U) ? 1.0F : (b == 5U) ? -1.0F : 0.0F;
                const float x = (b == 6U) ? 1.0F : (b == 7U) ? -1.0F : 0.0F;
                eventBridgeMouseScroll(this, desc_, cb, x, y);
            } else {
                const std::uint8_t mods = mapNativeModifiersToEvents(WindowAPI::eX11Window,
                                                                     static_cast<std::uint64_t>(ev.xbutton.state),
                                                                     desc_.input_mapping);
                const vne::events::MouseButton mb =
                    mapNativeMouseToEvents(WindowAPI::eX11Window, packX11NativeMouse(b), desc_.input_mapping);
                eventBridgeMouseButton(this,
                                       desc_,
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
            const std::uint8_t mods = mapNativeModifiersToEvents(WindowAPI::eX11Window,
                                                                 static_cast<std::uint64_t>(ev.xbutton.state),
                                                                 desc_.input_mapping);
            const vne::events::MouseButton mb =
                mapNativeMouseToEvents(WindowAPI::eX11Window, packX11NativeMouse(b), desc_.input_mapping);
            eventBridgeMouseButton(this,
                                   desc_,
                                   cb,
                                   mb,
                                   false,
                                   static_cast<double>(ev.xbutton.x),
                                   static_cast<double>(ev.xbutton.y),
                                   mods);
        } else if (ev.type == MotionNotify) {
            const std::uint8_t mods = mapNativeModifiersToEvents(WindowAPI::eX11Window,
                                                                 static_cast<std::uint64_t>(ev.xmotion.state),
                                                                 desc_.input_mapping);
            eventBridgeMouseMove(this,
                                 desc_,
                                 cb,
                                 static_cast<double>(ev.xmotion.x),
                                 static_cast<double>(ev.xmotion.y),
                                 mods);
        } else if (ev.type == FocusIn) {
            eventBridgeWindowFocus(this, desc_, cb, true);
            if (owner_) {
                WindowEventData data{};
                data.type = WindowEventType::eFocus;
                data.focused = true;
                owner_->NotifyWindowEvent(this, data);
            }
        } else if (ev.type == FocusOut) {
            eventBridgeWindowFocus(this, desc_, cb, false);
            if (owner_) {
                WindowEventData data{};
                data.type = WindowEventType::eFocus;
                data.focused = false;
                owner_->NotifyWindowEvent(this, data);
            }
        }
    }
}

void X11Window_C::SwapBuffers() {}

void X11Window_C::SetTitle(const std::string& title) {
    desc_.title = title;
    if (display_ && window_) {
        XStoreName(display_, window_, title.c_str());
        XFlush(display_);
    }
}

void X11Window_C::SetWindowMode(WindowMode mode) {
    desc_.mode = mode;
    if (!display_ || !window_) {
        return;
    }
    if (mode == WindowMode::eFullscreen) {
        SetFullscreen(true);
        return;
    }
    if (fullscreen_) {
        SetFullscreen(false);
    }
    if (mode == WindowMode::eBorderless) {
        struct MotifWmHints {
            uint32_t flags;
            uint32_t functions;
            uint32_t decorations;
            int32_t input_mode;
            uint32_t status;
        } hints{2U, 0U, 0U, 0, 0U};
        Atom hints_atom = XInternAtom(display_, "_MOTIF_WM_HINTS", False);
        XChangeProperty(display_,
                        window_,
                        hints_atom,
                        hints_atom,
                        32,
                        PropModeReplace,
                        reinterpret_cast<unsigned char*>(&hints),
                        5);
    }
}

WindowMode X11Window_C::GetWindowMode() const {
    return desc_.mode;
}

void X11Window_C::send_ewmh_state(bool add, Atom atom1, Atom atom2) {
    if (!display_ || !window_) {
        return;
    }
    XEvent ev{};
    ev.type = ClientMessage;
    ev.xclient.window = window_;
    ev.xclient.message_type = XInternAtom(display_, "_NET_WM_STATE", False);
    ev.xclient.format = 32;
    ev.xclient.data.l[0] = add ? 1 : 0;  // 1=add, 0=remove
    ev.xclient.data.l[1] = static_cast<long>(atom1);
    ev.xclient.data.l[2] = static_cast<long>(atom2);
    ev.xclient.data.l[3] = 1;  // source: application
    XSendEvent(display_, root_, False, SubstructureRedirectMask | SubstructureNotifyMask, &ev);
    XFlush(display_);
}

void X11Window_C::SetFullscreen(bool enabled) {
    if (!display_ || !window_ || enabled == fullscreen_) {
        return;
    }
    Atom fs = XInternAtom(display_, "_NET_WM_STATE_FULLSCREEN", False);
    send_ewmh_state(enabled, fs);
    fullscreen_ = enabled;
}

bool X11Window_C::IsFullscreen() const {
    return fullscreen_;
}

void X11Window_C::Minimize() {
    if (display_ && window_) {
        XIconifyWindow(display_, window_, screen_);
        XFlush(display_);
    }
}

void X11Window_C::Maximize() {
    if (!display_ || !window_) {
        return;
    }
    Atom maxH = XInternAtom(display_, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
    Atom maxV = XInternAtom(display_, "_NET_WM_STATE_MAXIMIZED_VERT", False);
    send_ewmh_state(true, maxH, maxV);
}

void X11Window_C::Restore() {
    if (display_ && window_) {
        // Unset maximized states first, then map
        Atom maxH = XInternAtom(display_, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
        Atom maxV = XInternAtom(display_, "_NET_WM_STATE_MAXIMIZED_VERT", False);
        send_ewmh_state(false, maxH, maxV);
        XMapWindow(display_, window_);
        XFlush(display_);
    }
}

void X11Window_C::SetWindowLimits(const WindowLimits& limits) {
    desc_.limits = limits;
    if (!display_ || !window_) {
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
    XSetWMNormalHints(display_, window_, hints);
    XFree(hints);
    XFlush(display_);
}

void X11Window_C::SetCursor(WindowCursor cursor) {
    if (!display_ || !window_) {
        return;
    }
    switch (cursor) {
        case WindowCursor::eHidden:
        case WindowCursor::eDisabled: {
            // Create an invisible cursor using a 1×1 blank pixmap
            if (blank_cursor_ == None) {
                static const char kBlank = 0;
                Pixmap pix = XCreateBitmapFromData(display_, window_, &kBlank, 1, 1);
                XColor black{};
                blank_cursor_ = XCreatePixmapCursor(display_, pix, pix, &black, &black, 0, 0);
                XFreePixmap(display_, pix);
            }
            XDefineCursor(display_, window_, blank_cursor_);
            break;
        }
        case WindowCursor::eNormal:
        default:
            XUndefineCursor(display_, window_);
            break;
    }
    XFlush(display_);
}

void X11Window_C::SetPosition(int x, int y) {
    desc_.position.x = x;
    desc_.position.y = y;
    if (display_ && window_) {
        XMoveWindow(display_, window_, x, y);
        XFlush(display_);
    }
}

void X11Window_C::GetPosition(int& x, int& y) const {
    x = desc_.position.x;
    y = desc_.position.y;
}

void X11Window_C::Resize(uint32_t width, uint32_t height) {
    desc_.size.width = width;
    desc_.size.height = height;
    if (display_ && window_) {
        XResizeWindow(display_, window_, static_cast<unsigned>(width), static_cast<unsigned>(height));
        XFlush(display_);
    }
}

void X11Window_C::Close() {
    destroy();
}

bool X11Window_C::IsOpen() const {
    return open_ && window_ != 0;
}

void* X11Window_C::GetNativeWindow() const {
    return reinterpret_cast<void*>(static_cast<uintptr_t>(window_));
}

NativeWindowHandle X11Window_C::GetNativeHandle() const {
    NativeWindowHandle handle{};
    handle.api = WindowAPI::eX11Window;
    handle.x11_display = display_;
    handle.x11_window_id = static_cast<uint32_t>(window_);
    handle.xcb_connection = xcb_connection_;
    handle.xcb_window_id = static_cast<uint32_t>(window_);
    return handle;
}

WindowAPI X11Window_C::GetWindowAPI() const {
    return WindowAPI::eX11Window;
}

int X11Window_C::GetWidth() const {
    return static_cast<int>(desc_.size.width);
}

int X11Window_C::GetHeight() const {
    return static_cast<int>(desc_.size.height);
}

float X11Window_C::GetDPIScale() const {
    if (!display_) {
        return 1.0F;
    }
    const int width_px = DisplayWidth(display_, screen_);
    const int width_mm = DisplayWidthMM(display_, screen_);
    if (width_px <= 0 || width_mm <= 0) {
        return 1.0F;
    }
    const float dpi = (static_cast<float>(width_px) * 25.4F) / static_cast<float>(width_mm);
    return dpi / 96.0F;
}

}  // namespace vne::xwin
