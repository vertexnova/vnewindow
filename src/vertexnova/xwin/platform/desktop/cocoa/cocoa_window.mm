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

#include "cocoa_window.h"

#include "cocoa_window_manager.h"

#import <Cocoa/Cocoa.h>

namespace vne::xwin {

CocoaWindow_C::CocoaWindow_C() = default;

CocoaWindow_C::~CocoaWindow_C() {
    destroy_native();
}

void CocoaWindow_C::SetEventOwner(CocoaWindowManager_C* owner) {
    _owner = owner;
}

void CocoaWindow_C::destroy_native() {
    if (_ns_window) {
        NSWindow* win = (__bridge_transfer NSWindow*)_ns_window;
        _ns_window = nullptr;
        _ns_view = nullptr;
        [win close];
    }
    _open = false;
}

void CocoaWindow_C::Initialize(const WindowDescriptor_C& descriptor) {
    destroy_native();
    _desc = descriptor;

    NSRect rect = NSMakeRect(_desc.position.x, _desc.position.y, static_cast<CGFloat>(_desc.size.width),
                             static_cast<CGFloat>(_desc.size.height));
    NSUInteger style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable;
    if (!_desc.decorated) {
        style = NSWindowStyleMaskBorderless;
    }
    NSWindow* win = [[NSWindow alloc] initWithContentRect:rect styleMask:style backing:NSBackingStoreBuffered defer:NO];
    [win setTitle:[NSString stringWithUTF8String:_desc.title.c_str()]];
    NSView* view = [win contentView];
    _ns_window = (__bridge_retained void*)win;
    _ns_view = (__bridge void*)view;
    if (_desc.visible) {
        [win orderFrontRegardless];
    }
    _open = true;
}

void CocoaWindow_C::PollEvents() {
    for (;;) {
        NSEvent* ev = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:[NSDate distantPast] inMode:NSDefaultRunLoopMode dequeue:YES];
        if (!ev) {
            break;
        }
        [NSApp sendEvent:ev];
    }
}

void CocoaWindow_C::SwapBuffers() {}

void CocoaWindow_C::SetTitle(const std::string& title) {
    _desc.title = title;
    if (_ns_window) {
        NSWindow* win = (__bridge NSWindow*)_ns_window;
        [win setTitle:[NSString stringWithUTF8String:title.c_str()]];
    }
}

void CocoaWindow_C::SetWindowMode(WindowMode_TP mode) {
    _desc.mode = mode;
}

WindowMode_TP CocoaWindow_C::GetWindowMode() const {
    return _desc.mode;
}

void CocoaWindow_C::SetFullscreen(bool enabled) {
    (void)enabled;
}

bool CocoaWindow_C::IsFullscreen() const {
    return false;
}

void CocoaWindow_C::SetPosition(int x, int y) {
    _desc.position.x = x;
    _desc.position.y = y;
    if (_ns_window) {
        NSWindow* win = (__bridge NSWindow*)_ns_window;
        [win setFrameTopLeftPoint:NSMakePoint(static_cast<CGFloat>(x), static_cast<CGFloat>(y))];
    }
}

void CocoaWindow_C::GetPosition(int& x, int& y) const {
    if (_ns_window) {
        NSWindow* win = (__bridge NSWindow*)_ns_window;
        NSRect r = win.frame;
        x = static_cast<int>(r.origin.x);
        y = static_cast<int>(r.origin.y);
        return;
    }
    x = _desc.position.x;
    y = _desc.position.y;
}

void CocoaWindow_C::Resize(uint32_t width, uint32_t height) {
    _desc.size.width = width;
    _desc.size.height = height;
    if (_ns_window) {
        NSWindow* win = (__bridge NSWindow*)_ns_window;
        NSRect f = win.frame;
        f.size.width = static_cast<CGFloat>(width);
        f.size.height = static_cast<CGFloat>(height);
        [win setFrame:f display:YES];
    }
}

void CocoaWindow_C::Close() {
    destroy_native();
}

bool CocoaWindow_C::IsOpen() const {
    return _open && _ns_window != nullptr;
}

void* CocoaWindow_C::GetNativeWindow() const {
    return _ns_view;
}

WindowAPI_TP CocoaWindow_C::GetWindowAPI() const {
    return WindowAPI_TP::COCOA_WINDOW;
}

int CocoaWindow_C::GetWidth() const {
    return static_cast<int>(_desc.size.width);
}

int CocoaWindow_C::GetHeight() const {
    return static_cast<int>(_desc.size.height);
}

float CocoaWindow_C::GetDPIScale() const {
    if (!_ns_window) {
        return 1.0F;
    }
    NSWindow* win = (__bridge NSWindow*)_ns_window;
    NSScreen* screen = win.screen ? win.screen : [NSScreen mainScreen];
    if (!screen) {
        return 1.0F;
    }
    return static_cast<float>(screen.backingScaleFactor);
}

}  // namespace vne::xwin
