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

#include "wasm_window.h"

#include "wasm_window_manager.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

namespace vne::xwin {

WasmWindow_C::WasmWindow_C() = default;

WasmWindow_C::~WasmWindow_C() {
#ifdef __EMSCRIPTEN__
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, 0, nullptr);
#endif
}

void WasmWindow_C::SetEventOwner(WasmWindowManager_C* owner) {
    _owner = owner;
}

void WasmWindow_C::Initialize(const WindowDescriptor_C& descriptor) {
    _desc = descriptor;
#ifdef __EMSCRIPTEN__
    _canvas_tag = const_cast<char*>("#canvas");
    emscripten_set_canvas_element_size("#canvas", static_cast<int>(_desc.size.width), static_cast<int>(_desc.size.height));
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, 1, &WasmWindow_C::ResizeCallback);
#endif
    _initialized = true;
    _should_close = false;
}

#ifdef __EMSCRIPTEN__
EM_BOOL WasmWindow_C::ResizeCallback(int /*event_type*/, const EmscriptenUiEvent* event, void* user_data) {
    auto* self = static_cast<WasmWindow_C*>(user_data);
    if (!self || !event) {
        return EM_FALSE;
    }
    self->_desc.size.width = static_cast<uint32_t>(event->windowInnerWidth);
    self->_desc.size.height = static_cast<uint32_t>(event->windowInnerHeight);
    emscripten_set_canvas_element_size("#canvas", static_cast<int>(self->_desc.size.width), static_cast<int>(self->_desc.size.height));
    if (self->_owner) {
        WindowEventData_C data{};
        data.type = WindowEventType_TP::RESIZE;
        data.size = self->_desc.size;
        self->_owner->NotifyWindowEvent(self, data);
    }
    return EM_TRUE;
}
#endif

void WasmWindow_C::PollEvents() {}

void WasmWindow_C::SwapBuffers() {}

void WasmWindow_C::SetTitle(const std::string& title) {
    _desc.title = title;
#ifdef __EMSCRIPTEN__
    emscripten_set_window_title(title.c_str());
#endif
}

void WasmWindow_C::SetWindowMode(WindowMode_TP mode) {
    _desc.mode = mode;
}

WindowMode_TP WasmWindow_C::GetWindowMode() const {
    return WindowMode_TP::WINDOWED;
}

void WasmWindow_C::SetFullscreen(bool enabled) {
#ifdef __EMSCRIPTEN__
    if (enabled) {
        emscripten_request_fullscreen("#canvas", 1);
    } else {
        emscripten_exit_fullscreen();
    }
#else
    (void)enabled;
#endif
}

bool WasmWindow_C::IsFullscreen() const {
    return false;
}

void WasmWindow_C::SetPosition(int x, int y) {
    _desc.position.x = x;
    _desc.position.y = y;
}

void WasmWindow_C::GetPosition(int& x, int& y) const {
    x = _desc.position.x;
    y = _desc.position.y;
}

void WasmWindow_C::Resize(uint32_t width, uint32_t height) {
    _desc.size.width = width;
    _desc.size.height = height;
#ifdef __EMSCRIPTEN__
    emscripten_set_canvas_element_size("#canvas", static_cast<int>(width), static_cast<int>(height));
#endif
}

void WasmWindow_C::Close() {
    _should_close = true;
}

bool WasmWindow_C::IsOpen() const {
    return !_should_close && _initialized;
}

void* WasmWindow_C::GetNativeWindow() const {
    return _canvas_tag;
}

WindowAPI_TP WasmWindow_C::GetWindowAPI() const {
    return WindowAPI_TP::WASM_WINDOW;
}

int WasmWindow_C::GetWidth() const {
    return static_cast<int>(_desc.size.width);
}

int WasmWindow_C::GetHeight() const {
    return static_cast<int>(_desc.size.height);
}

uint32_t WasmWindow_C::GetFramebufferWidth() const {
#ifdef __EMSCRIPTEN__
    int w = 0;
    int h = 0;
    emscripten_get_canvas_element_size("#canvas", &w, &h);
    if (w > 0) {
        return static_cast<uint32_t>(w);
    }
#endif
    return static_cast<uint32_t>(_desc.size.width);
}

uint32_t WasmWindow_C::GetFramebufferHeight() const {
#ifdef __EMSCRIPTEN__
    int w = 0;
    int h = 0;
    emscripten_get_canvas_element_size("#canvas", &w, &h);
    if (h > 0) {
        return static_cast<uint32_t>(h);
    }
#endif
    return static_cast<uint32_t>(_desc.size.height);
}

}  // namespace vne::xwin
