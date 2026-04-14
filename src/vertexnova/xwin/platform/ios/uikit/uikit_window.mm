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

#import <UIKit/UIKit.h>

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
    CGRect frame = CGRectMake(static_cast<CGFloat>(_desc.position.x), static_cast<CGFloat>(_desc.position.y),
                              static_cast<CGFloat>(_desc.size.width), static_cast<CGFloat>(_desc.size.height));
    UIView* v = [[UIView alloc] initWithFrame:frame];
    v.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    _ui_view = (__bridge_retained void*)v;
    _open = true;
}

void UIKitWindow_C::PollEvents() {}

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
    (void)enabled;
}

bool UIKitWindow_C::IsFullscreen() const {
    return false;
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
        f.size.width = static_cast<CGFloat>(width);
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
    if (!s) {
        return 1.0F;
    }
    return static_cast<float>(s.scale);
}

}  // namespace vne::xwin
