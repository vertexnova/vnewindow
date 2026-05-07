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
#include <vector>

namespace vne::xwin {
namespace {
constexpr int kTextInputBufferSize = 32;
constexpr unsigned char kAsciiSpace = 0x20;
constexpr unsigned int kScrollUpButton = 4U;
constexpr unsigned int kScrollDownButton = 5U;
constexpr unsigned int kScrollLeftButton = 6U;
constexpr unsigned int kScrollRightButton = 7U;
constexpr int kX11Format32 = 32;
constexpr int kMotifHintsElementCount = 5;
constexpr float kDefaultDpi = 96.0F;
constexpr int kBitsPerItem8 = 8;
constexpr unsigned char kUtf8AsciiMask = 0x80U;
constexpr unsigned char kUtf8Lead2Mask = 0xE0U;
constexpr unsigned char kUtf8Lead3Mask = 0xF0U;
constexpr unsigned char kUtf8Lead4Mask = 0xF8U;
constexpr unsigned char kUtf8ContinuationMask = 0xC0U;
constexpr unsigned char kUtf8ContinuationPrefix = 0x80U;
constexpr unsigned char kUtf8Lead2Prefix = 0xC0U;
constexpr unsigned char kUtf8Lead3Prefix = 0xE0U;
constexpr unsigned char kUtf8Lead4Prefix = 0xF0U;
constexpr unsigned char kUtf8Lead2PayloadMask = 0x1FU;
constexpr unsigned char kUtf8Lead3PayloadMask = 0x0FU;
constexpr unsigned char kUtf8Lead4PayloadMask = 0x07U;
constexpr unsigned char kUtf8ContinuationPayloadMask = 0x3FU;
constexpr uint32_t kUtf8Shift6 = 6U;
constexpr uint32_t kUtf8Shift12 = 12U;
constexpr uint32_t kUtf8Shift18 = 18U;
constexpr uint32_t kLatin1MaxCodePoint = 0xFFU;
constexpr char kLatin1ReplacementByte = '?';

std::string utf8ToLatin1Lossy(const std::string& utf8) {
    std::string out;
    out.reserve(utf8.size());
    size_t i = 0;
    while (i < utf8.size()) {
        const auto c0 = static_cast<unsigned char>(utf8[i]);
        if ((c0 & kUtf8AsciiMask) == 0U) {
            out.push_back(static_cast<char>(c0));
            ++i;
            continue;
        }

        auto is_cont = [](unsigned char c) { return (c & kUtf8ContinuationMask) == kUtf8ContinuationPrefix; };

        uint32_t code_point = 0U;
        size_t advance = 0U;
        if ((c0 & kUtf8Lead2Mask) == kUtf8Lead2Prefix) {
            if (i + 1U < utf8.size()) {
                const auto c1 = static_cast<unsigned char>(utf8[i + 1U]);
                if (is_cont(c1)) {
                    code_point = (static_cast<uint32_t>(c0 & kUtf8Lead2PayloadMask) << kUtf8Shift6)
                                 | static_cast<uint32_t>(c1 & kUtf8ContinuationPayloadMask);
                    advance = 2U;
                }
            }
        } else if ((c0 & kUtf8Lead3Mask) == kUtf8Lead3Prefix) {
            if (i + 2U < utf8.size()) {
                const auto c1 = static_cast<unsigned char>(utf8[i + 1U]);
                const auto c2 = static_cast<unsigned char>(utf8[i + 2U]);
                if (is_cont(c1) && is_cont(c2)) {
                    code_point = (static_cast<uint32_t>(c0 & kUtf8Lead3PayloadMask) << kUtf8Shift12)
                                 | (static_cast<uint32_t>(c1 & kUtf8ContinuationPayloadMask) << kUtf8Shift6)
                                 | static_cast<uint32_t>(c2 & kUtf8ContinuationPayloadMask);
                    advance = 3U;
                }
            }
        } else if ((c0 & kUtf8Lead4Mask) == kUtf8Lead4Prefix) {
            if (i + 3U < utf8.size()) {
                const auto c1 = static_cast<unsigned char>(utf8[i + 1U]);
                const auto c2 = static_cast<unsigned char>(utf8[i + 2U]);
                const auto c3 = static_cast<unsigned char>(utf8[i + 3U]);
                if (is_cont(c1) && is_cont(c2) && is_cont(c3)) {
                    code_point = (static_cast<uint32_t>(c0 & kUtf8Lead4PayloadMask) << kUtf8Shift18)
                                 | (static_cast<uint32_t>(c1 & kUtf8ContinuationPayloadMask) << kUtf8Shift12)
                                 | (static_cast<uint32_t>(c2 & kUtf8ContinuationPayloadMask) << kUtf8Shift6)
                                 | static_cast<uint32_t>(c3 & kUtf8ContinuationPayloadMask);
                    advance = 4U;
                }
            }
        }

        if (advance == 0U) {
            out.push_back(kLatin1ReplacementByte);
            ++i;
            continue;
        }

        if (code_point <= kLatin1MaxCodePoint) {
            out.push_back(static_cast<char>(static_cast<unsigned char>(code_point)));
        } else {
            out.push_back(kLatin1ReplacementByte);
        }
        i += advance;
    }
    return out;
}
}  // namespace

X11Window::X11Window() = default;

X11Window::~X11Window() {
    destroyNative();
}

void X11Window::setEventOwner(X11WindowManager* owner) {
    owner_ = owner;
}

void X11Window::setDisplay(Display* display, int screen, ::Window root, void* xcb_connection) {
    display_ = display;
    screen_ = screen;
    root_ = root;
    xcb_connection_ = xcb_connection;
}

void X11Window::destroyNative() {
    if (display_ && window_) {
        const Atom clip = XInternAtom(display_, "CLIPBOARD", False);
        if (XGetSelectionOwner(display_, clip) == window_) {
            XSetSelectionOwner(display_, clip, None, CurrentTime);
        }
        XDestroyWindow(display_, window_);
        window_ = 0;
    }
    open_ = false;
}

void X11Window::initialize(const WindowDescriptor& descriptor) {
    destroyNative();
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
        setWindowLimits(desc_.limits);
    }

    open_ = true;
}

void X11Window::pollEvents() {
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
            handleSelectionRequest(ev.xselectionrequest);
            continue;
        }
        if (ev.xany.window != window_) {
            continue;
        }
        if (ev.type == ClientMessage) {
            if (static_cast<Atom>(ev.xclient.data.l[0]) == wm_delete_) {
                eventBridgeWindowClose(this, desc_, cb);
                open_ = false;
                destroyNative();
                if (owner_) {
                    WindowEventData data{};
                    data.type = WindowEventType::eClose;
                    owner_->notifyWindowEvent(this, data);
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
                owner_->notifyWindowEvent(this, data);
            }
        } else if (ev.type == KeyPress) {
            const auto kc = static_cast<unsigned int>(ev.xkey.keycode);
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
            if (desc_.enable_events || cb.on_text_input) {
                char buf[kTextInputBufferSize] = {};
                const int n = XLookupString(&ev.xkey, buf, static_cast<int>(sizeof(buf) - 1), nullptr, nullptr);
                if (n > 0 && static_cast<unsigned char>(buf[0]) >= kAsciiSpace) {
                    buf[n] = '\0';
                    eventBridgeTextInput(this, desc_, cb, buf);
                }
            }
        } else if (ev.type == KeyRelease) {
            const auto kc = static_cast<unsigned int>(ev.xkey.keycode);
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
            const auto b = static_cast<unsigned int>(ev.xbutton.button);
            if (b == kScrollUpButton || b == kScrollDownButton || b == kScrollLeftButton || b == kScrollRightButton) {
                const float y = (b == kScrollUpButton) ? 1.0F : (b == kScrollDownButton) ? -1.0F : 0.0F;
                const float x = (b == kScrollLeftButton) ? 1.0F : (b == kScrollRightButton) ? -1.0F : 0.0F;
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
            const auto b = static_cast<unsigned int>(ev.xbutton.button);
            if (b >= kScrollUpButton && b <= kScrollRightButton) {
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
                owner_->notifyWindowEvent(this, data);
            }
        } else if (ev.type == FocusOut) {
            eventBridgeWindowFocus(this, desc_, cb, false);
            if (owner_) {
                WindowEventData data{};
                data.type = WindowEventType::eFocus;
                data.focused = false;
                owner_->notifyWindowEvent(this, data);
            }
        }
    }
}

void X11Window::swapBuffers() {}

void X11Window::setTitle(const std::string& title) {
    desc_.title = title;
    if (display_ && window_) {
        XStoreName(display_, window_, title.c_str());
        XFlush(display_);
    }
}

void X11Window::setWindowMode(WindowMode mode) {
    desc_.mode = mode;
    if (!display_ || !window_) {
        return;
    }
    if (mode == WindowMode::eFullscreen) {
        setFullscreen(true);
        return;
    }
    if (fullscreen_) {
        setFullscreen(false);
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
                        kX11Format32,
                        PropModeReplace,
                        reinterpret_cast<unsigned char*>(&hints),
                        kMotifHintsElementCount);
    }
}

WindowMode X11Window::getWindowMode() const noexcept {
    return desc_.mode;
}

void X11Window::sendEwmhState(bool add, Atom atom1, Atom atom2) {
    if (!display_ || !window_) {
        return;
    }
    XEvent ev{};
    ev.type = ClientMessage;
    ev.xclient.window = window_;
    ev.xclient.message_type = XInternAtom(display_, "_NET_WM_STATE", False);
    ev.xclient.format = kX11Format32;
    ev.xclient.data.l[0] = add ? 1 : 0;  // 1=add, 0=remove
    ev.xclient.data.l[1] = static_cast<long>(atom1);
    ev.xclient.data.l[2] = static_cast<long>(atom2);
    ev.xclient.data.l[3] = 1;  // source: application
    XSendEvent(display_, root_, False, SubstructureRedirectMask | SubstructureNotifyMask, &ev);
    XFlush(display_);
}

void X11Window::setFullscreen(bool enabled) {
    if (!display_ || !window_ || enabled == fullscreen_) {
        return;
    }
    Atom fs = XInternAtom(display_, "_NET_WM_STATE_FULLSCREEN", False);
    sendEwmhState(enabled, fs);
    fullscreen_ = enabled;
}

bool X11Window::isFullscreen() const noexcept {
    return fullscreen_;
}

void X11Window::minimize() {
    if (display_ && window_) {
        XIconifyWindow(display_, window_, screen_);
        XFlush(display_);
    }
}

void X11Window::maximize() {
    if (!display_ || !window_) {
        return;
    }
    Atom max_h = XInternAtom(display_, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
    Atom max_v = XInternAtom(display_, "_NET_WM_STATE_MAXIMIZED_VERT", False);
    sendEwmhState(true, max_h, max_v);
}

void X11Window::restore() {
    if (display_ && window_) {
        // Unset maximized states first, then map
        Atom max_h = XInternAtom(display_, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
        Atom max_v = XInternAtom(display_, "_NET_WM_STATE_MAXIMIZED_VERT", False);
        sendEwmhState(false, max_h, max_v);
        XMapWindow(display_, window_);
        XFlush(display_);
    }
}

void X11Window::setWindowLimits(const WindowLimits& limits) {
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

void X11Window::setCursor(WindowCursor cursor) {
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

void X11Window::setPosition(int x, int y) {
    desc_.position.x = x;
    desc_.position.y = y;
    if (display_ && window_) {
        XMoveWindow(display_, window_, x, y);
        XFlush(display_);
    }
}

WindowPosition X11Window::getPosition() const {
    return desc_.position;
}

void X11Window::resize(uint32_t width, uint32_t height) {
    desc_.size.width = width;
    desc_.size.height = height;
    if (display_ && window_) {
        XResizeWindow(display_, window_, static_cast<unsigned>(width), static_cast<unsigned>(height));
        XFlush(display_);
    }
}

void X11Window::close() {
    destroyNative();
}

bool X11Window::isOpen() const noexcept {
    return open_ && window_ != 0;
}

NativeWindowHandle X11Window::getNativeHandle() const noexcept {
    NativeWindowHandle handle{};
    handle.api = WindowAPI::eX11Window;
    handle.x11_display = display_;
    handle.x11_window_id = static_cast<uint32_t>(window_);
    handle.xcb_connection = xcb_connection_;
    handle.xcb_window_id = static_cast<uint32_t>(window_);
    return handle;
}

WindowAPI X11Window::getWindowAPI() const noexcept {
    return WindowAPI::eX11Window;
}

int X11Window::getWidth() const noexcept {
    return static_cast<int>(desc_.size.width);
}

int X11Window::getHeight() const noexcept {
    return static_cast<int>(desc_.size.height);
}

float X11Window::getDpiScale() const noexcept {
    if (!display_) {
        return 1.0F;
    }
    const int width_px = DisplayWidth(display_, screen_);
    const int width_mm = DisplayWidthMM(display_, screen_);
    if (width_px <= 0 || width_mm <= 0) {
        return 1.0F;
    }
    const float dpi = (static_cast<float>(width_px) * 25.4F) / static_cast<float>(width_mm);
    return dpi / kDefaultDpi;
}

std::string X11Window::getClipboardText() const {
    if (!display_ || !window_) {
        return {};
    }
    const Atom clip = XInternAtom(display_, "CLIPBOARD", False);
    const ::Window owner = XGetSelectionOwner(display_, clip);
    if (owner == None) {
        return {};
    }
    if (owner == window_) {
        return clipboard_text_;
    }
    /* Reading another client's CLIPBOARD requires an async protocol (XConvertSelection + SelectionNotify). */
    return {};
}

void X11Window::setClipboardText(const std::string& text) {
    if (!display_ || !window_) {
        return;
    }
    clipboard_text_ = text;
    const Atom clip = XInternAtom(display_, "CLIPBOARD", False);
    XSetSelectionOwner(display_, clip, window_, CurrentTime);
    XFlush(display_);
}

void X11Window::setWindowIcon(std::span<const uint8_t> rgba_pixels, uint32_t width, uint32_t height) {
    const size_t expected_bytes = static_cast<size_t>(width) * static_cast<size_t>(height) * 4U;
    if (!display_ || !window_ || rgba_pixels.empty() || width == 0U || height == 0U
        || rgba_pixels.size() < expected_bytes) {
        return;
    }
    std::vector<long> icon;
    icon.reserve(static_cast<size_t>(2U + width * height));
    icon.push_back(static_cast<long>(width));
    icon.push_back(static_cast<long>(height));
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            const size_t i = (static_cast<size_t>(y) * width + x) * 4U;
            const auto r = static_cast<unsigned long>(rgba_pixels[i]);
            const auto g = static_cast<unsigned long>(rgba_pixels[i + 1U]);
            const auto b = static_cast<unsigned long>(rgba_pixels[i + 2U]);
            const auto a = static_cast<unsigned long>(rgba_pixels[i + 3U]);
            const unsigned long argb = (a << 24U) | (r << 16U) | (g << 8U) | b;
            icon.push_back(static_cast<long>(argb));
        }
    }
    const Atom net_wm_icon = XInternAtom(display_, "_NET_WM_ICON", False);
    XChangeProperty(display_,
                    window_,
                    net_wm_icon,
                    XA_CARDINAL,
                    kX11Format32,
                    PropModeReplace,
                    reinterpret_cast<unsigned char*>(icon.data()),
                    static_cast<int>(icon.size()));
    XFlush(display_);
}

void X11Window::handleSelectionRequest(const XSelectionRequestEvent& req) {
    if (!display_) {
        return;
    }
    XSelectionEvent notify{};
    notify.type = SelectionNotify;
    notify.send_event = True;
    notify.display = req.display;
    notify.requestor = req.requestor;
    notify.selection = req.selection;
    notify.target = req.target;
    notify.property = None;
    notify.time = req.time;

    const Atom clip = XInternAtom(display_, "CLIPBOARD", False);
    if (req.selection != clip || req.property == None) {
        XSendEvent(display_, req.requestor, False, NoEventMask, reinterpret_cast<XEvent*>(&notify));
        return;
    }

    const Atom utf8 = XInternAtom(display_, "UTF8_STRING", False);
    const Atom targets = XInternAtom(display_, "TARGETS", False);

    if (req.target == targets) {
        const Atom offered[] = {utf8, XA_STRING, targets};
        XChangeProperty(display_,
                        req.requestor,
                        req.property,
                        XA_ATOM,
                        kX11Format32,
                        PropModeReplace,
                        reinterpret_cast<const unsigned char*>(offered),
                        static_cast<int>(sizeof(offered) / sizeof(offered[0])));
        notify.property = req.property;
    } else if (req.target == utf8) {
        XChangeProperty(display_,
                        req.requestor,
                        req.property,
                        utf8,
                        kBitsPerItem8,
                        PropModeReplace,
                        reinterpret_cast<const unsigned char*>(clipboard_text_.data()),
                        static_cast<int>(clipboard_text_.size()));
        notify.property = req.property;
    } else if (req.target == XA_STRING) {
        const std::string latin1 = utf8ToLatin1Lossy(clipboard_text_);
        XChangeProperty(display_,
                        req.requestor,
                        req.property,
                        XA_STRING,
                        kBitsPerItem8,
                        PropModeReplace,
                        reinterpret_cast<const unsigned char*>(latin1.data()),
                        static_cast<int>(latin1.size()));
        notify.property = req.property;
    }

    XSendEvent(display_, req.requestor, False, NoEventMask, reinterpret_cast<XEvent*>(&notify));
}

}  // namespace vne::xwin
