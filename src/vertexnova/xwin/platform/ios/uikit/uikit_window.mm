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

#include "uikit_window.h"

#include "uikit_window_manager.h"
#include "event_bridge.h"

#import <UIKit/UIKit.h>

// ---------------------------------------------------------------------------
// VneXWinUIView — UIView subclass that routes multi-touch to the bridge
// ---------------------------------------------------------------------------
@interface VneXWinUIView : UIView {
    vne::xwin::UIKitWindow_C* _xwin;
}
- (instancetype)initWithFrame:(CGRect)frame xwin:(vne::xwin::UIKitWindow_C*)xwin;
@end

@implementation VneXWinUIView

- (instancetype)initWithFrame:(CGRect)frame xwin:(vne::xwin::UIKitWindow_C*)xwin {
    self = [super initWithFrame:frame];
    if (self) {
        _xwin = xwin;
        self.multipleTouchEnabled = YES;
        self.userInteractionEnabled = YES;
    }
    return self;
}

- (void)deliverTouches:(NSSet<UITouch*>*)touches phase:(vne::xwin::EventBridgeTouchPhase_C)phase {
    if (!_xwin) { return; }
    for (UITouch* touch in touches) {
        const CGPoint p = [touch locationInView:self];
        // Use the touch object pointer hash as a stable per-finger id
        const uint32_t touch_id = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(touch) & 0xFFFFFFFFu);
        _xwin->handleTouch(touch_id, static_cast<double>(p.x), static_cast<double>(p.y), phase);
    }
}

- (void)touchesBegan:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {
    (void)event;
    [self deliverTouches:touches phase:vne::xwin::EventBridgeTouchPhase_C::eDown];
}
- (void)touchesMoved:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {
    (void)event;
    [self deliverTouches:touches phase:vne::xwin::EventBridgeTouchPhase_C::eMove];
}
- (void)touchesEnded:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {
    (void)event;
    [self deliverTouches:touches phase:vne::xwin::EventBridgeTouchPhase_C::eUp];
}
- (void)touchesCancelled:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {
    (void)event;
    // Treat cancel as up so the app can release any held state
    [self deliverTouches:touches phase:vne::xwin::EventBridgeTouchPhase_C::eUp];
}

@end


// ---------------------------------------------------------------------------
// UIKitWindow_C implementation
// ---------------------------------------------------------------------------
namespace vne::xwin {

UIKitWindow_C::UIKitWindow_C() = default;

UIKitWindow_C::~UIKitWindow_C() {
    destroy_native();
}

void UIKitWindow_C::SetEventOwner(UIKitWindowManager_C* owner) {
    _owner = owner;
}

void UIKitWindow_C::destroy_native() {
    if (_ui_view) {
        UIView* v = (__bridge_transfer UIView*)_ui_view;
        _ui_view = nullptr;
        [v removeFromSuperview];
    }
    _open = false;
}

void UIKitWindow_C::Initialize(const WindowDescriptor_C& descriptor) {
    destroy_native();
    _desc = descriptor;
    CGRect frame = CGRectMake(static_cast<CGFloat>(_desc.position.x),
                              static_cast<CGFloat>(_desc.position.y),
                              static_cast<CGFloat>(_desc.size.width),
                              static_cast<CGFloat>(_desc.size.height));
    VneXWinUIView* v = [[VneXWinUIView alloc] initWithFrame:frame xwin:this];
    v.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    _ui_view = (__bridge_retained void*)v;
    _open = true;
}

void UIKitWindow_C::handleTouch(uint32_t touch_id, double x, double y, EventBridgeTouchPhase_C phase) {
    const EventBridgeCallbacks_C& cb = _owner ? _owner->eventBridgeCallbacks() : _empty_callbacks;
    eventBridgeTouch(this, _desc, cb, touch_id, x, y, phase);
}

void UIKitWindow_C::PollEvents() {
    // Touch events are delivered by UIKit on the main run loop via VneXWinUIView.
}

void UIKitWindow_C::SwapBuffers() {}

void UIKitWindow_C::SetTitle(const std::string& title) {
    _desc.title = title;
}

void UIKitWindow_C::SetWindowMode(WindowMode_TP mode) {
    _desc.mode = mode;
}

WindowMode_TP UIKitWindow_C::GetWindowMode() const {
    return _desc.mode;
}

void UIKitWindow_C::SetFullscreen(bool enabled) {
    // On iOS the app always occupies the full screen; this is a no-op.
    (void)enabled;
}

bool UIKitWindow_C::IsFullscreen() const {
    return true;  // iOS is always full-screen
}

void UIKitWindow_C::Minimize() {
    // iOS does not expose minimizing a view surface like a desktop window.
}

void UIKitWindow_C::Maximize() {}

void UIKitWindow_C::Restore() {}

void UIKitWindow_C::SetWindowLimits(const WindowLimits_C& limits) {
    _desc.limits = limits;
}

void UIKitWindow_C::SetCursor(WindowCursor_TP cursor) {
    (void)cursor;
}

void UIKitWindow_C::SetPosition(int x, int y) {
    _desc.position.x = x;
    _desc.position.y = y;
    if (_ui_view) {
        UIView* v = (__bridge UIView*)_ui_view;
        CGRect f = v.frame;
        f.origin.x = static_cast<CGFloat>(x);
        f.origin.y = static_cast<CGFloat>(y);
        v.frame = f;
    }
}

void UIKitWindow_C::GetPosition(int& x, int& y) const {
    if (_ui_view) {
        UIView* v = (__bridge UIView*)_ui_view;
        x = static_cast<int>(v.frame.origin.x);
        y = static_cast<int>(v.frame.origin.y);
        return;
    }
    x = _desc.position.x;
    y = _desc.position.y;
}

void UIKitWindow_C::Resize(uint32_t width, uint32_t height) {
    _desc.size.width = width;
    _desc.size.height = height;
    if (_ui_view) {
        UIView* v = (__bridge UIView*)_ui_view;
        CGRect f = v.frame;
        f.size.width  = static_cast<CGFloat>(width);
        f.size.height = static_cast<CGFloat>(height);
        v.frame = f;
    }
}

void UIKitWindow_C::Close() {
    destroy_native();
}

bool UIKitWindow_C::IsOpen() const {
    return _open && _ui_view != nullptr;
}

void* UIKitWindow_C::GetNativeWindow() const {
    return _ui_view;
}

WindowAPI_TP UIKitWindow_C::GetWindowAPI() const {
    return WindowAPI_TP::IOS_UIKIT_WINDOW;
}

int UIKitWindow_C::GetWidth() const {
    return static_cast<int>(_desc.size.width);
}

int UIKitWindow_C::GetHeight() const {
    return static_cast<int>(_desc.size.height);
}

float UIKitWindow_C::GetDPIScale() const {
    UIScreen* s = [UIScreen mainScreen];
    return s ? static_cast<float>(s.scale) : 1.0F;
}

}  // namespace vne::xwin
