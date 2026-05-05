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

void NullWindow_C::Initialize(const WindowDescriptor_C& descriptor) {
    _descriptor = descriptor;
    _open = true;
}

void NullWindow_C::PollEvents() {}

void NullWindow_C::SwapBuffers() {}

void NullWindow_C::SetTitle(const std::string& title) {
    _descriptor.title = title;
}

void NullWindow_C::SetWindowMode(WindowMode_TP mode) {
    _descriptor.mode = mode;
}

WindowMode_TP NullWindow_C::GetWindowMode() const {
    return _descriptor.mode;
}

void NullWindow_C::SetFullscreen(bool enabled) {
    if (enabled) {
        _descriptor.mode = WindowMode_TP::FULLSCREEN;
    } else {
        _descriptor.mode = WindowMode_TP::WINDOWED;
    }
}

bool NullWindow_C::IsFullscreen() const {
    return _descriptor.mode == WindowMode_TP::FULLSCREEN;
}

void NullWindow_C::Minimize() {}

void NullWindow_C::Maximize() {}

void NullWindow_C::Restore() {}

void NullWindow_C::SetWindowLimits(const WindowLimits_C& limits) {
    _descriptor.limits = limits;
}

void NullWindow_C::SetCursor(WindowCursor_TP cursor) {
    (void)cursor;
}

void NullWindow_C::SetPosition(int x, int y) {
    _descriptor.position.x = x;
    _descriptor.position.y = y;
}

void NullWindow_C::GetPosition(int& x, int& y) const {
    x = _descriptor.position.x;
    y = _descriptor.position.y;
}

void NullWindow_C::Resize(uint32_t width, uint32_t height) {
    _descriptor.size.width = width;
    _descriptor.size.height = height;
}

void NullWindow_C::Close() {
    _open = false;
}

bool NullWindow_C::IsOpen() const {
    return _open;
}

void* NullWindow_C::GetNativeWindow() const {
    return nullptr;
}

NativeWindowHandle_C NullWindow_C::GetNativeHandle() const {
    return {};
}

WindowAPI_TP NullWindow_C::GetWindowAPI() const {
    return WindowAPI_TP::NULL_WINDOW;
}

int NullWindow_C::GetWidth() const {
    return static_cast<int>(_descriptor.size.width);
}

int NullWindow_C::GetHeight() const {
    return static_cast<int>(_descriptor.size.height);
}

}  // namespace vne::xwin
