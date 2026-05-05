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

void NullWindow_C::Initialize(const WindowDescriptor& descriptor) {
    descriptor_ = descriptor;
    open_ = true;
}

void NullWindow_C::PollEvents() {}

void NullWindow_C::SwapBuffers() {}

void NullWindow_C::SetTitle(const std::string& title) {
    descriptor_.title = title;
}

void NullWindow_C::SetWindowMode(WindowMode_TP mode) {
    descriptor_.mode = mode;
}

WindowMode_TP NullWindow_C::GetWindowMode() const {
    return descriptor_.mode;
}

void NullWindow_C::SetFullscreen(bool enabled) {
    if (enabled) {
        descriptor_.mode = WindowMode_TP::FULLSCREEN;
    } else {
        descriptor_.mode = WindowMode_TP::WINDOWED;
    }
}

bool NullWindow_C::IsFullscreen() const {
    return descriptor_.mode == WindowMode_TP::FULLSCREEN;
}

void NullWindow_C::Minimize() {}

void NullWindow_C::Maximize() {}

void NullWindow_C::Restore() {}

void NullWindow_C::SetWindowLimits(const WindowLimits& limits) {
    descriptor_.limits = limits;
}

void NullWindow_C::SetCursor(WindowCursor_TP cursor) {
    (void)cursor;
}

void NullWindow_C::SetPosition(int x, int y) {
    descriptor_.position.x = x;
    descriptor_.position.y = y;
}

void NullWindow_C::GetPosition(int& x, int& y) const {
    x = descriptor_.position.x;
    y = descriptor_.position.y;
}

void NullWindow_C::Resize(uint32_t width, uint32_t height) {
    descriptor_.size.width = width;
    descriptor_.size.height = height;
}

void NullWindow_C::Close() {
    open_ = false;
}

bool NullWindow_C::IsOpen() const {
    return open_;
}

void* NullWindow_C::GetNativeWindow() const {
    return nullptr;
}

NativeWindowHandle NullWindow_C::GetNativeHandle() const {
    return {};
}

WindowAPI_TP NullWindow_C::GetWindowAPI() const {
    return WindowAPI_TP::NULL_WINDOW;
}

int NullWindow_C::GetWidth() const {
    return static_cast<int>(descriptor_.size.width);
}

int NullWindow_C::GetHeight() const {
    return static_cast<int>(descriptor_.size.height);
}

}  // namespace vne::xwin
