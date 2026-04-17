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

#include "cocoa_map_key.h"
#include "cocoa_window_manager.h"
#include "event_bridge.h"

#include "vertexnova/xwin/xwin_types.h"

#import <Cocoa/Cocoa.h>

// ---------------------------------------------------------------------------
// Forward declarations of our ObjC helpers
// ---------------------------------------------------------------------------
@class VneXWinView;
@class VneXWinWindowDelegate;

// ---------------------------------------------------------------------------
// VneXWinView — NSView subclass that routes keyboard + mouse to the bridge
// ---------------------------------------------------------------------------
@interface VneXWinView : NSView {
    vne::xwin::CocoaWindow_C* _xwin;
}
- (instancetype)initWithFrame:(NSRect)frame xwin:(vne::xwin::CocoaWindow_C*)xwin;
@end

@implementation VneXWinView

- (instancetype)initWithFrame:(NSRect)frame xwin:(vne::xwin::CocoaWindow_C*)xwin {
    self = [super initWithFrame:frame];
    if (self) {
        _xwin = xwin;
        // Track mouse movement even when no button is pressed
        [self addTrackingArea:[[NSTrackingArea alloc]
                                initWithRect:NSZeroRect
                                     options:NSTrackingMouseMoved | NSTrackingActiveInKeyWindow | NSTrackingInVisibleRect
                                       owner:self
                                    userInfo:nil]];
    }
    return self;
}

- (BOOL)acceptsFirstResponder { return YES; }
- (BOOL)acceptsFirstMouse:(NSEvent*)event { (void)event; return YES; }
- (BOOL)isFlipped { return YES; }  // top-left origin to match other platforms

// ---- Keyboard ----

- (void)keyDown:(NSEvent*)ev {
    if (!_xwin) { return; }
    const vne::events::KeyCode kc = vne::xwin::xwinMapCocoaKeyCode(ev.keyCode);
    const uint8_t mods = vne::xwin::xwinMapCocoaModifiers(ev.modifierFlags);
    const bool repeat = ev.isARepeat;
    _xwin->handleKeyDown(kc, mods, repeat);
}

- (void)keyUp:(NSEvent*)ev {
    if (!_xwin) { return; }
    const vne::events::KeyCode kc = vne::xwin::xwinMapCocoaKeyCode(ev.keyCode);
    const uint8_t mods = vne::xwin::xwinMapCocoaModifiers(ev.modifierFlags);
    _xwin->handleKeyUp(kc, mods);
}

- (void)flagsChanged:(NSEvent*)ev {
    if (!_xwin) { return; }
    // Treat modifier-only changes as key press/release based on current flags
    const vne::events::KeyCode kc = vne::xwin::xwinMapCocoaKeyCode(ev.keyCode);
    if (kc == vne::events::KeyCode::eUnknown) { return; }
    const uint8_t mods = vne::xwin::xwinMapCocoaModifiers(ev.modifierFlags);
    // Heuristic: if the modifier bit is still set it was just pressed, otherwise released
    const bool is_press = (mods != 0);
    if (is_press) {
        _xwin->handleKeyDown(kc, mods, false);
    } else {
        _xwin->handleKeyUp(kc, 0);
    }
}

// ---- Mouse buttons ----

- (void)mouseDown:(NSEvent*)ev {
    if (!_xwin) { return; }
    const NSPoint p = [self convertPoint:ev.locationInWindow fromView:nil];
    _xwin->handleMouseButton(vne::events::MouseButton::eLeft, true, p.x, p.y,
                             vne::xwin::xwinMapCocoaModifiers(ev.modifierFlags));
}
- (void)mouseUp:(NSEvent*)ev {
    if (!_xwin) { return; }
    const NSPoint p = [self convertPoint:ev.locationInWindow fromView:nil];
    _xwin->handleMouseButton(vne::events::MouseButton::eLeft, false, p.x, p.y,
                             vne::xwin::xwinMapCocoaModifiers(ev.modifierFlags));
}
- (void)rightMouseDown:(NSEvent*)ev {
    if (!_xwin) { return; }
    const NSPoint p = [self convertPoint:ev.locationInWindow fromView:nil];
    _xwin->handleMouseButton(vne::events::MouseButton::eRight, true, p.x, p.y,
                             vne::xwin::xwinMapCocoaModifiers(ev.modifierFlags));
}
- (void)rightMouseUp:(NSEvent*)ev {
    if (!_xwin) { return; }
    const NSPoint p = [self convertPoint:ev.locationInWindow fromView:nil];
    _xwin->handleMouseButton(vne::events::MouseButton::eRight, false, p.x, p.y,
                             vne::xwin::xwinMapCocoaModifiers(ev.modifierFlags));
}
- (void)otherMouseDown:(NSEvent*)ev {
    if (!_xwin) { return; }
    const NSPoint p = [self convertPoint:ev.locationInWindow fromView:nil];
    _xwin->handleMouseButton(vne::events::MouseButton::eMiddle, true, p.x, p.y,
                             vne::xwin::xwinMapCocoaModifiers(ev.modifierFlags));
}
- (void)otherMouseUp:(NSEvent*)ev {
    if (!_xwin) { return; }
    const NSPoint p = [self convertPoint:ev.locationInWindow fromView:nil];
    _xwin->handleMouseButton(vne::events::MouseButton::eMiddle, false, p.x, p.y,
                             vne::xwin::xwinMapCocoaModifiers(ev.modifierFlags));
}

// ---- Mouse motion ----

- (void)mouseMoved:(NSEvent*)ev {
    if (!_xwin) { return; }
    const NSPoint p = [self convertPoint:ev.locationInWindow fromView:nil];
    _xwin->handleMouseMove(p.x, p.y, vne::xwin::xwinMapCocoaModifiers(ev.modifierFlags));
}
- (void)mouseDragged:(NSEvent*)ev      { [self mouseMoved:ev]; }
- (void)rightMouseDragged:(NSEvent*)ev { [self mouseMoved:ev]; }
- (void)otherMouseDragged:(NSEvent*)ev { [self mouseMoved:ev]; }

// ---- Scroll wheel ----

- (void)scrollWheel:(NSEvent*)ev {
    if (!_xwin) { return; }
    _xwin->handleMouseScroll(static_cast<float>(ev.scrollingDeltaX),
                             static_cast<float>(ev.scrollingDeltaY));
}

@end


// ---------------------------------------------------------------------------
// VneXWinWindowDelegate — NSWindowDelegate for window-level events
// ---------------------------------------------------------------------------
@interface VneXWinWindowDelegate : NSObject<NSWindowDelegate> {
    vne::xwin::CocoaWindow_C* _xwin;
}
- (instancetype)initWithXwin:(vne::xwin::CocoaWindow_C*)xwin;
@end

@implementation VneXWinWindowDelegate

- (instancetype)initWithXwin:(vne::xwin::CocoaWindow_C*)xwin {
    self = [super init];
    if (self) { _xwin = xwin; }
    return self;
}

- (BOOL)windowShouldClose:(NSWindow*)sender {
    (void)sender;
    if (_xwin) { _xwin->handleWindowClose(); }
    return NO;  // CocoaWindow_C::Close() calls destroy_native()
}

- (void)windowDidResize:(NSNotification*)notification {
    if (!_xwin) { return; }
    NSWindow* win = notification.object;
    const NSRect r = [win.contentView frame];
    _xwin->handleWindowResize(static_cast<uint32_t>(r.size.width),
                              static_cast<uint32_t>(r.size.height));
}

- (void)windowDidBecomeKey:(NSNotification*)notification {
    (void)notification;
    if (_xwin) { _xwin->handleWindowFocus(true); }
}

- (void)windowDidResignKey:(NSNotification*)notification {
    (void)notification;
    if (_xwin) { _xwin->handleWindowFocus(false); }
}

- (void)windowDidEnterFullScreen:(NSNotification*)notification {
    (void)notification;
    if (_xwin) { _xwin->setFullscreenState(true); }
}

- (void)windowDidExitFullScreen:(NSNotification*)notification {
    (void)notification;
    if (_xwin) { _xwin->setFullscreenState(false); }
}

@end


// ---------------------------------------------------------------------------
// CocoaWindow_C implementation
// ---------------------------------------------------------------------------
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
        _ns_delegate = nullptr;
        [win setDelegate:nil];
        [win close];
    }
    _open = false;
}

void CocoaWindow_C::Initialize(const WindowDescriptor_C& descriptor) {
    destroy_native();
    _desc = descriptor;

    NSRect rect = NSMakeRect(_desc.position.x,
                             _desc.position.y,
                             static_cast<CGFloat>(_desc.size.width),
                             static_cast<CGFloat>(_desc.size.height));
    NSUInteger style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable
                       | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable;
    if (!_desc.decorated) {
        style = NSWindowStyleMaskBorderless;
    }

    NSWindow* win = [[NSWindow alloc] initWithContentRect:rect
                                                styleMask:style
                                                  backing:NSBackingStoreBuffered
                                                    defer:NO];
    [win setTitle:[NSString stringWithUTF8String:_desc.title.c_str()]];
    [win setCollectionBehavior:[win collectionBehavior] | NSWindowCollectionBehaviorFullScreenPrimary];

    // Custom content view
    VneXWinView* view = [[VneXWinView alloc] initWithFrame:rect xwin:this];
    [win setContentView:view];
    [win makeFirstResponder:view];

    // Window delegate for close/resize/focus
    VneXWinWindowDelegate* delegate = [[VneXWinWindowDelegate alloc] initWithXwin:this];
    [win setDelegate:delegate];

    _ns_window = (__bridge_retained void*)win;
    _ns_view = (__bridge void*)view;
    _ns_delegate = (__bridge_retained void*)delegate;

    if (_desc.visible) {
        [win orderFrontRegardless];
        [NSApp activateIgnoringOtherApps:YES];
    }

    // Apply size limits from descriptor
    if (_desc.limits.has_min_size) {
        [win setMinSize:NSMakeSize(_desc.limits.min_size.width, _desc.limits.min_size.height)];
    }
    if (_desc.limits.has_max_size) {
        [win setMaxSize:NSMakeSize(_desc.limits.max_size.width, _desc.limits.max_size.height)];
    }

    _open = true;
}

void CocoaWindow_C::PollEvents() {
    for (;;) {
        NSEvent* ev = [NSApp nextEventMatchingMask:NSEventMaskAny
                                         untilDate:[NSDate distantPast]
                                            inMode:NSDefaultRunLoopMode
                                           dequeue:YES];
        if (!ev) { break; }
        [NSApp sendEvent:ev];
    }
}

void CocoaWindow_C::SwapBuffers() {}

// ---- Event dispatch helpers (called from ObjC) ----

void CocoaWindow_C::handleKeyDown(vne::events::KeyCode key, uint8_t mods, bool repeat) {
    if (key == vne::events::KeyCode::eUnknown) { return; }
    const EventBridgeCallbacks_C& cb = _owner ? _owner->eventBridgeCallbacks() : _empty_callbacks;
    eventBridgeKeyDown(this, _desc, cb, key, mods, repeat);
}

void CocoaWindow_C::handleKeyUp(vne::events::KeyCode key, uint8_t mods) {
    if (key == vne::events::KeyCode::eUnknown) { return; }
    const EventBridgeCallbacks_C& cb = _owner ? _owner->eventBridgeCallbacks() : _empty_callbacks;
    eventBridgeKeyUp(this, _desc, cb, key, mods);
}

void CocoaWindow_C::handleMouseButton(vne::events::MouseButton button, bool pressed,
                                      double x, double y, uint8_t mods) {
    const EventBridgeCallbacks_C& cb = _owner ? _owner->eventBridgeCallbacks() : _empty_callbacks;
    eventBridgeMouseButton(this, _desc, cb, button, pressed, x, y, mods);
}

void CocoaWindow_C::handleMouseMove(double x, double y, uint8_t mods) {
    const EventBridgeCallbacks_C& cb = _owner ? _owner->eventBridgeCallbacks() : _empty_callbacks;
    eventBridgeMouseMove(this, _desc, cb, x, y, mods);
}

void CocoaWindow_C::handleMouseScroll(float dx, float dy) {
    const EventBridgeCallbacks_C& cb = _owner ? _owner->eventBridgeCallbacks() : _empty_callbacks;
    eventBridgeMouseScroll(this, _desc, cb, dx, dy);
}

void CocoaWindow_C::handleWindowClose() {
    const EventBridgeCallbacks_C& cb = _owner ? _owner->eventBridgeCallbacks() : _empty_callbacks;
    eventBridgeWindowClose(this, _desc, cb);
    _open = false;
    if (_owner) {
        WindowEventData_C data{};
        data.type = WindowEventType_TP::CLOSE;
        _owner->NotifyWindowEvent(this, data);
    }
    destroy_native();
}

void CocoaWindow_C::handleWindowResize(uint32_t w, uint32_t h) {
    _desc.size.width = w;
    _desc.size.height = h;
    const EventBridgeCallbacks_C& cb = _owner ? _owner->eventBridgeCallbacks() : _empty_callbacks;
    eventBridgeWindowResize(this, _desc, cb, w, h);
    if (_owner) {
        WindowEventData_C data{};
        data.type = WindowEventType_TP::RESIZE;
        data.size = _desc.size;
        _owner->NotifyWindowEvent(this, data);
    }
}

void CocoaWindow_C::handleWindowFocus(bool focused) {
    const EventBridgeCallbacks_C& cb = _owner ? _owner->eventBridgeCallbacks() : _empty_callbacks;
    eventBridgeWindowFocus(this, _desc, cb, focused);
    if (_owner) {
        WindowEventData_C data{};
        data.type = WindowEventType_TP::FOCUS;
        data.focused = focused;
        _owner->NotifyWindowEvent(this, data);
    }
}

void CocoaWindow_C::setFullscreenState(bool fs) {
    _fullscreen = fs;
}

// ---- Window_I interface ----

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
    if (!_ns_window) { return; }
    if (enabled == _fullscreen) { return; }
    NSWindow* win = (__bridge NSWindow*)_ns_window;
    [win toggleFullScreen:nil];
    // _fullscreen updated via windowDidEnterFullScreen: / windowDidExitFullScreen:
}

bool CocoaWindow_C::IsFullscreen() const {
    return _fullscreen;
}

void CocoaWindow_C::Minimize() {
    if (_ns_window) {
        [(__bridge NSWindow*)_ns_window miniaturize:nil];
    }
}

void CocoaWindow_C::Maximize() {
    if (_ns_window) {
        NSWindow* win = (__bridge NSWindow*)_ns_window;
        if (!win.isZoomed) { [win zoom:nil]; }
    }
}

void CocoaWindow_C::Restore() {
    if (_ns_window) {
        NSWindow* win = (__bridge NSWindow*)_ns_window;
        if (win.isMiniaturized) {
            [win deminiaturize:nil];
        } else if (win.isZoomed) {
            [win zoom:nil];
        }
    }
}

void CocoaWindow_C::SetPosition(int x, int y) {
    _desc.position.x = x;
    _desc.position.y = y;
    if (_ns_window) {
        [(__bridge NSWindow*)_ns_window
            setFrameTopLeftPoint:NSMakePoint(static_cast<CGFloat>(x), static_cast<CGFloat>(y))];
    }
}

void CocoaWindow_C::GetPosition(int& x, int& y) const {
    if (_ns_window) {
        NSRect r = ((__bridge NSWindow*)_ns_window).frame;
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

void CocoaWindow_C::SetWindowLimits(const WindowLimits_C& limits) {
    _desc.limits = limits;
    if (!_ns_window) { return; }
    NSWindow* win = (__bridge NSWindow*)_ns_window;
    if (limits.has_min_size) {
        [win setMinSize:NSMakeSize(limits.min_size.width, limits.min_size.height)];
    }
    if (limits.has_max_size) {
        [win setMaxSize:NSMakeSize(limits.max_size.width, limits.max_size.height)];
    }
}

void CocoaWindow_C::SetCursor(WindowCursor_TP cursor) {
    switch (cursor) {
        case WindowCursor_TP::HIDDEN:
            [NSCursor hide];
            break;
        case WindowCursor_TP::DISABLED:
            [NSCursor hide];
            CGAssociateMouseAndMouseCursorPosition(false);
            break;
        case WindowCursor_TP::NORMAL:
        default:
            CGAssociateMouseAndMouseCursorPosition(true);
            [NSCursor unhide];
            break;
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
    if (!_ns_window) { return 1.0F; }
    NSWindow* win = (__bridge NSWindow*)_ns_window;
    NSScreen* screen = win.screen ? win.screen : [NSScreen mainScreen];
    return screen ? static_cast<float>(screen.backingScaleFactor) : 1.0F;
}

}  // namespace vne::xwin
