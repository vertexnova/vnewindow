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

#include "null_window.h"

namespace vne::xwin {

void NullWindow::initialize(const WindowDescriptor& descriptor) {
    descriptor_ = descriptor;
    open_ = true;
}

void NullWindow::pollEvents() {}

void NullWindow::swapBuffers() {}

void NullWindow::setTitle(const std::string& title) {
    descriptor_.title = title;
}

void NullWindow::setWindowMode(WindowMode mode) {
    descriptor_.mode = mode;
}

WindowMode NullWindow::getWindowMode() const noexcept {
    return descriptor_.mode;
}

void NullWindow::setFullscreen(bool enabled) {
    if (enabled) {
        descriptor_.mode = WindowMode::eFullscreen;
    } else {
        descriptor_.mode = WindowMode::eWindowed;
    }
}

bool NullWindow::isFullscreen() const noexcept {
    return descriptor_.mode == WindowMode::eFullscreen;
}

void NullWindow::minimize() {}

void NullWindow::maximize() {}

void NullWindow::restore() {}

void NullWindow::setWindowLimits(const WindowLimits& limits) {
    descriptor_.limits = limits;
}

void NullWindow::setCursor(WindowCursor cursor) {
    (void)cursor;
}

void NullWindow::setPosition(int x, int y) {
    descriptor_.position.x = x;
    descriptor_.position.y = y;
}

WindowPosition NullWindow::getPosition() const {
    return descriptor_.position;
}

void NullWindow::resize(uint32_t width, uint32_t height) {
    descriptor_.size.width = width;
    descriptor_.size.height = height;
}

void NullWindow::close() {
    open_ = false;
}

bool NullWindow::isOpen() const noexcept {
    return open_;
}

NativeWindowHandle NullWindow::getNativeHandle() const noexcept {
    return {};
}

WindowAPI NullWindow::getWindowAPI() const noexcept {
    return WindowAPI::eNullWindow;
}

int NullWindow::getWidth() const noexcept {
    return static_cast<int>(descriptor_.size.width);
}

int NullWindow::getHeight() const noexcept {
    return static_cast<int>(descriptor_.size.height);
}

float NullWindow::getDpiScale() const noexcept {
    return 1.0F;
}

}  // namespace vne::xwin
