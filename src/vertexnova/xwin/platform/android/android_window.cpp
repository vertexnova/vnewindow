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

#include "android_window.h"

#include "event_bridge.h"

namespace vne::xwin {

AndroidWindow_C::AndroidWindow_C() = default;

AndroidWindow_C::~AndroidWindow_C() = default;

void AndroidWindow_C::Initialize(const WindowDescriptor& descriptor) {
    desc_ = descriptor;
    native_ = descriptor.platform_data;
    open_ = native_ != nullptr;
}

void AndroidWindow_C::PollEvents() {}

void AndroidWindow_C::SwapBuffers() {}

void AndroidWindow_C::SetTitle(const std::string& title) {
    desc_.title = title;
}

void AndroidWindow_C::SetWindowMode(WindowMode_TP mode) {
    desc_.mode = mode;
}

WindowMode_TP AndroidWindow_C::GetWindowMode() const {
    return desc_.mode;
}

void AndroidWindow_C::SetFullscreen(bool enabled) {
    (void)enabled;
}

bool AndroidWindow_C::IsFullscreen() const {
    return false;
}

void AndroidWindow_C::SetPosition(int x, int y) {
    desc_.position.x = x;
    desc_.position.y = y;
}

void AndroidWindow_C::GetPosition(int& x, int& y) const {
    x = desc_.position.x;
    y = desc_.position.y;
}

void AndroidWindow_C::Resize(uint32_t width, uint32_t height) {
    desc_.size.width = width;
    desc_.size.height = height;
}

void AndroidWindow_C::Minimize() {
    // Window state is owned by the Activity; hook via JNI if needed.
}

void AndroidWindow_C::Maximize() {}

void AndroidWindow_C::Restore() {}

void AndroidWindow_C::SetWindowLimits(const WindowLimits& limits) {
    desc_.limits = limits;
}

void AndroidWindow_C::SetCursor(WindowCursor_TP cursor) {
    (void)cursor;
}

void AndroidWindow_C::Close() {
    open_ = false;
    native_ = nullptr;
}

bool AndroidWindow_C::IsOpen() const {
    return open_ && native_ != nullptr;
}

void* AndroidWindow_C::GetNativeWindow() const {
    return native_;
}

NativeWindowHandle AndroidWindow_C::GetNativeHandle() const {
    NativeWindowHandle handle{};
    handle.api = WindowAPI_TP::ANDROID_SURFACE_WINDOW;
    handle.a_native_window = native_;
    return handle;
}

WindowAPI_TP AndroidWindow_C::GetWindowAPI() const {
    return WindowAPI_TP::ANDROID_SURFACE_WINDOW;
}

int AndroidWindow_C::GetWidth() const {
    return static_cast<int>(desc_.size.width);
}

int AndroidWindow_C::GetHeight() const {
    return static_cast<int>(desc_.size.height);
}

float AndroidWindow_C::GetDPIScale() const {
    return 1.0F;
}

void AndroidWindow_C::InjectTouchEvent(uint32_t touch_id, double x, double y, EventBridgeTouchPhase phase) {
    eventBridgeTouch(this, desc_, event_bridge_callbacks_, touch_id, x, y, phase);
}

void AndroidWindow_C::InjectKeyEvent(vne::events::KeyCode key, bool down, uint8_t modifiers) {
    if (down) {
        eventBridgeKeyDown(this, desc_, event_bridge_callbacks_, key, modifiers, false);
    } else {
        eventBridgeKeyUp(this, desc_, event_bridge_callbacks_, key, modifiers);
    }
}

void AndroidWindow_C::InjectResizeEvent(uint32_t width, uint32_t height) {
    desc_.size.width = width;
    desc_.size.height = height;
    eventBridgeWindowResize(this, desc_, event_bridge_callbacks_, width, height);
}

void AndroidWindow_C::setEventBridgeCallbacks(EventBridgeCallbacks callbacks) {
    event_bridge_callbacks_ = std::move(callbacks);
}

}  // namespace vne::xwin
