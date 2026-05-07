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
#include "event_bridge.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/em_asm.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#endif

namespace vne::xwin {

WasmWindow::WasmWindow() = default;

WasmWindow::~WasmWindow() {
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

void WasmWindow::setEventOwner(WasmWindowManager* owner) {
    owner_ = owner;
}

const EventBridgeCallbacks& WasmWindow::eventBridgeCallbacks() const noexcept {
    return owner_ ? owner_->eventBridgeCallbacks() : empty_callbacks_;
}

void WasmWindow::initialize(const WindowDescriptor& descriptor) {
    desc_ = descriptor;
#ifdef __EMSCRIPTEN__
    canvas_tag_ = const_cast<char*>("#canvas");
    emscripten_set_canvas_element_size("#canvas",
                                       static_cast<int>(desc_.size.width),
                                       static_cast<int>(desc_.size.height));

    // Window resize
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, 1, &WasmWindow::ResizeCallback);

    // Keyboard (on window so we receive keys even when canvas lacks focus)
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, 1, &WasmWindow::KeyDownCallback);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, 1, &WasmWindow::KeyUpCallback);

    // Mouse (on canvas)
    emscripten_set_mousedown_callback("#canvas", this, 1, &WasmWindow::MouseDownCallback);
    emscripten_set_mouseup_callback("#canvas", this, 1, &WasmWindow::MouseUpCallback);
    emscripten_set_mousemove_callback("#canvas", this, 1, &WasmWindow::MouseMoveCallback);
    emscripten_set_wheel_callback("#canvas", this, 1, &WasmWindow::WheelCallback);

    // Touch (on canvas)
    emscripten_set_touchstart_callback("#canvas", this, 1, &WasmWindow::TouchStartCallback);
    emscripten_set_touchend_callback("#canvas", this, 1, &WasmWindow::TouchEndCallback);
    emscripten_set_touchmove_callback("#canvas", this, 1, &WasmWindow::TouchMoveCallback);
    emscripten_set_touchcancel_callback("#canvas", this, 1, &WasmWindow::TouchCancelCallback);

    // Fullscreen
    emscripten_set_fullscreenchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT,
                                             this,
                                             1,
                                             &WasmWindow::FullscreenChangeCallback);

    // Window / document: focus routing for WindowFocusEvent
    emscripten_set_focus_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, 1, &WasmWindow::FocusCallback);
    emscripten_set_blur_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, 1, &WasmWindow::BlurCallback);
#endif
    initialized_ = true;
    should_close_ = false;
}

// ---------------------------------------------------------------------------
// Static Emscripten callbacks
// ---------------------------------------------------------------------------

#ifdef __EMSCRIPTEN__

EM_BOOL WasmWindow::ResizeCallback(int /*event_type*/, const EmscriptenUiEvent* event, void* user_data) {
    auto* self = static_cast<WasmWindow*>(user_data);
    if (!self || !event) {
        return EM_FALSE;
    }
    self->desc_.size.width = static_cast<uint32_t>(event->windowInnerWidth);
    self->desc_.size.height = static_cast<uint32_t>(event->windowInnerHeight);
    emscripten_set_canvas_element_size("#canvas",
                                       static_cast<int>(self->desc_.size.width),
                                       static_cast<int>(self->desc_.size.height));
    eventBridgeWindowResize(self,
                            self->desc_,
                            self->eventBridgeCallbacks(),
                            self->desc_.size.width,
                            self->desc_.size.height);
    if (self->owner_) {
        WindowEventData data{};
        data.type = WindowEventType::eResize;
        data.size = self->desc_.size;
        self->owner_->notifyWindowEvent(self, data);
    }
    return EM_TRUE;
}

EM_BOOL WasmWindow::KeyDownCallback(int /*event_type*/, const EmscriptenKeyboardEvent* ev, void* ud) {
    auto* self = static_cast<WasmWindow*>(ud);
    if (!self || !ev) {
        return EM_FALSE;
    }
    const vne::events::KeyCode kc = mapEmscriptenKey(ev->code);
    const uint8_t mods = mapEmscriptenModifiers(ev->shiftKey, ev->ctrlKey, ev->altKey, ev->metaKey);
    const bool repeat = ev->repeat;
    eventBridgeKeyDown(self, self->desc_, self->eventBridgeCallbacks(), kc, mods, repeat);
    return EM_TRUE;
}

EM_BOOL WasmWindow::KeyUpCallback(int /*event_type*/, const EmscriptenKeyboardEvent* ev, void* ud) {
    auto* self = static_cast<WasmWindow*>(ud);
    if (!self || !ev) {
        return EM_FALSE;
    }
    const vne::events::KeyCode kc = mapEmscriptenKey(ev->code);
    const uint8_t mods = mapEmscriptenModifiers(ev->shiftKey, ev->ctrlKey, ev->altKey, ev->metaKey);
    eventBridgeKeyUp(self, self->desc_, self->eventBridgeCallbacks(), kc, mods);
    return EM_TRUE;
}

EM_BOOL WasmWindow::MouseDownCallback(int /*event_type*/, const EmscriptenMouseEvent* ev, void* ud) {
    auto* self = static_cast<WasmWindow*>(ud);
    if (!self || !ev) {
        return EM_FALSE;
    }
    const vne::events::MouseButton btn = mapEmscriptenMouseButton(static_cast<unsigned short>(ev->button));
    const uint8_t mods = mapEmscriptenModifiers(ev->shiftKey, ev->ctrlKey, ev->altKey, ev->metaKey);
    eventBridgeMouseButton(self,
                           self->desc_,
                           self->eventBridgeCallbacks(),
                           btn,
                           true,
                           static_cast<double>(ev->targetX),
                           static_cast<double>(ev->targetY),
                           mods);
    return EM_TRUE;
}

EM_BOOL WasmWindow::MouseUpCallback(int /*event_type*/, const EmscriptenMouseEvent* ev, void* ud) {
    auto* self = static_cast<WasmWindow*>(ud);
    if (!self || !ev) {
        return EM_FALSE;
    }
    const vne::events::MouseButton btn = mapEmscriptenMouseButton(static_cast<unsigned short>(ev->button));
    const uint8_t mods = mapEmscriptenModifiers(ev->shiftKey, ev->ctrlKey, ev->altKey, ev->metaKey);
    eventBridgeMouseButton(self,
                           self->desc_,
                           self->eventBridgeCallbacks(),
                           btn,
                           false,
                           static_cast<double>(ev->targetX),
                           static_cast<double>(ev->targetY),
                           mods);
    return EM_TRUE;
}

EM_BOOL WasmWindow::MouseMoveCallback(int /*event_type*/, const EmscriptenMouseEvent* ev, void* ud) {
    auto* self = static_cast<WasmWindow*>(ud);
    if (!self || !ev) {
        return EM_FALSE;
    }
    const uint8_t mods = mapEmscriptenModifiers(ev->shiftKey, ev->ctrlKey, ev->altKey, ev->metaKey);
    eventBridgeMouseMove(self,
                         self->desc_,
                         self->eventBridgeCallbacks(),
                         static_cast<double>(ev->targetX),
                         static_cast<double>(ev->targetY),
                         mods);
    return EM_TRUE;
}

EM_BOOL WasmWindow::WheelCallback(int /*event_type*/, const EmscriptenWheelEvent* ev, void* ud) {
    auto* self = static_cast<WasmWindow*>(ud);
    if (!self || !ev) {
        return EM_FALSE;
    }
    // deltaX/deltaY are in CSS pixels (deltaMode=0); normalise to scroll steps
    eventBridgeMouseScroll(self,
                           self->desc_,
                           self->eventBridgeCallbacks(),
                           static_cast<float>(-ev->deltaX / 100.0),
                           static_cast<float>(-ev->deltaY / 100.0));
    return EM_TRUE;
}

EM_BOOL WasmWindow::TouchStartCallback(int /*event_type*/, const EmscriptenTouchEvent* ev, void* ud) {
    auto* self = static_cast<WasmWindow*>(ud);
    if (!self || !ev) {
        return EM_FALSE;
    }
    for (int i = 0; i < ev->numTouches; ++i) {
        const EmscriptenTouchPoint& tp = ev->touches[i];
        if (!tp.isChanged) {
            continue;
        }
        eventBridgeTouch(self,
                         self->desc_,
                         self->eventBridgeCallbacks(),
                         static_cast<uint32_t>(tp.identifier),
                         static_cast<double>(tp.targetX),
                         static_cast<double>(tp.targetY),
                         EventBridgeTouchPhase::eDown);
    }
    return EM_TRUE;
}

EM_BOOL WasmWindow::TouchEndCallback(int /*event_type*/, const EmscriptenTouchEvent* ev, void* ud) {
    auto* self = static_cast<WasmWindow*>(ud);
    if (!self || !ev) {
        return EM_FALSE;
    }
    for (int i = 0; i < ev->numTouches; ++i) {
        const EmscriptenTouchPoint& tp = ev->touches[i];
        if (!tp.isChanged) {
            continue;
        }
        eventBridgeTouch(self,
                         self->desc_,
                         self->eventBridgeCallbacks(),
                         static_cast<uint32_t>(tp.identifier),
                         static_cast<double>(tp.targetX),
                         static_cast<double>(tp.targetY),
                         EventBridgeTouchPhase::eUp);
    }
    return EM_TRUE;
}

EM_BOOL WasmWindow::TouchMoveCallback(int /*event_type*/, const EmscriptenTouchEvent* ev, void* ud) {
    auto* self = static_cast<WasmWindow*>(ud);
    if (!self || !ev) {
        return EM_FALSE;
    }
    for (int i = 0; i < ev->numTouches; ++i) {
        const EmscriptenTouchPoint& tp = ev->touches[i];
        if (!tp.isChanged) {
            continue;
        }
        eventBridgeTouch(self,
                         self->desc_,
                         self->eventBridgeCallbacks(),
                         static_cast<uint32_t>(tp.identifier),
                         static_cast<double>(tp.targetX),
                         static_cast<double>(tp.targetY),
                         EventBridgeTouchPhase::eMove);
    }
    return EM_TRUE;
}

EM_BOOL WasmWindow::TouchCancelCallback(int /*event_type*/, const EmscriptenTouchEvent* ev, void* ud) {
    auto* self = static_cast<WasmWindow*>(ud);
    if (!self || !ev) {
        return EM_FALSE;
    }
    for (int i = 0; i < ev->numTouches; ++i) {
        const EmscriptenTouchPoint& tp = ev->touches[i];
        if (!tp.isChanged) {
            continue;
        }
        eventBridgeTouch(self,
                         self->desc_,
                         self->eventBridgeCallbacks(),
                         static_cast<uint32_t>(tp.identifier),
                         static_cast<double>(tp.targetX),
                         static_cast<double>(tp.targetY),
                         EventBridgeTouchPhase::eUp);
    }
    return EM_TRUE;
}

EM_BOOL WasmWindow::FocusCallback(int /*event_type*/, const EmscriptenFocusEvent*, void* ud) {
    auto* self = static_cast<WasmWindow*>(ud);
    if (!self) {
        return EM_FALSE;
    }
    eventBridgeWindowFocus(self, self->desc_, self->eventBridgeCallbacks(), true);
    if (self->owner_) {
        WindowEventData data{};
        data.type = WindowEventType::eFocus;
        data.focused = true;
        self->owner_->notifyWindowEvent(self, data);
    }
    return EM_TRUE;
}

EM_BOOL WasmWindow::BlurCallback(int /*event_type*/, const EmscriptenFocusEvent*, void* ud) {
    auto* self = static_cast<WasmWindow*>(ud);
    if (!self) {
        return EM_FALSE;
    }
    eventBridgeWindowFocus(self, self->desc_, self->eventBridgeCallbacks(), false);
    if (self->owner_) {
        WindowEventData data{};
        data.type = WindowEventType::eFocus;
        data.focused = false;
        self->owner_->notifyWindowEvent(self, data);
    }
    return EM_TRUE;
}

EM_BOOL WasmWindow::FullscreenChangeCallback(int /*event_type*/, const EmscriptenFullscreenChangeEvent* ev, void* ud) {
    auto* self = static_cast<WasmWindow*>(ud);
    if (!self || !ev) {
        return EM_FALSE;
    }
    self->fullscreen_ = ev->isFullscreen;
    return EM_TRUE;
}

#endif  // __EMSCRIPTEN__

// ---------------------------------------------------------------------------
// IWindow interface
// ---------------------------------------------------------------------------

void WasmWindow::pollEvents() {
    // Input is driven by Emscripten's JS event loop via the callbacks registered
    // in initialize(). Nothing to poll manually.
}

void WasmWindow::swapBuffers() {}

void WasmWindow::setTitle(const std::string& title) {
    desc_.title = title;
#ifdef __EMSCRIPTEN__
    emscripten_set_window_title(title.c_str());
#endif
}

void WasmWindow::setWindowMode(WindowMode mode) {
    desc_.mode = mode;
}

WindowMode WasmWindow::getWindowMode() const noexcept {
    return WindowMode::eWindowed;
}

void WasmWindow::setFullscreen(bool enabled) {
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

bool WasmWindow::isFullscreen() const noexcept {
    return fullscreen_;
}

void WasmWindow::setPosition(int x, int y) {
    desc_.position.x = x;
    desc_.position.y = y;
}

void WasmWindow::getPosition(int& x, int& y) const {
    x = desc_.position.x;
    y = desc_.position.y;
}

void WasmWindow::resize(uint32_t width, uint32_t height) {
    desc_.size.width = width;
    desc_.size.height = height;
#ifdef __EMSCRIPTEN__
    emscripten_set_canvas_element_size("#canvas", static_cast<int>(width), static_cast<int>(height));
#endif
}

void WasmWindow::close() {
    should_close_ = true;
}

bool WasmWindow::isOpen() const noexcept {
    return !should_close_ && initialized_;
}


NativeWindowHandle WasmWindow::getNativeHandle() const noexcept {
    NativeWindowHandle handle{};
    handle.api = WindowAPI::eWasmWindow;
    handle.canvas_id = "#canvas";
    return handle;
}

WindowAPI WasmWindow::getWindowAPI() const noexcept {
    return WindowAPI::eWasmWindow;
}

int WasmWindow::getWidth() const noexcept {
    return static_cast<int>(desc_.size.width);
}

int WasmWindow::getHeight() const noexcept {
    return static_cast<int>(desc_.size.height);
}

uint32_t WasmWindow::getFramebufferWidth() const noexcept {
#ifdef __EMSCRIPTEN__
    int w = 0, h = 0;
    emscripten_get_canvas_element_size("#canvas", &w, &h);
    if (w > 0) {
        return static_cast<uint32_t>(w);
    }
#endif
    return static_cast<uint32_t>(desc_.size.width);
}

uint32_t WasmWindow::getFramebufferHeight() const noexcept {
#ifdef __EMSCRIPTEN__
    int w = 0, h = 0;
    emscripten_get_canvas_element_size("#canvas", &w, &h);
    if (h > 0) {
        return static_cast<uint32_t>(h);
    }
#endif
    return static_cast<uint32_t>(desc_.size.height);
}

float WasmWindow::getDpiScale() const noexcept {
#ifdef __EMSCRIPTEN__
    return static_cast<float>(emscripten_get_device_pixel_ratio());
#else
    return 1.0F;
#endif
}

void WasmWindow::minimize() {
    // Browser tabs cannot be minimized programmatically from canvas.
}

void WasmWindow::maximize() {
    // No standard browser API; fullscreen is handled by setFullscreen.
}

void WasmWindow::restore() {
    // See setFullscreen / maximize.
}

void WasmWindow::setWindowLimits(const WindowLimits& limits) {
    desc_.limits = limits;
}

void WasmWindow::setCursor(WindowCursor cursor) {
#ifdef __EMSCRIPTEN__
    const char* css = "auto";
    switch (cursor) {
        case WindowCursor::eHidden:
        case WindowCursor::eDisabled:
            css = "none";
            break;
        case WindowCursor::eNormal:
        default:
            css = "auto";
            break;
    }
    EM_ASM(
        {
            var s = UTF8ToString($0);
            if (typeof document != = 'undefined' && document.body) {
                document.body.style.cursor = s;
            }
            var c = document.querySelector('#canvas');
            if (c) {
                c.style.cursor = s;
            }
        },
        css);
#else
    (void)cursor;
#endif
}

}  // namespace vne::xwin
