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

#include "uikit_main_sync.h"
#include "uikit_window_manager.h"
#include "event_bridge.h"

#import <UIKit/UIKit.h>
#import <QuartzCore/CAMetalLayer.h>

// ---------------------------------------------------------------------------
// VneXWinUIView — UIView subclass that routes multi-touch to the bridge
// ---------------------------------------------------------------------------
@interface VneXWinUIView : UIView {
    vne::xwin::UIKitWindow* xwin_;
}
- (instancetype)initWithFrame:(CGRect)frame xwin:(vne::xwin::UIKitWindow*)xwin;
- (void)clearXwin;
@end

@implementation VneXWinUIView

+ (Class)layerClass {
    return [CAMetalLayer class];
}

- (instancetype)initWithFrame:(CGRect)frame xwin:(vne::xwin::UIKitWindow*)xwin {
    self = [super initWithFrame:frame];
    if (self) {
        xwin_ = xwin;
        self.multipleTouchEnabled = YES;
        self.userInteractionEnabled = YES;
    }
    return self;
}

- (void)clearXwin {
    xwin_ = nullptr;
}

- (void)deliverTouches:(NSSet<UITouch*>*)touches phase:(vne::xwin::EventBridgeTouchPhase)phase {
    if (!xwin_) {
        return;
    }
    for (UITouch* touch in touches) {
        const CGPoint p = [touch locationInView:self];
        // Use the touch object pointer hash as a stable per-finger id
        const uint32_t touch_id = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(touch) & 0xFFFFFFFFu);
        xwin_->handleTouch(touch_id, static_cast<double>(p.x), static_cast<double>(p.y), phase);
    }
}

- (void)touchesBegan:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {
    (void)event;
    [self deliverTouches:touches phase:vne::xwin::EventBridgeTouchPhase::eDown];
}
- (void)touchesMoved:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {
    (void)event;
    [self deliverTouches:touches phase:vne::xwin::EventBridgeTouchPhase::eMove];
}
- (void)touchesEnded:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {
    (void)event;
    [self deliverTouches:touches phase:vne::xwin::EventBridgeTouchPhase::eUp];
}
- (void)touchesCancelled:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {
    (void)event;
    // Treat cancel as up so the app can release any held state
    [self deliverTouches:touches phase:vne::xwin::EventBridgeTouchPhase::eUp];
}

@end

// ---------------------------------------------------------------------------
// UIKitWindow implementation
// ---------------------------------------------------------------------------
namespace vne::xwin {

UIKitWindow::UIKitWindow() = default;

UIKitWindow::~UIKitWindow() {
    uikitRunOnMainSync(^{
      destroyNative();
    });
}

void UIKitWindow::setEventOwner(UIKitWindowManager* owner) {
    owner_ = owner;
}

void UIKitWindow::destroyNative() {
    if (ui_view_) {
        UIView* v = (__bridge_transfer UIView*)ui_view_;
        ui_view_ = nullptr;
        if ([v isKindOfClass:[VneXWinUIView class]]) {
            [static_cast<VneXWinUIView*>(v) clearXwin];
        }
        [v removeFromSuperview];
    }
    open_ = false;
}

void UIKitWindow::initialize(const WindowDescriptor& descriptor) {
    uikitRunOnMainSync(^{
      destroyNative();
      desc_ = descriptor;
      CGRect frame = CGRectMake(static_cast<CGFloat>(desc_.position.x),
                                static_cast<CGFloat>(desc_.position.y),
                                static_cast<CGFloat>(desc_.size.width),
                                static_cast<CGFloat>(desc_.size.height));
      VneXWinUIView* v = [[VneXWinUIView alloc] initWithFrame:frame xwin:this];
      v.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
      ui_view_ = (__bridge_retained void*)v;
      open_ = true;
    });
}

void UIKitWindow::handleTouch(uint32_t touch_id, double x, double y, EventBridgeTouchPhase phase) {
    const EventBridgeCallbacks& cb = owner_ ? owner_->eventBridgeCallbacks() : empty_callbacks_;
    eventBridgeTouch(this, desc_, cb, touch_id, x, y, phase);
}

void UIKitWindow::pollEvents() {
    // Touch events are delivered by UIKit on the main run loop via VneXWinUIView.
}

void UIKitWindow::swapBuffers() {}

void UIKitWindow::setTitle(const std::string& title) {
    desc_.title = title;
}

void UIKitWindow::setWindowMode(WindowMode mode) {
    desc_.mode = mode;
}

WindowMode UIKitWindow::getWindowMode() const noexcept {
    return desc_.mode;
}

void UIKitWindow::setFullscreen(bool enabled) {
    // On iOS the app always occupies the full screen; this is a no-op.
    (void)enabled;
}

bool UIKitWindow::isFullscreen() const noexcept {
    return true;  // iOS is always full-screen
}

void UIKitWindow::minimize() {
    // iOS does not expose minimizing a view surface like a desktop window.
}

void UIKitWindow::maximize() {}

void UIKitWindow::restore() {}

void UIKitWindow::setWindowLimits(const WindowLimits& limits) {
    desc_.limits = limits;
}

void UIKitWindow::setCursor(WindowCursor cursor) {
    (void)cursor;
}

void UIKitWindow::setPosition(int x, int y) {
    uikitRunOnMainSync(^{
      desc_.position.x = x;
      desc_.position.y = y;
      if (ui_view_) {
          UIView* v = (__bridge UIView*)ui_view_;
          CGRect f = v.frame;
          f.origin.x = static_cast<CGFloat>(x);
          f.origin.y = static_cast<CGFloat>(y);
          v.frame = f;
      }
    });
}

WindowPosition UIKitWindow::getPosition() const {
    if (!ui_view_) {
        return desc_.position;
    }
    __block WindowPosition pos{};
    UIKitWindow* self = const_cast<UIKitWindow*>(this);
    uikitRunOnMainSync(^{
      UIView* v = (__bridge UIView*)self->ui_view_;
      pos = WindowPosition{static_cast<int32_t>(v.frame.origin.x), static_cast<int32_t>(v.frame.origin.y)};
    });
    return pos;
}

void UIKitWindow::resize(uint32_t width, uint32_t height) {
    uikitRunOnMainSync(^{
      desc_.size.width = width;
      desc_.size.height = height;
      if (ui_view_) {
          UIView* v = (__bridge UIView*)ui_view_;
          CGRect f = v.frame;
          f.size.width = static_cast<CGFloat>(width);
          f.size.height = static_cast<CGFloat>(height);
          v.frame = f;
      }
    });
}

void UIKitWindow::close() {
    uikitRunOnMainSync(^{
      destroyNative();
    });
}

bool UIKitWindow::isOpen() const noexcept {
    return open_ && ui_view_ != nullptr;
}

NativeWindowHandle UIKitWindow::getNativeHandle() const noexcept {
    NativeWindowHandle handle{};
    handle.api = WindowAPI::eIosUikitWindow;
    handle.ui_view = ui_view_;
    if (ui_view_) {
        UIView* view = (__bridge UIView*)ui_view_;
        handle.ca_layer = (__bridge void*)view.layer;
    }
    return handle;
}

WindowAPI UIKitWindow::getWindowAPI() const noexcept {
    return WindowAPI::eIosUikitWindow;
}

int UIKitWindow::getWidth() const noexcept {
    return static_cast<int>(desc_.size.width);
}

int UIKitWindow::getHeight() const noexcept {
    return static_cast<int>(desc_.size.height);
}

float UIKitWindow::getDpiScale() const noexcept {
#if defined(VNE_PLATFORM_VISIONOS)
    if (ui_view_) {
        UIView* v = (__bridge UIView*)ui_view_;
        const CGFloat scale = v.traitCollection.displayScale;
        if (scale > 0.0) {
            return static_cast<float>(scale);
        }
    }
    return 2.0F;
#else
    UIScreen* s = [UIScreen mainScreen];
    return s ? static_cast<float>(s.scale) : 1.0F;
#endif
}

}  // namespace vne::xwin
