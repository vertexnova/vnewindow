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

void NullWindow::Initialize(const WindowDescriptor& descriptor) {
    descriptor_ = descriptor;
    open_ = true;
}

void NullWindow::PollEvents() {}

void NullWindow::SwapBuffers() {}

void NullWindow::SetTitle(const std::string& title) {
    descriptor_.title = title;
}

void NullWindow::SetWindowMode(WindowMode mode) {
    descriptor_.mode = mode;
}

WindowMode NullWindow::GetWindowMode() const noexcept {
    return descriptor_.mode;
}

void NullWindow::SetFullscreen(bool enabled) {
    if (enabled) {
        descriptor_.mode = WindowMode::eFullscreen;
    } else {
        descriptor_.mode = WindowMode::eWindowed;
    }
}

bool NullWindow::IsFullscreen() const noexcept {
    return descriptor_.mode == WindowMode::eFullscreen;
}

void NullWindow::Minimize() {}

void NullWindow::Maximize() {}

void NullWindow::Restore() {}

void NullWindow::SetWindowLimits(const WindowLimits& limits) {
    descriptor_.limits = limits;
}

void NullWindow::SetCursor(WindowCursor cursor) {
    (void)cursor;
}

void NullWindow::SetPosition(int x, int y) {
    descriptor_.position.x = x;
    descriptor_.position.y = y;
}

void NullWindow::GetPosition(int& x, int& y) const {
    x = descriptor_.position.x;
    y = descriptor_.position.y;
}

void NullWindow::Resize(uint32_t width, uint32_t height) {
    descriptor_.size.width = width;
    descriptor_.size.height = height;
}

void NullWindow::Close() {
    open_ = false;
}

bool NullWindow::IsOpen() const noexcept {
    return open_;
}

void* NullWindow::GetNativeWindow() const noexcept {
    return nullptr;
}

NativeWindowHandle NullWindow::GetNativeHandle() const noexcept {
    return {};
}

WindowAPI NullWindow::GetWindowAPI() const noexcept {
    return WindowAPI::eNullWindow;
}

int NullWindow::GetWidth() const noexcept {
    return static_cast<int>(descriptor_.size.width);
}

int NullWindow::GetHeight() const noexcept {
    return static_cast<int>(descriptor_.size.height);
}

float NullWindow::GetDPIScale() const noexcept {
    return 1.0F;
}

}  // namespace vne::xwin
