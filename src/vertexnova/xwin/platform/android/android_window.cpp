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

#include "event_emitter.h"

namespace vne::xwin {

AndroidWindow::AndroidWindow() = default;

AndroidWindow::~AndroidWindow() = default;

void AndroidWindow::swapBuffers() {}

void AndroidWindow::setTitle(const std::string& title) {
    desc_.title = title;
}

void AndroidWindow::initialize(const WindowDescriptor& descriptor) {
    desc_ = descriptor;
    native_ = descriptor.platform_data;
    open_ = native_ != nullptr;
}

void AndroidWindow::setWindowMode(WindowMode mode) {
    desc_.mode = mode;
}

WindowMode AndroidWindow::getWindowMode() const noexcept {
    return desc_.mode;
}

void AndroidWindow::setFullscreen(bool enabled) {
    desc_.mode = enabled ? WindowMode::eFullscreen : WindowMode::eWindowed;
}

bool AndroidWindow::isFullscreen() const noexcept {
    return desc_.mode == WindowMode::eFullscreen;
}

void AndroidWindow::setPosition(int x, int y) {
    desc_.position.x = x;
    desc_.position.y = y;
}

WindowPosition AndroidWindow::getPosition() const {
    return desc_.position;
}

void AndroidWindow::resize(uint32_t width, uint32_t height) {
    desc_.size.width = width;
    desc_.size.height = height;
}

void AndroidWindow::minimize() {
    // Window state is owned by the Activity; hook via JNI if needed.
}

void AndroidWindow::maximize() {}

void AndroidWindow::restore() {}

void AndroidWindow::setWindowLimits(const WindowLimits& limits) {
    desc_.limits = limits;
}

void AndroidWindow::setCursor(WindowCursor cursor) {
    (void)cursor;
}

void AndroidWindow::close() {
    open_ = false;
    native_ = nullptr;
}

bool AndroidWindow::isOpen() const noexcept {
    return open_ && native_ != nullptr;
}

NativeWindowHandle AndroidWindow::getNativeHandle() const noexcept {
    NativeWindowHandle handle{};
    handle.api = WindowAPI::eAndroidSurfaceWindow;
    handle.a_native_window = native_;
    return handle;
}

WindowAPI AndroidWindow::getWindowAPI() const noexcept {
    return WindowAPI::eAndroidSurfaceWindow;
}

int AndroidWindow::getWidth() const noexcept {
    return static_cast<int>(desc_.size.width);
}

int AndroidWindow::getHeight() const noexcept {
    return static_cast<int>(desc_.size.height);
}

float AndroidWindow::getDpiScale() const noexcept {
    return 1.0F;
}

void AndroidWindow::injectTouchEvent(uint32_t touch_id, double x, double y, TouchPhase phase, uint8_t modifiers) {
    events_.touch(touch_id, x, y, phase, modifiers);
}

void AndroidWindow::injectKeyEvent(vne::events::KeyCode key, bool down, uint8_t modifiers, bool repeat) {
    if (down) {
        events_.keyDown(key, modifiers, repeat);
    } else {
        events_.keyUp(key, modifiers);
    }
}

void AndroidWindow::injectTextInput(const char* utf8_text) {
    events_.textInput(utf8_text);
}

void AndroidWindow::injectResizeEvent(uint32_t width, uint32_t height) {
    desc_.size.width = width;
    desc_.size.height = height;
    events_.windowResize(width, height);
}

void AndroidWindow::injectFocusEvent(bool focused) {
    events_.windowFocus(focused);
}

void AndroidWindow::injectDpiChanged(float scale) {
    events_.windowDpiChanged(scale);
}

void AndroidWindow::injectSafeAreaChanged(float top, float left, float bottom, float right) {
    events_.windowSafeAreaChanged(top, left, bottom, right);
}

}  // namespace vne::xwin
