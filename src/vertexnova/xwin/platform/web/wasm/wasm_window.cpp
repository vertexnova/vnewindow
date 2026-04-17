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

#include "wasm_map_key.h"
#include "wasm_window_manager.h"
#include "xwin_vne_events_bridge.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/em_asm.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#endif

namespace vne::xwin {

WasmWindow_C::WasmWindow_C() = default;

WasmWindow_C::~WasmWindow_C() {
#ifdef __EMSCRIPTEN__
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, 0, nullptr);
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, 0, nullptr);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, 0, nullptr);
    emscripten_set_mousedown_callback("#canvas", nullptr, 0, nullptr);
    emscripten_set_mouseup_callback("#canvas", nullptr, 0, nullptr);
    emscripten_set_mousemove_callback("#canvas", nullptr, 0, nullptr);
    emscripten_set_wheel_callback("#canvas", nullptr, 0, nullptr);
    emscripten_set_touchstart_callback("#canvas", nullptr, 0, nullptr);
    emscripten_set_touchend_callback("#canvas", nullptr, 0, nullptr);
    emscripten_set_touchmove_callback("#canvas", nullptr, 0, nullptr);
    emscripten_set_touchcancel_callback("#canvas", nullptr, 0, nullptr);
    emscripten_set_fullscreenchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, 0, nullptr);
    emscripten_set_focus_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, 0, nullptr);
    emscripten_set_blur_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, 0, nullptr);
#endif
}

void WasmWindow_C::SetEventOwner(WasmWindowManager_C* owner) {
    _owner = owner;
}

const XWinVneEventCallbacks_C& WasmWindow_C::vneCallbacks() const {
    return _owner ? _owner->vneEventCallbacks() : _empty_callbacks;
}

void WasmWindow_C::Initialize(const WindowDescriptor_C& descriptor) {
    _desc = descriptor;
#ifdef __EMSCRIPTEN__
    _canvas_tag = const_cast<char*>("#canvas");
    emscripten_set_canvas_element_size("#canvas",
                                       static_cast<int>(_desc.size.width),
                                       static_cast<int>(_desc.size.height));

    // Window resize
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, 1, &WasmWindow_C::ResizeCallback);

    // Keyboard (on window so we receive keys even when canvas lacks focus)
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, 1, &WasmWindow_C::KeyDownCallback);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, 1, &WasmWindow_C::KeyUpCallback);

    // Mouse (on canvas)
    emscripten_set_mousedown_callback("#canvas", this, 1, &WasmWindow_C::MouseDownCallback);
    emscripten_set_mouseup_callback("#canvas", this, 1, &WasmWindow_C::MouseUpCallback);
    emscripten_set_mousemove_callback("#canvas", this, 1, &WasmWindow_C::MouseMoveCallback);
    emscripten_set_wheel_callback("#canvas", this, 1, &WasmWindow_C::WheelCallback);

    // Touch (on canvas)
    emscripten_set_touchstart_callback("#canvas", this, 1, &WasmWindow_C::TouchStartCallback);
    emscripten_set_touchend_callback("#canvas", this, 1, &WasmWindow_C::TouchEndCallback);
    emscripten_set_touchmove_callback("#canvas", this, 1, &WasmWindow_C::TouchMoveCallback);
    emscripten_set_touchcancel_callback("#canvas", this, 1, &WasmWindow_C::TouchCancelCallback);

    // Fullscreen
    emscripten_set_fullscreenchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, this, 1,
                                             &WasmWindow_C::FullscreenChangeCallback);

    // Window / document: focus routing for WindowFocusEvent
    emscripten_set_focus_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, 1, &WasmWindow_C::FocusCallback);
    emscripten_set_blur_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, 1, &WasmWindow_C::BlurCallback);
#endif
    _initialized = true;
    _should_close = false;
}

// ---------------------------------------------------------------------------
// Static Emscripten callbacks
// ---------------------------------------------------------------------------

#ifdef __EMSCRIPTEN__

EM_BOOL WasmWindow_C::ResizeCallback(int /*event_type*/, const EmscriptenUiEvent* event, void* user_data) {
    auto* self = static_cast<WasmWindow_C*>(user_data);
    if (!self || !event) { return EM_FALSE; }
    self->_desc.size.width  = static_cast<uint32_t>(event->windowInnerWidth);
    self->_desc.size.height = static_cast<uint32_t>(event->windowInnerHeight);
    emscripten_set_canvas_element_size("#canvas",
                                       static_cast<int>(self->_desc.size.width),
                                       static_cast<int>(self->_desc.size.height));
    xwinVneBridgeWindowResize(self, self->_desc, self->vneCallbacks(),
                              self->_desc.size.width, self->_desc.size.height);
    if (self->_owner) {
        WindowEventData_C data{};
        data.type = WindowEventType_TP::RESIZE;
        data.size = self->_desc.size;
        self->_owner->NotifyWindowEvent(self, data);
    }
    return EM_TRUE;
}

EM_BOOL WasmWindow_C::KeyDownCallback(int /*event_type*/, const EmscriptenKeyboardEvent* ev, void* ud) {
    auto* self = static_cast<WasmWindow_C*>(ud);
    if (!self || !ev) { return EM_FALSE; }
    const vne::events::KeyCode kc = xwinMapEmscriptenKey(ev->code);
    const uint8_t mods = xwinMapEmscriptenModifiers(ev->shiftKey, ev->ctrlKey, ev->altKey, ev->metaKey);
    const bool repeat = ev->repeat;
    xwinVneBridgeKeyDown(self, self->_desc, self->vneCallbacks(), kc, mods, repeat);
    return EM_TRUE;
}

EM_BOOL WasmWindow_C::KeyUpCallback(int /*event_type*/, const EmscriptenKeyboardEvent* ev, void* ud) {
    auto* self = static_cast<WasmWindow_C*>(ud);
    if (!self || !ev) { return EM_FALSE; }
    const vne::events::KeyCode kc = xwinMapEmscriptenKey(ev->code);
    const uint8_t mods = xwinMapEmscriptenModifiers(ev->shiftKey, ev->ctrlKey, ev->altKey, ev->metaKey);
    xwinVneBridgeKeyUp(self, self->_desc, self->vneCallbacks(), kc, mods);
    return EM_TRUE;
}

EM_BOOL WasmWindow_C::MouseDownCallback(int /*event_type*/, const EmscriptenMouseEvent* ev, void* ud) {
    auto* self = static_cast<WasmWindow_C*>(ud);
    if (!self || !ev) { return EM_FALSE; }
    const vne::events::MouseButton btn = xwinMapEmscriptenMouseButton(static_cast<unsigned short>(ev->button));
    const uint8_t mods = xwinMapEmscriptenModifiers(ev->shiftKey, ev->ctrlKey, ev->altKey, ev->metaKey);
    xwinVneBridgeMouseButton(self, self->_desc, self->vneCallbacks(), btn, true,
                             static_cast<double>(ev->targetX), static_cast<double>(ev->targetY), mods);
    return EM_TRUE;
}

EM_BOOL WasmWindow_C::MouseUpCallback(int /*event_type*/, const EmscriptenMouseEvent* ev, void* ud) {
    auto* self = static_cast<WasmWindow_C*>(ud);
    if (!self || !ev) { return EM_FALSE; }
    const vne::events::MouseButton btn = xwinMapEmscriptenMouseButton(static_cast<unsigned short>(ev->button));
    const uint8_t mods = xwinMapEmscriptenModifiers(ev->shiftKey, ev->ctrlKey, ev->altKey, ev->metaKey);
    xwinVneBridgeMouseButton(self, self->_desc, self->vneCallbacks(), btn, false,
                             static_cast<double>(ev->targetX), static_cast<double>(ev->targetY), mods);
    return EM_TRUE;
}

EM_BOOL WasmWindow_C::MouseMoveCallback(int /*event_type*/, const EmscriptenMouseEvent* ev, void* ud) {
    auto* self = static_cast<WasmWindow_C*>(ud);
    if (!self || !ev) { return EM_FALSE; }
    const uint8_t mods = xwinMapEmscriptenModifiers(ev->shiftKey, ev->ctrlKey, ev->altKey, ev->metaKey);
    xwinVneBridgeMouseMove(self, self->_desc, self->vneCallbacks(),
                           static_cast<double>(ev->targetX), static_cast<double>(ev->targetY), mods);
    return EM_TRUE;
}

EM_BOOL WasmWindow_C::WheelCallback(int /*event_type*/, const EmscriptenWheelEvent* ev, void* ud) {
    auto* self = static_cast<WasmWindow_C*>(ud);
    if (!self || !ev) { return EM_FALSE; }
    // deltaX/deltaY are in CSS pixels (deltaMode=0); normalise to scroll steps
    xwinVneBridgeMouseScroll(self, self->_desc, self->vneCallbacks(),
                             static_cast<float>(-ev->deltaX / 100.0),
                             static_cast<float>(-ev->deltaY / 100.0));
    return EM_TRUE;
}

EM_BOOL WasmWindow_C::TouchStartCallback(int /*event_type*/, const EmscriptenTouchEvent* ev, void* ud) {
    auto* self = static_cast<WasmWindow_C*>(ud);
    if (!self || !ev) { return EM_FALSE; }
    for (int i = 0; i < ev->numTouches; ++i) {
        const EmscriptenTouchPoint& tp = ev->touches[i];
        if (!tp.isChanged) { continue; }
        xwinVneBridgeTouch(self, self->_desc, self->vneCallbacks(),
                           static_cast<uint32_t>(tp.identifier),
                           static_cast<double>(tp.targetX),
                           static_cast<double>(tp.targetY),
                           XWinTouchPhase_TP::Down);
    }
    return EM_TRUE;
}

EM_BOOL WasmWindow_C::TouchEndCallback(int /*event_type*/, const EmscriptenTouchEvent* ev, void* ud) {
    auto* self = static_cast<WasmWindow_C*>(ud);
    if (!self || !ev) { return EM_FALSE; }
    for (int i = 0; i < ev->numTouches; ++i) {
        const EmscriptenTouchPoint& tp = ev->touches[i];
        if (!tp.isChanged) { continue; }
        xwinVneBridgeTouch(self, self->_desc, self->vneCallbacks(),
                           static_cast<uint32_t>(tp.identifier),
                           static_cast<double>(tp.targetX),
                           static_cast<double>(tp.targetY),
                           XWinTouchPhase_TP::Up);
    }
    return EM_TRUE;
}

EM_BOOL WasmWindow_C::TouchMoveCallback(int /*event_type*/, const EmscriptenTouchEvent* ev, void* ud) {
    auto* self = static_cast<WasmWindow_C*>(ud);
    if (!self || !ev) { return EM_FALSE; }
    for (int i = 0; i < ev->numTouches; ++i) {
        const EmscriptenTouchPoint& tp = ev->touches[i];
        if (!tp.isChanged) { continue; }
        xwinVneBridgeTouch(self, self->_desc, self->vneCallbacks(),
                           static_cast<uint32_t>(tp.identifier),
                           static_cast<double>(tp.targetX),
                           static_cast<double>(tp.targetY),
                           XWinTouchPhase_TP::Move);
    }
    return EM_TRUE;
}

EM_BOOL WasmWindow_C::TouchCancelCallback(int /*event_type*/, const EmscriptenTouchEvent* ev, void* ud) {
    auto* self = static_cast<WasmWindow_C*>(ud);
    if (!self || !ev) { return EM_FALSE; }
    for (int i = 0; i < ev->numTouches; ++i) {
        const EmscriptenTouchPoint& tp = ev->touches[i];
        if (!tp.isChanged) { continue; }
        xwinVneBridgeTouch(self, self->_desc, self->vneCallbacks(),
                           static_cast<uint32_t>(tp.identifier),
                           static_cast<double>(tp.targetX),
                           static_cast<double>(tp.targetY),
                           XWinTouchPhase_TP::Up);
    }
    return EM_TRUE;
}

EM_BOOL WasmWindow_C::FocusCallback(int /*event_type*/, const EmscriptenFocusEvent*, void* ud) {
    auto* self = static_cast<WasmWindow_C*>(ud);
    if (!self) { return EM_FALSE; }
    xwinVneBridgeWindowFocus(self, self->_desc, self->vneCallbacks(), true);
    if (self->_owner) {
        WindowEventData_C data{};
        data.type = WindowEventType_TP::FOCUS;
        data.focused = true;
        self->_owner->NotifyWindowEvent(self, data);
    }
    return EM_TRUE;
}

EM_BOOL WasmWindow_C::BlurCallback(int /*event_type*/, const EmscriptenFocusEvent*, void* ud) {
    auto* self = static_cast<WasmWindow_C*>(ud);
    if (!self) { return EM_FALSE; }
    xwinVneBridgeWindowFocus(self, self->_desc, self->vneCallbacks(), false);
    if (self->_owner) {
        WindowEventData_C data{};
        data.type = WindowEventType_TP::FOCUS;
        data.focused = false;
        self->_owner->NotifyWindowEvent(self, data);
    }
    return EM_TRUE;
}

EM_BOOL WasmWindow_C::FullscreenChangeCallback(int /*event_type*/,
                                                const EmscriptenFullscreenChangeEvent* ev,
                                                void* ud) {
    auto* self = static_cast<WasmWindow_C*>(ud);
    if (!self || !ev) { return EM_FALSE; }
    self->_fullscreen = ev->isFullscreen;
    return EM_TRUE;
}

#endif  // __EMSCRIPTEN__

// ---------------------------------------------------------------------------
// Window_I interface
// ---------------------------------------------------------------------------

void WasmWindow_C::PollEvents() {
    // Input is driven by Emscripten's JS event loop via the callbacks registered
    // in Initialize(). Nothing to poll manually.
}

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
    return _fullscreen;
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
    int w = 0, h = 0;
    emscripten_get_canvas_element_size("#canvas", &w, &h);
    if (w > 0) { return static_cast<uint32_t>(w); }
#endif
    return static_cast<uint32_t>(_desc.size.width);
}

uint32_t WasmWindow_C::GetFramebufferHeight() const {
#ifdef __EMSCRIPTEN__
    int w = 0, h = 0;
    emscripten_get_canvas_element_size("#canvas", &w, &h);
    if (h > 0) { return static_cast<uint32_t>(h); }
#endif
    return static_cast<uint32_t>(_desc.size.height);
}

void WasmWindow_C::Minimize() {
    // Browser tabs cannot be minimized programmatically from canvas.
}

void WasmWindow_C::Maximize() {
    // No standard browser API; fullscreen is handled by SetFullscreen.
}

void WasmWindow_C::Restore() {
    // See SetFullscreen / Maximize.
}

void WasmWindow_C::SetWindowLimits(const WindowLimits_C& limits) {
    _desc.limits = limits;
}

void WasmWindow_C::SetCursor(WindowCursor_TP cursor) {
#ifdef __EMSCRIPTEN__
    const char* css = "auto";
    switch (cursor) {
        case WindowCursor_TP::HIDDEN:
        case WindowCursor_TP::DISABLED:
            css = "none";
            break;
        case WindowCursor_TP::NORMAL:
        default:
            css = "auto";
            break;
    }
    EM_ASM(
        {
            var s = UTF8ToString($0);
            if (typeof document !== 'undefined' && document.body) { document.body.style.cursor = s; }
            var c = document.querySelector('#canvas');
            if (c) { c.style.cursor = s; }
        },
        css);
#else
    (void)cursor;
#endif
}

}  // namespace vne::xwin
