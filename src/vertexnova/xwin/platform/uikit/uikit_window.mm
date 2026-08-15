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
#include "platform/cocoa/cocoa_map_key.h"

#import <UIKit/UIKit.h>
#import <QuartzCore/CAMetalLayer.h>

namespace {

// Points-to-scroll-lines factor for continuous (trackpad) scrolling.
constexpr double kContinuousScrollToLines = 0.1;

double discreteScrollUnit(const CGFloat v) {
    if (v == 0.0) {
        return 0.0;
    }
    return (v > 0.0) ? 1.0 : -1.0;
}

double continuousScrollUnit(const CGFloat v) {
    return static_cast<double>(v) * kContinuousScrollToLines;
}

// UIKeyModifierFlags uses the same shift/ctrl/alt/cmd bits as NSEventModifierFlags.
uint8_t modifiersFromEvent(UIEvent* event) {
    if (event == nil) {
        return 0;
    }
    if (@available(iOS 13.4, *)) {
        return vne::xwin::mapCocoaModifiers(static_cast<unsigned long>(event.modifierFlags));
    }
    return 0;
}

CGRect vneXWinSceneBounds(UIWindowScene* window_scene) {
    if (window_scene == nil) {
        return CGRectZero;
    }
#if defined(VNE_PLATFORM_VISIONOS)
    return window_scene.coordinateSpace.bounds;
#else
    return window_scene.screen.bounds;
#endif
}

UIWindowScene* vneXWinFindWindowScene(UIWindow* window, void* platform_data) {
    if (@available(iOS 13.0, *)) {
        if (platform_data != nullptr) {
            id scene = (__bridge id)platform_data;
            if ([scene isKindOfClass:[UIWindowScene class]]) {
                return static_cast<UIWindowScene*>(scene);
            }
        }
        if (window.windowScene != nil) {
            return window.windowScene;
        }
        for (UIScene* scene in UIApplication.sharedApplication.connectedScenes) {
            if (![scene isKindOfClass:[UIWindowScene class]]) {
                continue;
            }
            if (scene.activationState == UISceneActivationStateForegroundActive
                || scene.activationState == UISceneActivationStateForegroundInactive) {
                return static_cast<UIWindowScene*>(scene);
            }
        }
    }
    return nil;
}

}  // namespace

// ---------------------------------------------------------------------------
// VneXWinUIView — UIView subclass that routes multi-touch / pointer / scroll
//
// Pointer input (hover, secondary/middle button, scroll wheel) only exists when the
// OS delivers indirect pointer events. Info.plist must set
// UIApplicationSupportsIndirectInputEvents, which every sample bundle does.
//
// In the iOS Simulator that is not sufficient: the host mouse is delivered as a plain
// direct touch (UITouchTypeDirect, buttonMask 0) and the wheel produces no event at
// all until "I/O > Input > Send Pointer to Device" is enabled. That toggle is host
// side, session scoped, has no persisted preference, and no guest API can request it,
// so wheel zoom in the Simulator is opt-in by the developer running it. Pinch (which
// the Simulator synthesizes from Option+drag as two real touches) always works and is
// the fallback path.
// ---------------------------------------------------------------------------
@interface VneXWinUIView : UIView <UIGestureRecognizerDelegate> {
    vne::xwin::UIKitWindow* xwin_;
    // Active indirect-pointer (mouse) button, so the matching release is emitted on lift.
    vne::events::MouseButton pointer_button_;
    BOOL pointer_down_;
    UIPanGestureRecognizer* discrete_scroll_recognizer_;
    UIPanGestureRecognizer* continuous_scroll_recognizer_;
    id app_active_observer_;
}
- (instancetype)initWithFrame:(CGRect)frame xwin:(vne::xwin::UIKitWindow*)xwin;
- (void)clearXwin;
- (void)activateInputRouting;
- (void)emitScroll:(UIPanGestureRecognizer*)recognizer discrete:(BOOL)discrete;
- (void)onDiscreteScroll:(UIPanGestureRecognizer*)recognizer;
- (void)onContinuousScroll:(UIPanGestureRecognizer*)recognizer;
#if !defined(VNE_PLATFORM_VISIONOS)
- (void)onHover:(UIHoverGestureRecognizer*)recognizer;
#endif
@end

// Reasserts first-responder status on the Metal view when the window becomes key.
@interface VneXWinUIWindow : UIWindow
@end

@interface VneXWinRootViewController : UIViewController
@end

@implementation VneXWinUIView

+ (Class)layerClass {
    return [CAMetalLayer class];
}

- (instancetype)initWithFrame:(CGRect)frame xwin:(vne::xwin::UIKitWindow*)xwin {
    self = [super initWithFrame:frame];
    if (self) {
        xwin_ = xwin;
        pointer_button_ = vne::events::MouseButton::eLeft;
        pointer_down_ = NO;
        discrete_scroll_recognizer_ = nil;
        continuous_scroll_recognizer_ = nil;
        app_active_observer_ = nil;
        self.multipleTouchEnabled = YES;
        self.userInteractionEnabled = YES;

        // Scroll wheel / trackpad scroll -> zoom.
        // UIPanGestureRecognizer has no scrollType, so discrete and continuous each
        // get their own recognizer and mapping. An empty allowedTouchTypes is the
        // documented way to make a pan recognizer scroll-only, so it never steals
        // finger pans. Do not also clamp maximumNumberOfTouches: minimumNumberOfTouches
        // defaults to 1, and a maximum below that leaves the recognizer unable to begin.
        if (@available(iOS 13.4, *)) {
            auto make_scroll_pan = ^(SEL action, UIScrollTypeMask mask) {
                UIPanGestureRecognizer* pan = [[UIPanGestureRecognizer alloc] initWithTarget:self action:action];
                pan.allowedScrollTypesMask = mask;
                pan.allowedTouchTypes = @[];
                pan.cancelsTouchesInView = NO;
                pan.delaysTouchesBegan = NO;
                pan.delaysTouchesEnded = NO;
                pan.delegate = self;
                [self addGestureRecognizer:pan];
                return pan;
            };
            discrete_scroll_recognizer_ = make_scroll_pan(@selector(onDiscreteScroll:), UIScrollTypeMaskDiscrete);
            continuous_scroll_recognizer_ = make_scroll_pan(@selector(onContinuousScroll:), UIScrollTypeMaskContinuous);

#if !defined(VNE_PLATFORM_VISIONOS)
            UIHoverGestureRecognizer* hover = [[UIHoverGestureRecognizer alloc] initWithTarget:self
                                                                                        action:@selector(onHover:)];
            [self addGestureRecognizer:hover];
            [self addInteraction:[[UIPointerInteraction alloc] initWithDelegate:nil]];
#endif
        }

        // No pinch recognizer here on purpose. The simulator synthesizes Option+drag as
        // two real touches, which already reach TouchToMouseConverter in the sample
        // framework and become a scroll there. A recognizer would double the zoom.

        __weak VneXWinUIView* weak_self = self;
        app_active_observer_ =
            [NSNotificationCenter.defaultCenter addObserverForName:UIApplicationDidBecomeActiveNotification
                                                            object:nil
                                                             queue:NSOperationQueue.mainQueue
                                                        usingBlock:^(NSNotification* note) {
                                                          (void)note;
                                                          [weak_self activateInputRouting];
                                                        }];
    }
    return self;
}

- (BOOL)canBecomeFirstResponder {
    return YES;
}

- (void)didMoveToWindow {
    [super didMoveToWindow];
    if (self.window != nil) {
        [self activateInputRouting];
    }
}

- (void)activateInputRouting {
    [self becomeFirstResponder];
}

- (BOOL)gestureRecognizer:(UIGestureRecognizer*)a
    shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer*)b {
    (void)a;
    (void)b;
    return YES;
}

#if !defined(VNE_PLATFORM_VISIONOS)
- (void)onHover:(UIHoverGestureRecognizer*)recognizer {
    if (xwin_ == nullptr) {
        return;
    }
    const CGPoint p = [recognizer locationInView:self];
    xwin_->handleMouseMove(static_cast<double>(p.x), static_cast<double>(p.y), 0);
}
#endif

- (void)clearXwin {
    xwin_ = nullptr;
    NSNotificationCenter* center = NSNotificationCenter.defaultCenter;
    if (app_active_observer_ != nil) {
        [center removeObserver:app_active_observer_];
        app_active_observer_ = nil;
    }
}

// UIEventButtonMaskForButtonNumber(n) expands to (1 << (n - 1)), so button 2 is the
// same bit as UIEventButtonMaskSecondary. Middle is button 3. Test the higher button
// first so a chorded press reports middle instead of falling through to right.
- (vne::events::MouseButton)buttonFromEvent:(UIEvent*)event {
    vne::events::MouseButton button = vne::events::MouseButton::eLeft;
    if (@available(iOS 13.4, *)) {
        if (event != nil) {
            const UIEventButtonMask mask = event.buttonMask;
            if ((mask & UIEventButtonMaskForButtonNumber(3)) != 0) {
                button = vne::events::MouseButton::eMiddle;
            } else if ((mask & UIEventButtonMaskSecondary) != 0) {
                button = vne::events::MouseButton::eRight;
            }
        }
    }
    return button;
}

- (void)deliverPointer:(CGPoint)p phase:(vne::xwin::EventBridgeTouchPhase)phase event:(UIEvent*)event {
    const double x = static_cast<double>(p.x);
    const double y = static_cast<double>(p.y);
    const vne::events::MouseButton button = [self buttonFromEvent:event];
    const uint8_t modifiers = modifiersFromEvent(event);

    switch (phase) {
        case vne::xwin::EventBridgeTouchPhase::eDown:
            pointer_button_ = button;
            pointer_down_ = YES;
            xwin_->handleMouseMove(x, y, modifiers);
            xwin_->handleMouseButton(button, true, x, y, modifiers);
            break;
        case vne::xwin::EventBridgeTouchPhase::eMove:
            xwin_->handleMouseMove(x, y, modifiers);
            break;
        case vne::xwin::EventBridgeTouchPhase::eUp:
            xwin_->handleMouseMove(x, y, modifiers);
            if (pointer_down_) {
                xwin_->handleMouseButton(pointer_button_, false, x, y, modifiers);
                pointer_down_ = NO;
            }
            break;
    }
}

- (void)deliverTouches:(NSSet<UITouch*>*)touches phase:(vne::xwin::EventBridgeTouchPhase)phase event:(UIEvent*)event {
    if (!xwin_) {
        return;
    }
    NSUInteger button_mask = 0;
    if (@available(iOS 13.4, *)) {
        if (event != nil) {
            button_mask = static_cast<NSUInteger>(event.buttonMask);
        }
    }
    for (UITouch* touch in touches) {
        const CGPoint p = [touch locationInView:self];
        // Mouse / trackpad: emit real mouse events (left/right/middle) so CameraLayer
        // does not need TouchToMouseConverter. Real fingers stay as touch events.
        if (@available(iOS 13.4, *)) {
            if (touch.type == UITouchTypeIndirectPointer) {
                [self deliverPointer:p phase:phase event:event];
                continue;
            }
            // iPhone Simulator often reports the mouse as Direct. Still honor buttonMask.
            const NSUInteger non_primary_buttons =
                static_cast<NSUInteger>(UIEventButtonMaskSecondary | UIEventButtonMaskForButtonNumber(3));
            if ((button_mask & non_primary_buttons) != 0) {
                [self deliverPointer:p phase:phase event:event];
                continue;
            }
        }
        // Stable per-touch identity for the lifetime of the gesture. Bridge to void*
        // first: bitmasking an Objective-C pointer directly is diagnosed.
        const auto touch_key = reinterpret_cast<uintptr_t>((__bridge const void*)touch);
        const uint32_t touch_id = static_cast<uint32_t>(touch_key & 0xFFFFFFFFu);
        xwin_->handleTouch(touch_id, static_cast<double>(p.x), static_cast<double>(p.y), phase);
    }
}

- (void)emitScroll:(UIPanGestureRecognizer*)recognizer discrete:(BOOL)discrete {
    if (!xwin_) {
        return;
    }
    const UIGestureRecognizerState state = recognizer.state;
    const CGPoint loc = [recognizer locationInView:self];
    const CGPoint t = [recognizer translationInView:self];
    if (state != UIGestureRecognizerStateBegan && state != UIGestureRecognizerStateChanged) {
        return;
    }

    // Hover does not move the tracked cursor; update before zoom so hit-tests work.
    xwin_->handleMouseMove(static_cast<double>(loc.x), static_cast<double>(loc.y), 0);
    [recognizer setTranslation:CGPointZero inView:self];
    if (t.x == 0.0 && t.y == 0.0) {
        return;
    }

    // Scroll lines: positive scroll_y zooms in. Discrete wheel: one line per
    // non-zero sample (one notch). Continuous trackpad: scale translation; the
    // mapping is monotonic in |t| and does not promote small chunks to a notch.
    const double dx = discrete ? discreteScrollUnit(t.x) : continuousScrollUnit(t.x);
    const double dy = discrete ? discreteScrollUnit(-t.y) : continuousScrollUnit(-t.y);
    xwin_->handleMouseScroll(dx, dy);
}

- (void)onDiscreteScroll:(UIPanGestureRecognizer*)recognizer {
    [self emitScroll:recognizer discrete:YES];
}

- (void)onContinuousScroll:(UIPanGestureRecognizer*)recognizer {
    [self emitScroll:recognizer discrete:NO];
}

- (void)touchesBegan:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {
    [self deliverTouches:touches phase:vne::xwin::EventBridgeTouchPhase::eDown event:event];
}
- (void)touchesMoved:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {
    [self deliverTouches:touches phase:vne::xwin::EventBridgeTouchPhase::eMove event:event];
}
- (void)touchesEnded:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {
    [self deliverTouches:touches phase:vne::xwin::EventBridgeTouchPhase::eUp event:event];
}
- (void)touchesCancelled:(NSSet<UITouch*>*)touches withEvent:(UIEvent*)event {
    [self deliverTouches:touches phase:vne::xwin::EventBridgeTouchPhase::eUp event:event];
}

@end

@implementation VneXWinUIWindow
- (void)becomeKeyWindow {
    [super becomeKeyWindow];
    if ([self.rootViewController.view isKindOfClass:[VneXWinUIView class]]) {
        [static_cast<VneXWinUIView*>(self.rootViewController.view) activateInputRouting];
    }
}

@end

@implementation VneXWinRootViewController
- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    if ([self.view isKindOfClass:[VneXWinUIView class]]) {
        [static_cast<VneXWinUIView*>(self.view) activateInputRouting];
    }
}

- (BOOL)prefersStatusBarHidden {
    return YES;
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
    if (ui_window_) {
        UIWindow* window = (__bridge_transfer UIWindow*)ui_window_;
        ui_window_ = nullptr;
        window.hidden = YES;
    }
    open_ = false;
}

void UIKitWindow::initialize(const WindowDescriptor& descriptor) {
    uikitRunOnMainSync(^{
      destroyNative();
      desc_ = descriptor;

      VneXWinUIWindow* window = nil;
      UIWindowScene* window_scene = nil;
      if (@available(iOS 13.0, *)) {
          window_scene = vneXWinFindWindowScene(nil, desc_.platform_data);
          if (window_scene != nil) {
              window = [[VneXWinUIWindow alloc] initWithWindowScene:window_scene];
          }
      }
      if (window == nil) {
#if defined(VNE_PLATFORM_VISIONOS)
          window = [[VneXWinUIWindow alloc] initWithFrame:CGRectMake(0, 0, 1280, 720)];
#else
          window = [[VneXWinUIWindow alloc] initWithFrame:UIScreen.mainScreen.bounds];
#endif
      }

      UIWindowScene* bounds_scene = vneXWinFindWindowScene(window, desc_.platform_data);
      if (bounds_scene == nil) {
          bounds_scene = window_scene;
      }

      // The Metal view is the root view, so UIKit always sizes it to the window.
      // desc_.size is a hint only and is overwritten from the view below.
      CGRect bounds = window.bounds;
      if (bounds_scene != nil) {
          bounds = vneXWinSceneBounds(bounds_scene);
      } else if (CGRectIsEmpty(bounds)) {
#if !defined(VNE_PLATFORM_VISIONOS)
          bounds = UIScreen.mainScreen.bounds;
#endif
      }

      // Metal view must be the root view (not a subview). Scroll-type pans and
      // HID I/O land on the first responder / root, not a covered child.
      VneXWinRootViewController* root_vc = [[VneXWinRootViewController alloc] init];
      VneXWinUIView* v = [[VneXWinUIView alloc] initWithFrame:bounds xwin:this];
      v.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
      root_vc.view = v;
      window.rootViewController = root_vc;
      v.frame = window.bounds;

      desc_.size.width = static_cast<uint32_t>(v.bounds.size.width);
      desc_.size.height = static_cast<uint32_t>(v.bounds.size.height);

      ui_view_ = (__bridge_retained void*)v;
      ui_window_ = (__bridge_retained void*)window;
      open_ = (ui_view_ != nullptr) && (ui_window_ != nullptr);
    });
}

void UIKitWindow::handleTouch(uint32_t touch_id, double x, double y, EventBridgeTouchPhase phase) {
    const EventBridgeCallbacks& cb = owner_ ? owner_->eventBridgeCallbacks() : empty_callbacks_;
    eventBridgeTouch(this, desc_, cb, touch_id, x, y, phase);
}

void UIKitWindow::handleMouseButton(vne::events::MouseButton button,
                                    bool pressed,
                                    double x,
                                    double y,
                                    uint8_t modifiers) {
    const EventBridgeCallbacks& cb = owner_ ? owner_->eventBridgeCallbacks() : empty_callbacks_;
    eventBridgeMouseButton(this, desc_, cb, button, pressed, x, y, modifiers);
}

void UIKitWindow::handleMouseMove(double x, double y, uint8_t modifiers) {
    const EventBridgeCallbacks& cb = owner_ ? owner_->eventBridgeCallbacks() : empty_callbacks_;
    eventBridgeMouseMove(this, desc_, cb, x, y, modifiers);
}

void UIKitWindow::handleMouseScroll(double x_offset, double y_offset) {
    const EventBridgeCallbacks& cb = owner_ ? owner_->eventBridgeCallbacks() : empty_callbacks_;
    eventBridgeMouseScroll(this, desc_, cb, static_cast<float>(x_offset), static_cast<float>(y_offset));
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
    // Root view is sized to the window on every layout pass; a caller size cannot hold.
    (void)width;
    (void)height;
}

void UIKitWindow::close() {
    uikitRunOnMainSync(^{
      destroyNative();
    });
}

bool UIKitWindow::isOpen() const noexcept {
    return open_ && ui_view_ != nullptr && ui_window_ != nullptr;
}

NativeWindowHandle UIKitWindow::getNativeHandle() const noexcept {
    NativeWindowHandle handle{};
    handle.api = WindowAPI::eIosUikitWindow;
    handle.ui_view = ui_view_;
    handle.ui_window = ui_window_;
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
    if (ui_view_) {
        __block int width = 0;
        UIKitWindow* self = const_cast<UIKitWindow*>(this);
        uikitRunOnMainSync(^{
          UIView* v = (__bridge UIView*)self->ui_view_;
          width = static_cast<int>(CGRectGetWidth(v.bounds));
        });
        if (width > 0) {
            return width;
        }
    }
    return static_cast<int>(desc_.size.width);
}

int UIKitWindow::getHeight() const noexcept {
    if (ui_view_) {
        __block int height = 0;
        UIKitWindow* self = const_cast<UIKitWindow*>(this);
        uikitRunOnMainSync(^{
          UIView* v = (__bridge UIView*)self->ui_view_;
          height = static_cast<int>(CGRectGetHeight(v.bounds));
        });
        if (height > 0) {
            return height;
        }
    }
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
