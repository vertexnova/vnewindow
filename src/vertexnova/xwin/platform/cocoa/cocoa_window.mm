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
#include "event_bridge.h"

#include "vertexnova/xwin/input_mapping.h"
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
@interface VneXWinView : NSView <NSTextInputClient> {
    vne::xwin::CocoaWindow* xwin_;
}
- (instancetype)initWithFrame:(NSRect)frame xwin:(vne::xwin::CocoaWindow*)xwin;
@end

@implementation VneXWinView

- (instancetype)initWithFrame:(NSRect)frame xwin:(vne::xwin::CocoaWindow*)xwin {
    self = [super initWithFrame:frame];
    if (self) {
        xwin_ = xwin;
        // Track mouse movement even when no button is pressed
        [self addTrackingArea:[[NSTrackingArea alloc] initWithRect:NSZeroRect
                                                           options:NSTrackingMouseMoved | NSTrackingActiveInKeyWindow
                                                                   | NSTrackingInVisibleRect
                                                             owner:self
                                                          userInfo:nil]];
    }
    return self;
}

- (BOOL)acceptsFirstResponder {
    return YES;
}
- (BOOL)acceptsFirstMouse:(NSEvent*)event {
    (void)event;
    return YES;
}
- (BOOL)isFlipped {
    return YES;
}  // top-left origin to match other platforms

// ---- Keyboard ----

- (void)keyDown:(NSEvent*)ev {
    if (!xwin_) {
        return;
    }
    const vne::events::KeyCode kc = vne::xwin::mapNativeKeyToEvents(vne::xwin::WindowAPI::eCocoaWindow,
                                                                    vne::xwin::packCocoaNativeKey(ev.keyCode),
                                                                    xwin_->inputMapping());
    const uint8_t mods = vne::xwin::mapNativeModifiersToEvents(vne::xwin::WindowAPI::eCocoaWindow,
                                                               static_cast<uint64_t>(ev.modifierFlags),
                                                               xwin_->inputMapping());
    const bool repeat = ev.isARepeat;
    xwin_->handleKeyDown(kc, mods, repeat);
}

- (void)keyUp:(NSEvent*)ev {
    if (!xwin_) {
        return;
    }
    const vne::events::KeyCode kc = vne::xwin::mapNativeKeyToEvents(vne::xwin::WindowAPI::eCocoaWindow,
                                                                    vne::xwin::packCocoaNativeKey(ev.keyCode),
                                                                    xwin_->inputMapping());
    const uint8_t mods = vne::xwin::mapNativeModifiersToEvents(vne::xwin::WindowAPI::eCocoaWindow,
                                                               static_cast<uint64_t>(ev.modifierFlags),
                                                               xwin_->inputMapping());
    xwin_->handleKeyUp(kc, mods);
}

- (void)flagsChanged:(NSEvent*)ev {
    if (!xwin_) {
        return;
    }
    // Treat modifier-only changes as key press/release based on current flags
    const vne::events::KeyCode kc = vne::xwin::mapNativeKeyToEvents(vne::xwin::WindowAPI::eCocoaWindow,
                                                                    vne::xwin::packCocoaNativeKey(ev.keyCode),
                                                                    xwin_->inputMapping());
    if (kc == vne::events::KeyCode::eUnknown) {
        return;
    }
    const uint8_t mods = vne::xwin::mapNativeModifiersToEvents(vne::xwin::WindowAPI::eCocoaWindow,
                                                               static_cast<uint64_t>(ev.modifierFlags),
                                                               xwin_->inputMapping());
    // Heuristic: if the modifier bit is still set it was just pressed, otherwise released
    const bool is_press = (mods != 0);
    if (is_press) {
        xwin_->handleKeyDown(kc, mods, false);
    } else {
        xwin_->handleKeyUp(kc, 0);
    }
}

// ---- Mouse buttons ----

- (void)mouseDown:(NSEvent*)ev {
    if (!xwin_) {
        return;
    }
    const NSPoint p = [self convertPoint:ev.locationInWindow fromView:nil];
    const auto btn =
        vne::xwin::mapNativeMouseToEvents(vne::xwin::WindowAPI::eCocoaWindow,
                                          vne::xwin::packCocoaNativeMouse(static_cast<uint16_t>(ev.buttonNumber)),
                                          xwin_->inputMapping());
    xwin_->handleMouseButton(btn,
                             true,
                             p.x,
                             p.y,
                             vne::xwin::mapNativeModifiersToEvents(vne::xwin::WindowAPI::eCocoaWindow,
                                                                   static_cast<uint64_t>(ev.modifierFlags),
                                                                   xwin_->inputMapping()));
}
- (void)mouseUp:(NSEvent*)ev {
    if (!xwin_) {
        return;
    }
    const NSPoint p = [self convertPoint:ev.locationInWindow fromView:nil];
    const auto btn =
        vne::xwin::mapNativeMouseToEvents(vne::xwin::WindowAPI::eCocoaWindow,
                                          vne::xwin::packCocoaNativeMouse(static_cast<uint16_t>(ev.buttonNumber)),
                                          xwin_->inputMapping());
    xwin_->handleMouseButton(btn,
                             false,
                             p.x,
                             p.y,
                             vne::xwin::mapNativeModifiersToEvents(vne::xwin::WindowAPI::eCocoaWindow,
                                                                   static_cast<uint64_t>(ev.modifierFlags),
                                                                   xwin_->inputMapping()));
}
- (void)rightMouseDown:(NSEvent*)ev {
    if (!xwin_) {
        return;
    }
    const NSPoint p = [self convertPoint:ev.locationInWindow fromView:nil];
    const auto btn =
        vne::xwin::mapNativeMouseToEvents(vne::xwin::WindowAPI::eCocoaWindow,
                                          vne::xwin::packCocoaNativeMouse(static_cast<uint16_t>(ev.buttonNumber)),
                                          xwin_->inputMapping());
    xwin_->handleMouseButton(btn,
                             true,
                             p.x,
                             p.y,
                             vne::xwin::mapNativeModifiersToEvents(vne::xwin::WindowAPI::eCocoaWindow,
                                                                   static_cast<uint64_t>(ev.modifierFlags),
                                                                   xwin_->inputMapping()));
}
- (void)rightMouseUp:(NSEvent*)ev {
    if (!xwin_) {
        return;
    }
    const NSPoint p = [self convertPoint:ev.locationInWindow fromView:nil];
    const auto btn =
        vne::xwin::mapNativeMouseToEvents(vne::xwin::WindowAPI::eCocoaWindow,
                                          vne::xwin::packCocoaNativeMouse(static_cast<uint16_t>(ev.buttonNumber)),
                                          xwin_->inputMapping());
    xwin_->handleMouseButton(btn,
                             false,
                             p.x,
                             p.y,
                             vne::xwin::mapNativeModifiersToEvents(vne::xwin::WindowAPI::eCocoaWindow,
                                                                   static_cast<uint64_t>(ev.modifierFlags),
                                                                   xwin_->inputMapping()));
}
- (void)otherMouseDown:(NSEvent*)ev {
    if (!xwin_) {
        return;
    }
    const NSPoint p = [self convertPoint:ev.locationInWindow fromView:nil];
    const auto btn =
        vne::xwin::mapNativeMouseToEvents(vne::xwin::WindowAPI::eCocoaWindow,
                                          vne::xwin::packCocoaNativeMouse(static_cast<uint16_t>(ev.buttonNumber)),
                                          xwin_->inputMapping());
    xwin_->handleMouseButton(btn,
                             true,
                             p.x,
                             p.y,
                             vne::xwin::mapNativeModifiersToEvents(vne::xwin::WindowAPI::eCocoaWindow,
                                                                   static_cast<uint64_t>(ev.modifierFlags),
                                                                   xwin_->inputMapping()));
}
- (void)otherMouseUp:(NSEvent*)ev {
    if (!xwin_) {
        return;
    }
    const NSPoint p = [self convertPoint:ev.locationInWindow fromView:nil];
    const auto btn =
        vne::xwin::mapNativeMouseToEvents(vne::xwin::WindowAPI::eCocoaWindow,
                                          vne::xwin::packCocoaNativeMouse(static_cast<uint16_t>(ev.buttonNumber)),
                                          xwin_->inputMapping());
    xwin_->handleMouseButton(btn,
                             false,
                             p.x,
                             p.y,
                             vne::xwin::mapNativeModifiersToEvents(vne::xwin::WindowAPI::eCocoaWindow,
                                                                   static_cast<uint64_t>(ev.modifierFlags),
                                                                   xwin_->inputMapping()));
}

// ---- Mouse motion ----

- (void)mouseMoved:(NSEvent*)ev {
    if (!xwin_) {
        return;
    }
    const NSPoint p = [self convertPoint:ev.locationInWindow fromView:nil];
    xwin_->handleMouseMove(p.x,
                           p.y,
                           vne::xwin::mapNativeModifiersToEvents(vne::xwin::WindowAPI::eCocoaWindow,
                                                                 static_cast<uint64_t>(ev.modifierFlags),
                                                                 xwin_->inputMapping()));
}
- (void)mouseDragged:(NSEvent*)ev {
    [self mouseMoved:ev];
}
- (void)rightMouseDragged:(NSEvent*)ev {
    [self mouseMoved:ev];
}
- (void)otherMouseDragged:(NSEvent*)ev {
    [self mouseMoved:ev];
}

// ---- Scroll wheel ----

- (void)scrollWheel:(NSEvent*)ev {
    if (!xwin_) {
        return;
    }
    xwin_->handleMouseScroll(static_cast<float>(ev.scrollingDeltaX), static_cast<float>(ev.scrollingDeltaY));
}

// ---- Text / IME input ----

- (BOOL)hasMarkedText {
    return NO;
}
- (NSRange)markedRange {
    return NSMakeRange(NSNotFound, 0);
}
- (NSRange)selectedRange {
    return NSMakeRange(NSNotFound, 0);
}
- (void)setMarkedText:(id)string selectedRange:(NSRange)sel replacementRange:(NSRange)rep {
    (void)string;
    (void)sel;
    (void)rep;
}
- (void)unmarkText {
}
- (NSArray<NSAttributedStringKey>*)validAttributesForMarkedText {
    return @[];
}
- (NSAttributedString*)attributedSubstringForProposedRange:(NSRange)r actualRange:(NSRangePointer)ar {
    (void)r;
    (void)ar;
    return nil;
}
- (NSUInteger)characterIndexForPoint:(NSPoint)p {
    (void)p;
    return NSNotFound;
}
- (NSRect)firstRectForCharacterRange:(NSRange)r actualRange:(NSRangePointer)ar {
    (void)r;
    (void)ar;
    return NSZeroRect;
}
- (void)insertText:(id)string replacementRange:(NSRange)rep {
    (void)rep;
    if (!xwin_) {
        return;
    }
    NSString* str = [string isKindOfClass:[NSAttributedString class]]
                        ? [static_cast<NSAttributedString*>(string) string]
                        : static_cast<NSString*>(string);
    if (!str || str.length == 0) {
        return;
    }
    const char* utf8 = str.UTF8String;
    if (utf8) {
        xwin_->handleTextInput(utf8);
    }
}
- (void)doCommandBySelector:(SEL)sel {
    (void)sel;
}

@end

// ---------------------------------------------------------------------------
// VneXWinWindowDelegate — NSWindowDelegate for window-level events
// ---------------------------------------------------------------------------
@interface VneXWinWindowDelegate : NSObject <NSWindowDelegate> {
    vne::xwin::CocoaWindow* xwin_;
}
- (instancetype)initWithXwin:(vne::xwin::CocoaWindow*)xwin;
@end

@implementation VneXWinWindowDelegate

- (instancetype)initWithXwin:(vne::xwin::CocoaWindow*)xwin {
    self = [super init];
    if (self) {
        xwin_ = xwin;
    }
    return self;
}

- (BOOL)windowShouldClose:(NSWindow*)sender {
    (void)sender;
    if (xwin_) {
        xwin_->handleWindowClose();
    }
    return NO;  // CocoaWindow::close() calls destroyNative()
}

- (void)windowDidResize:(NSNotification*)notification {
    if (!xwin_) {
        return;
    }
    NSWindow* win = notification.object;
    const NSRect r = [win.contentView frame];
    xwin_->handleWindowResize(static_cast<uint32_t>(r.size.width), static_cast<uint32_t>(r.size.height));
}

- (void)windowDidBecomeKey:(NSNotification*)notification {
    (void)notification;
    if (xwin_) {
        xwin_->handleWindowFocus(true);
    }
}

- (void)windowDidResignKey:(NSNotification*)notification {
    (void)notification;
    if (xwin_) {
        xwin_->handleWindowFocus(false);
    }
}

- (void)windowDidEnterFullScreen:(NSNotification*)notification {
    (void)notification;
    if (xwin_) {
        xwin_->setFullscreenState(true);
    }
}

- (void)windowDidExitFullScreen:(NSNotification*)notification {
    (void)notification;
    if (xwin_) {
        xwin_->setFullscreenState(false);
    }
}

@end

// ---------------------------------------------------------------------------
// CocoaWindow implementation
// ---------------------------------------------------------------------------
namespace vne::xwin {

CocoaWindow::CocoaWindow() = default;

CocoaWindow::~CocoaWindow() {
    destroyNative();
}

void CocoaWindow::setEventOwner(CocoaWindowManager* owner) {
    owner_ = owner;
}

void CocoaWindow::destroyNative() {
    if (ns_window_) {
        NSWindow* win = (__bridge_transfer NSWindow*)ns_window_;
        ns_window_ = nullptr;
        ns_view_ = nullptr;
        ns_delegate_ = nullptr;
        [win setDelegate:nil];
        [win close];
    }
    open_ = false;
}

void CocoaWindow::initialize(const WindowDescriptor& descriptor) {
    destroyNative();
    desc_ = descriptor;

    NSRect rect = NSMakeRect(desc_.position.x,
                             desc_.position.y,
                             static_cast<CGFloat>(desc_.size.width),
                             static_cast<CGFloat>(desc_.size.height));
    NSUInteger style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable
                       | NSWindowStyleMaskResizable;
    if (!desc_.decorated) {
        style = NSWindowStyleMaskBorderless;
    }

    NSWindow* win = [[NSWindow alloc] initWithContentRect:rect styleMask:style backing:NSBackingStoreBuffered defer:NO];
    [win setTitle:[NSString stringWithUTF8String:desc_.title.c_str()]];
    [win setCollectionBehavior:[win collectionBehavior] | NSWindowCollectionBehaviorFullScreenPrimary];

    // Custom content view
    VneXWinView* view = [[VneXWinView alloc] initWithFrame:rect xwin:this];
    [win setContentView:view];
    [win makeFirstResponder:view];

    // Window delegate for close/resize/focus
    VneXWinWindowDelegate* delegate = [[VneXWinWindowDelegate alloc] initWithXwin:this];
    [win setDelegate:delegate];

    ns_window_ = (__bridge_retained void*)win;
    ns_view_ = (__bridge void*)view;
    ns_delegate_ = (__bridge_retained void*)delegate;

    if (desc_.visible) {
        [win orderFrontRegardless];
        [NSApp activateIgnoringOtherApps:YES];
    }

    // Apply size limits from descriptor
    if (desc_.limits.has_min_size) {
        [win setMinSize:NSMakeSize(desc_.limits.min_size.width, desc_.limits.min_size.height)];
    }
    if (desc_.limits.has_max_size) {
        [win setMaxSize:NSMakeSize(desc_.limits.max_size.width, desc_.limits.max_size.height)];
    }

    open_ = true;
}

void CocoaWindow::pollEvents() {
    for (;;) {
        NSEvent* ev = [NSApp nextEventMatchingMask:NSEventMaskAny
                                         untilDate:[NSDate distantPast]
                                            inMode:NSDefaultRunLoopMode
                                           dequeue:YES];
        if (!ev) {
            break;
        }
        [NSApp sendEvent:ev];
    }
}

void CocoaWindow::swapBuffers() {}

// ---- Event dispatch helpers (called from ObjC) ----

void CocoaWindow::handleKeyDown(vne::events::KeyCode key, uint8_t mods, bool repeat) {
    if (key == vne::events::KeyCode::eUnknown) {
        return;
    }
    const EventBridgeCallbacks& cb = owner_ ? owner_->eventBridgeCallbacks() : empty_callbacks_;
    eventBridgeKeyDown(this, desc_, cb, key, mods, repeat);
}

void CocoaWindow::handleKeyUp(vne::events::KeyCode key, uint8_t mods) {
    if (key == vne::events::KeyCode::eUnknown) {
        return;
    }
    const EventBridgeCallbacks& cb = owner_ ? owner_->eventBridgeCallbacks() : empty_callbacks_;
    eventBridgeKeyUp(this, desc_, cb, key, mods);
}

void CocoaWindow::handleMouseButton(vne::events::MouseButton button, bool pressed, double x, double y, uint8_t mods) {
    const EventBridgeCallbacks& cb = owner_ ? owner_->eventBridgeCallbacks() : empty_callbacks_;
    eventBridgeMouseButton(this, desc_, cb, button, pressed, x, y, mods);
}

void CocoaWindow::handleMouseMove(double x, double y, uint8_t mods) {
    const EventBridgeCallbacks& cb = owner_ ? owner_->eventBridgeCallbacks() : empty_callbacks_;
    eventBridgeMouseMove(this, desc_, cb, x, y, mods);
}

void CocoaWindow::handleMouseScroll(float dx, float dy) {
    const EventBridgeCallbacks& cb = owner_ ? owner_->eventBridgeCallbacks() : empty_callbacks_;
    eventBridgeMouseScroll(this, desc_, cb, dx, dy);
}

void CocoaWindow::handleWindowClose() {
    const EventBridgeCallbacks& cb = owner_ ? owner_->eventBridgeCallbacks() : empty_callbacks_;
    eventBridgeWindowClose(this, desc_, cb);
    open_ = false;
    if (owner_) {
        WindowEventData data{};
        data.type = WindowEventType::eClose;
        owner_->notifyWindowEvent(this, data);
    }
    destroyNative();
}

void CocoaWindow::handleWindowResize(uint32_t w, uint32_t h) {
    desc_.size.width = w;
    desc_.size.height = h;
    const EventBridgeCallbacks& cb = owner_ ? owner_->eventBridgeCallbacks() : empty_callbacks_;
    eventBridgeWindowResize(this, desc_, cb, w, h);
    if (owner_) {
        WindowEventData data{};
        data.type = WindowEventType::eResize;
        data.size = desc_.size;
        owner_->notifyWindowEvent(this, data);
    }
}

void CocoaWindow::handleWindowFocus(bool focused) {
    const EventBridgeCallbacks& cb = owner_ ? owner_->eventBridgeCallbacks() : empty_callbacks_;
    eventBridgeWindowFocus(this, desc_, cb, focused);
    if (owner_) {
        WindowEventData data{};
        data.type = WindowEventType::eFocus;
        data.focused = focused;
        owner_->notifyWindowEvent(this, data);
    }
}

void CocoaWindow::setFullscreenState(bool fs) {
    fullscreen_ = fs;
}

// ---- IWindow interface ----

void CocoaWindow::setTitle(const std::string& title) {
    desc_.title = title;
    if (ns_window_) {
        NSWindow* win = (__bridge NSWindow*)ns_window_;
        [win setTitle:[NSString stringWithUTF8String:title.c_str()]];
    }
}

void CocoaWindow::setWindowMode(WindowMode mode) {
    desc_.mode = mode;
    if (!ns_window_) {
        return;
    }
    NSWindow* win = (__bridge NSWindow*)ns_window_;
    if (mode == WindowMode::eFullscreen) {
        setFullscreen(true);
        return;
    }
    if (fullscreen_) {
        setFullscreen(false);
    }
    if (mode == WindowMode::eBorderless) {
        [win setStyleMask:NSWindowStyleMaskBorderless];
        NSScreen* screen = win.screen ? win.screen : [NSScreen mainScreen];
        if (screen) {
            [win setFrame:screen.frame display:YES];
        }
        return;
    }
    [win setStyleMask:(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable
                       | NSWindowStyleMaskResizable)];
}

WindowMode CocoaWindow::getWindowMode() const noexcept {
    return desc_.mode;
}

void CocoaWindow::setFullscreen(bool enabled) {
    if (!ns_window_) {
        return;
    }
    if (enabled == fullscreen_) {
        return;
    }
    NSWindow* win = (__bridge NSWindow*)ns_window_;
    [win toggleFullScreen:nil];
    // fullscreen_ updated via windowDidEnterFullScreen: / windowDidExitFullScreen:
}

bool CocoaWindow::isFullscreen() const noexcept {
    return fullscreen_;
}

void CocoaWindow::minimize() {
    if (ns_window_) {
        [(__bridge NSWindow*)ns_window_ miniaturize:nil];
    }
}

void CocoaWindow::maximize() {
    if (ns_window_) {
        NSWindow* win = (__bridge NSWindow*)ns_window_;
        if (!win.isZoomed) {
            [win zoom:nil];
        }
    }
}

void CocoaWindow::restore() {
    if (ns_window_) {
        NSWindow* win = (__bridge NSWindow*)ns_window_;
        if (win.isMiniaturized) {
            [win deminiaturize:nil];
        } else if (win.isZoomed) {
            [win zoom:nil];
        }
    }
}

void CocoaWindow::setPosition(int x, int y) {
    desc_.position.x = x;
    desc_.position.y = y;
    if (ns_window_) {
        [(__bridge NSWindow*)ns_window_
            setFrameTopLeftPoint:NSMakePoint(static_cast<CGFloat>(x), static_cast<CGFloat>(y))];
    }
}

void CocoaWindow::getPosition(int& x, int& y) const {
    if (ns_window_) {
        NSRect r = ((__bridge NSWindow*)ns_window_).frame;
        x = static_cast<int>(r.origin.x);
        y = static_cast<int>(r.origin.y);
        return;
    }
    x = desc_.position.x;
    y = desc_.position.y;
}

void CocoaWindow::resize(uint32_t width, uint32_t height) {
    desc_.size.width = width;
    desc_.size.height = height;
    if (ns_window_) {
        NSWindow* win = (__bridge NSWindow*)ns_window_;
        NSRect f = win.frame;
        f.size.width = static_cast<CGFloat>(width);
        f.size.height = static_cast<CGFloat>(height);
        [win setFrame:f display:YES];
    }
}

void CocoaWindow::setWindowLimits(const WindowLimits& limits) {
    desc_.limits = limits;
    if (!ns_window_) {
        return;
    }
    NSWindow* win = (__bridge NSWindow*)ns_window_;
    if (limits.has_min_size) {
        [win setMinSize:NSMakeSize(limits.min_size.width, limits.min_size.height)];
    }
    if (limits.has_max_size) {
        [win setMaxSize:NSMakeSize(limits.max_size.width, limits.max_size.height)];
    }
}

void CocoaWindow::setCursor(WindowCursor cursor) {
    switch (cursor) {
        case WindowCursor::eHidden:
            [NSCursor hide];
            break;
        case WindowCursor::eDisabled:
            [NSCursor hide];
            CGAssociateMouseAndMouseCursorPosition(false);
            break;
        case WindowCursor::eNormal:
        default:
            CGAssociateMouseAndMouseCursorPosition(true);
            [NSCursor unhide];
            break;
    }
}

void CocoaWindow::close() {
    destroyNative();
}

bool CocoaWindow::isOpen() const noexcept {
    return open_ && ns_window_ != nullptr;
}


NativeWindowHandle CocoaWindow::getNativeHandle() const noexcept {
    NativeWindowHandle handle{};
    handle.api = WindowAPI::eCocoaWindow;
    handle.ns_view = ns_view_;
    handle.ns_window = ns_window_;
    return handle;
}

WindowAPI CocoaWindow::getWindowAPI() const noexcept {
    return WindowAPI::eCocoaWindow;
}

int CocoaWindow::getWidth() const noexcept {
    return static_cast<int>(desc_.size.width);
}

int CocoaWindow::getHeight() const noexcept {
    return static_cast<int>(desc_.size.height);
}

float CocoaWindow::getDpiScale() const noexcept {
    if (!ns_window_) {
        return 1.0F;
    }
    NSWindow* win = (__bridge NSWindow*)ns_window_;
    NSScreen* screen = win.screen ? win.screen : [NSScreen mainScreen];
    return screen ? static_cast<float>(screen.backingScaleFactor) : 1.0F;
}

void CocoaWindow::handleTextInput(const char* utf8_text) {
    const EventBridgeCallbacks& cb = owner_ ? owner_->eventBridgeCallbacks() : empty_callbacks_;
    eventBridgeTextInput(this, desc_, cb, utf8_text);
}

std::string CocoaWindow::getClipboardText() const {
    NSPasteboard* pb = [NSPasteboard generalPasteboard];
    NSString* str = [pb stringForType:NSPasteboardTypeString];
    if (!str) {
        return {};
    }
    const char* utf8 = str.UTF8String;
    return utf8 ? std::string(utf8) : std::string{};
}

void CocoaWindow::setClipboardText(const std::string& text) {
    NSPasteboard* pb = [NSPasteboard generalPasteboard];
    [pb clearContents];
    [pb setString:[NSString stringWithUTF8String:text.c_str()] forType:NSPasteboardTypeString];
}

}  // namespace vne::xwin
