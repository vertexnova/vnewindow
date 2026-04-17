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

void AndroidWindow_C::Initialize(const WindowDescriptor_C& descriptor) {
    _desc = descriptor;
    _native = descriptor.platform_data;
    _open = _native != nullptr;
}

void AndroidWindow_C::PollEvents() {}

void AndroidWindow_C::SwapBuffers() {}

void AndroidWindow_C::SetTitle(const std::string& title) {
    _desc.title = title;
}

void AndroidWindow_C::SetWindowMode(WindowMode_TP mode) {
    _desc.mode = mode;
}

WindowMode_TP AndroidWindow_C::GetWindowMode() const {
    return _desc.mode;
}

void AndroidWindow_C::SetFullscreen(bool enabled) {
    (void)enabled;
}

bool AndroidWindow_C::IsFullscreen() const {
    return false;
}

void AndroidWindow_C::SetPosition(int x, int y) {
    _desc.position.x = x;
    _desc.position.y = y;
}

void AndroidWindow_C::GetPosition(int& x, int& y) const {
    x = _desc.position.x;
    y = _desc.position.y;
}

void AndroidWindow_C::Resize(uint32_t width, uint32_t height) {
    _desc.size.width = width;
    _desc.size.height = height;
}

void AndroidWindow_C::Minimize() {
    // Window state is owned by the Activity; hook via JNI if needed.
}

void AndroidWindow_C::Maximize() {}

void AndroidWindow_C::Restore() {}

void AndroidWindow_C::SetWindowLimits(const WindowLimits_C& limits) {
    _desc.limits = limits;
}

void AndroidWindow_C::SetCursor(WindowCursor_TP cursor) {
    (void)cursor;
}

void AndroidWindow_C::Close() {
    _open = false;
    _native = nullptr;
}

bool AndroidWindow_C::IsOpen() const {
    return _open && _native != nullptr;
}

void* AndroidWindow_C::GetNativeWindow() const {
    return _native;
}

WindowAPI_TP AndroidWindow_C::GetWindowAPI() const {
    return WindowAPI_TP::ANDROID_SURFACE_WINDOW;
}

int AndroidWindow_C::GetWidth() const {
    return static_cast<int>(_desc.size.width);
}

int AndroidWindow_C::GetHeight() const {
    return static_cast<int>(_desc.size.height);
}

void AndroidWindow_C::InjectTouchEvent(uint32_t touch_id, double x, double y, EventBridgeTouchPhase_C phase) {
    eventBridgeTouch(this, _desc, _event_bridge_callbacks, touch_id, x, y, phase);
}

void AndroidWindow_C::InjectKeyEvent(vne::events::KeyCode key, bool down, uint8_t modifiers) {
    if (down) {
        eventBridgeKeyDown(this, _desc, _event_bridge_callbacks, key, modifiers, false);
    } else {
        eventBridgeKeyUp(this, _desc, _event_bridge_callbacks, key, modifiers);
    }
}

void AndroidWindow_C::InjectResizeEvent(uint32_t width, uint32_t height) {
    _desc.size.width = width;
    _desc.size.height = height;
    eventBridgeWindowResize(this, _desc, _event_bridge_callbacks, width, height);
}

void AndroidWindow_C::setEventBridgeCallbacks(EventBridgeCallbacks_C callbacks) {
    _event_bridge_callbacks = std::move(callbacks);
}

}  // namespace vne::xwin
