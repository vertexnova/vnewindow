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
#include "event_emitter.h"

#include <algorithm>
#include <unordered_map>

#ifdef __EMSCRIPTEN__
#include <emscripten/em_js.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

EM_JS(void, vne_xwin_wasm_apply_cursor_css, (const char* selector, const char* css_cstr), {
    var s = UTF8ToString(css_cstr);
    var sel = UTF8ToString(selector);
    if (typeof document !== 'undefined') {
        var c = document.querySelector(sel);
        if (c) {
            c.style.cursor = s;
        }
    }
});

EM_JS(void, vne_xwin_shell_create_window,
      (int id, const char* title, int w, int h, int x, int y, int is_primary),
      {
          if (typeof window.VneShell !== 'undefined') {
              window.VneShell.createWindow(id, UTF8ToString(title), w, h, x, y, is_primary !== 0);
          }
      });

EM_JS(void, vne_xwin_shell_close_window, (int id), {
    if (typeof window.VneShell !== 'undefined') {
        window.VneShell.closeWindow(id);
    }
});

EM_JS(void, vne_xwin_shell_focus_window, (int id), {
    if (typeof window.VneShell !== 'undefined') {
        window.VneShell.focusWindow(id);
    }
});

EM_JS(void, vne_xwin_shell_set_title, (int id, const char* title), {
    if (typeof window.VneShell !== 'undefined') {
        window.VneShell.setTitle(id, UTF8ToString(title));
    }
});

EM_JS(void, vne_xwin_shell_set_position, (int id, int x, int y), {
    if (typeof window.VneShell !== 'undefined') {
        window.VneShell.setPosition(id, x, y);
    }
});

EM_JS(void, vne_xwin_shell_set_size, (int id, int w, int h), {
    if (typeof window.VneShell !== 'undefined') {
        window.VneShell.setSize(id, w, h);
    }
});

EM_JS(int, vne_xwin_shell_content_width, (int id), {
    if (typeof window.VneShell !== 'undefined' && window.VneShell.getContentSize) {
        return window.VneShell.getContentSize(id).width | 0;
    }
    return 0;
});

EM_JS(int, vne_xwin_shell_content_height, (int id), {
    if (typeof window.VneShell !== 'undefined' && window.VneShell.getContentSize) {
        return window.VneShell.getContentSize(id).height | 0;
    }
    return 0;
});
#endif

namespace vne::xwin {

#ifdef __EMSCRIPTEN__
namespace {
/**
 * Live windows by shell id. The shell owns layout, so when it re-tiles it must tell each window
 * its new content size; that call arrives from JS with only an id to go on.
 */
std::unordered_map<int, WasmWindow*> g_shell_windows;
}  // namespace

extern "C" EMSCRIPTEN_KEEPALIVE void vne_xwin_on_shell_resize(int id, int width, int height) {
    const auto it = g_shell_windows.find(id);
    if (it == g_shell_windows.end() || it->second == nullptr || width <= 0 || height <= 0) {
        return;  // Unknown id: a window still initializing, or already gone. Both are benign.
    }
    it->second->applyViewportSize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
}
#endif

namespace {
#ifdef __EMSCRIPTEN__
[[nodiscard]] bool isValidMouseButton(vne::events::MouseButton button) noexcept {
    return static_cast<uint8_t>(button) <= static_cast<uint8_t>(vne::events::MouseButton::eLast);
}
#endif
}  // namespace

WasmWindow::WasmWindow() = default;

WasmWindow::~WasmWindow() {
#ifdef __EMSCRIPTEN__
    g_shell_windows.erase(static_cast<int>(id_));
#endif
#ifdef __EMSCRIPTEN__
    unregisterCanvasCallbacks();
#endif
}

void WasmWindow::setEventOwner(WasmWindowManager* owner) {
    owner_ = owner;
}

void WasmWindow::initialize(const WindowDescriptor& descriptor) {
    desc_ = descriptor;
#ifdef __EMSCRIPTEN__
    const int dom_id = static_cast<int>(id_);
    const bool legacy_shell = is_primary_ && detectLegacyCanvasShell();
    if (!is_primary_ && !detectVneShell()) {
        should_close_ = true;
        return;
    }

    uses_vne_shell_ = !legacy_shell && detectVneShell();
    if (uses_vne_shell_) {
        vne_xwin_shell_create_window(dom_id,
                                     desc_.title.c_str(),
                                     static_cast<int>(desc_.size.width),
                                     static_cast<int>(desc_.size.height),
                                     desc_.position.x,
                                     desc_.position.y,
                                     is_primary_ ? 1 : 0);
    }

    canvas_selector_ = is_primary_ ? "#canvas" : ("#canvas-" + std::to_string(dom_id));

    registerCanvasCallbacks();

    // Prefer the shell's laid-out content size (fill mode = desktop minus titlebar).
    // Falling back to the requested size avoids the old left-anchored 640x480 chip.
    uint32_t css_w = desc_.size.width;
    uint32_t css_h = desc_.size.height;
    if (uses_vne_shell_) {
        const int content_w = vne_xwin_shell_content_width(dom_id);
        const int content_h = vne_xwin_shell_content_height(dom_id);
        if (content_w > 0 && content_h > 0) {
            css_w = static_cast<uint32_t>(content_w);
            css_h = static_cast<uint32_t>(content_h);
        }
    } else if (is_primary_) {
        int vp_w = 0;
        int vp_h = 0;
        if (queryBrowserViewport(vp_w, vp_h) && vp_w > 0 && vp_h > 0) {
            css_w = std::min(css_w, static_cast<uint32_t>(vp_w));
            css_h = std::min(css_h, static_cast<uint32_t>(vp_h));
        }
    }
    applyViewportSize(css_w, css_h);
#endif
#ifdef __EMSCRIPTEN__
    g_shell_windows[dom_id] = this;
#endif
    initialized_ = true;
    should_close_ = false;
}

void WasmWindow::registerCanvasCallbacks() {
    const char* sel = canvasSelector();
    emscripten_set_mousedown_callback(sel, this, 1, &WasmWindow::MouseDownCallback);
    emscripten_set_mouseup_callback(sel, this, 1, &WasmWindow::MouseUpCallback);
    emscripten_set_mousemove_callback(sel, this, 1, &WasmWindow::MouseMoveCallback);
    emscripten_set_wheel_callback(sel, this, 1, &WasmWindow::WheelCallback);
    emscripten_set_touchstart_callback(sel, this, 1, &WasmWindow::TouchStartCallback);
    emscripten_set_touchend_callback(sel, this, 1, &WasmWindow::TouchEndCallback);
    emscripten_set_touchmove_callback(sel, this, 1, &WasmWindow::TouchMoveCallback);
    emscripten_set_touchcancel_callback(sel, this, 1, &WasmWindow::TouchCancelCallback);
    emscripten_set_fullscreenchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT,
                                             this,
                                             1,
                                             &WasmWindow::FullscreenChangeCallback);
}

void WasmWindow::unregisterCanvasCallbacks() {
    if (canvas_selector_.empty()) {
        return;
    }
    const char* sel = canvasSelector();
    emscripten_set_mousedown_callback(sel, nullptr, 0, nullptr);
    emscripten_set_mouseup_callback(sel, nullptr, 0, nullptr);
    emscripten_set_mousemove_callback(sel, nullptr, 0, nullptr);
    emscripten_set_wheel_callback(sel, nullptr, 0, nullptr);
    emscripten_set_touchstart_callback(sel, nullptr, 0, nullptr);
    emscripten_set_touchend_callback(sel, nullptr, 0, nullptr);
    emscripten_set_touchmove_callback(sel, nullptr, 0, nullptr);
    emscripten_set_touchcancel_callback(sel, nullptr, 0, nullptr);
    emscripten_set_fullscreenchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, 0, nullptr);
}

void WasmWindow::dispatchKeyDown(const vne::events::KeyCode key, const uint8_t mods, const bool repeat) {
    events_.keyDown(key, mods, repeat);
}

void WasmWindow::dispatchKeyUp(const vne::events::KeyCode key, const uint8_t mods) {
    events_.keyUp(key, mods);
}

void WasmWindow::emitWindowFocus(const bool focused) {
    events_.windowFocus(focused);
}

// ---------------------------------------------------------------------------
// Static Emscripten callbacks
// ---------------------------------------------------------------------------

#ifdef __EMSCRIPTEN__

bool WasmWindow::detectVneShell() noexcept {
    // clang-format off -- `!==` inside EM_ASM is lexed as C++ (`!=` then `=`) and reflowed apart,
    // which emits `!= =` and breaks module init. Keep this body on one line and out of the
    // formatter's reach. `typeof` yields a string, so `!=` is equivalent to `!==` here.
    return EM_ASM_INT({ return (typeof window != 'undefined' && window.VneShell) ? 1 : 0; }) != 0;
    // clang-format on
}

bool WasmWindow::detectLegacyCanvasShell() noexcept {
    // clang-format off -- see detectVneShell() above.
    return EM_ASM_INT({ return (typeof document != 'undefined' && document.getElementById('canvas')) ? 1 : 0; }) != 0;
    // clang-format on
}

bool WasmWindow::queryBrowserViewport(int& out_width, int& out_height) {
    out_width = EM_ASM_INT({
        if (typeof document !== 'undefined') {
            var wrap = document.getElementById('canvas-wrap');
            if (wrap && wrap.clientWidth > 0) {
                return wrap.clientWidth | 0;
            }
            var desktop = document.getElementById('vne-desktop');
            if (desktop && desktop.clientWidth > 0) {
                return desktop.clientWidth | 0;
            }
        }
        return (window && window.innerWidth) ? (window.innerWidth | 0) : 0;
    });
    out_height = EM_ASM_INT({
        if (typeof document !== 'undefined') {
            var wrap = document.getElementById('canvas-wrap');
            if (wrap && wrap.clientHeight > 0) {
                return wrap.clientHeight | 0;
            }
            var desktop = document.getElementById('vne-desktop');
            if (desktop && desktop.clientHeight > 0) {
                return desktop.clientHeight | 0;
            }
        }
        return (window && window.innerHeight) ? (window.innerHeight | 0) : 0;
    });
    return out_width > 0 && out_height > 0;
}

void WasmWindow::applyViewportSize(const uint32_t css_width, const uint32_t css_height) {
    if (css_width == 0 || css_height == 0 || canvas_selector_.empty()) {
        return;
    }

    uint32_t width = css_width;
    uint32_t height = css_height;
    // In VneShell fill mode the panel owns CSS layout; sync backing store to the
    // real content box so the canvas is not stretched from a 640x480 buffer.
    if (uses_vne_shell_) {
        const int content_w = vne_xwin_shell_content_width(static_cast<int>(id_));
        const int content_h = vne_xwin_shell_content_height(static_cast<int>(id_));
        if (content_w > 0 && content_h > 0) {
            width = static_cast<uint32_t>(content_w);
            height = static_cast<uint32_t>(content_h);
        }
        vne_xwin_shell_set_size(static_cast<int>(id_), static_cast<int>(width), static_cast<int>(height));
    }

    desc_.size.width = width;
    desc_.size.height = height;

    const float dpr = emscripten_get_device_pixel_ratio();
    const int backing_w = static_cast<int>(static_cast<float>(width) * dpr);
    const int backing_h = static_cast<int>(static_cast<float>(height) * dpr);

    // CSS size is owned by the shell (.fills / flex); only set the framebuffer.
    if (!uses_vne_shell_) {
        emscripten_set_element_css_size(canvasSelector(), static_cast<double>(width), static_cast<double>(height));
    }
    emscripten_set_canvas_element_size(canvasSelector(), backing_w, backing_h);

    events_.windowResize(width, height);
    events_.windowDpiChanged(dpr);
}

EM_BOOL WasmWindow::MouseDownCallback(int /*event_type*/, const EmscriptenMouseEvent* ev, void* ud) {
    auto* self = static_cast<WasmWindow*>(ud);
    if (!self || !ev) {
        return EM_FALSE;
    }
    if (self->owner_) {
        self->owner_->focusWindowFromCanvas(self);
    }
    const vne::events::MouseButton btn = mapEmscriptenMouseButton(static_cast<unsigned short>(ev->button));
    if (!isValidMouseButton(btn)) {
        return EM_FALSE;
    }
    const uint8_t mods = mapEmscriptenModifiers(ev->shiftKey, ev->ctrlKey, ev->altKey, ev->metaKey);
    self->events_.mouseButton(btn, true, static_cast<double>(ev->targetX), static_cast<double>(ev->targetY), mods);
    return EM_TRUE;
}

EM_BOOL WasmWindow::MouseUpCallback(int /*event_type*/, const EmscriptenMouseEvent* ev, void* ud) {
    auto* self = static_cast<WasmWindow*>(ud);
    if (!self || !ev) {
        return EM_FALSE;
    }
    const vne::events::MouseButton btn = mapEmscriptenMouseButton(static_cast<unsigned short>(ev->button));
    if (!isValidMouseButton(btn)) {
        return EM_FALSE;
    }
    const uint8_t mods = mapEmscriptenModifiers(ev->shiftKey, ev->ctrlKey, ev->altKey, ev->metaKey);
    self->events_.mouseButton(btn, false, static_cast<double>(ev->targetX), static_cast<double>(ev->targetY), mods);
    return EM_TRUE;
}

EM_BOOL WasmWindow::MouseMoveCallback(int /*event_type*/, const EmscriptenMouseEvent* ev, void* ud) {
    auto* self = static_cast<WasmWindow*>(ud);
    if (!self || !ev) {
        return EM_FALSE;
    }
    const uint8_t mods = mapEmscriptenModifiers(ev->shiftKey, ev->ctrlKey, ev->altKey, ev->metaKey);
    self->events_.mouseMove(static_cast<double>(ev->targetX), static_cast<double>(ev->targetY), mods);
    return EM_TRUE;
}

EM_BOOL WasmWindow::WheelCallback(int /*event_type*/, const EmscriptenWheelEvent* ev, void* ud) {
    auto* self = static_cast<WasmWindow*>(ud);
    if (!self || !ev) {
        return EM_FALSE;
    }
    // deltaX/deltaY are in CSS pixels (deltaMode=0); normalise to scroll steps
    const uint8_t mods =
        mapEmscriptenModifiers(ev->mouse.shiftKey, ev->mouse.ctrlKey, ev->mouse.altKey, ev->mouse.metaKey);
    self->events_.mouseScroll(static_cast<float>(-ev->deltaX / 100.0),
                              static_cast<float>(-ev->deltaY / 100.0),
                              static_cast<double>(ev->mouse.targetX),
                              static_cast<double>(ev->mouse.targetY),
                              mods);
    return EM_TRUE;
}

EM_BOOL WasmWindow::TouchStartCallback(int /*event_type*/, const EmscriptenTouchEvent* ev, void* ud) {
    auto* self = static_cast<WasmWindow*>(ud);
    if (!self || !ev) {
        return EM_FALSE;
    }
    const uint8_t mods = mapEmscriptenModifiers(ev->shiftKey, ev->ctrlKey, ev->altKey, ev->metaKey);
    for (int i = 0; i < ev->numTouches; ++i) {
        const EmscriptenTouchPoint& tp = ev->touches[i];
        if (!tp.isChanged) {
            continue;
        }
        self->events_.touch(static_cast<uint32_t>(tp.identifier),
                            static_cast<double>(tp.targetX),
                            static_cast<double>(tp.targetY),
                            TouchPhase::eDown,
                            mods);
    }
    return EM_TRUE;
}

EM_BOOL WasmWindow::TouchEndCallback(int /*event_type*/, const EmscriptenTouchEvent* ev, void* ud) {
    auto* self = static_cast<WasmWindow*>(ud);
    if (!self || !ev) {
        return EM_FALSE;
    }
    const uint8_t mods = mapEmscriptenModifiers(ev->shiftKey, ev->ctrlKey, ev->altKey, ev->metaKey);
    for (int i = 0; i < ev->numTouches; ++i) {
        const EmscriptenTouchPoint& tp = ev->touches[i];
        if (!tp.isChanged) {
            continue;
        }
        self->events_.touch(static_cast<uint32_t>(tp.identifier),
                            static_cast<double>(tp.targetX),
                            static_cast<double>(tp.targetY),
                            TouchPhase::eUp,
                            mods);
    }
    return EM_TRUE;
}

EM_BOOL WasmWindow::TouchMoveCallback(int /*event_type*/, const EmscriptenTouchEvent* ev, void* ud) {
    auto* self = static_cast<WasmWindow*>(ud);
    if (!self || !ev) {
        return EM_FALSE;
    }
    const uint8_t mods = mapEmscriptenModifiers(ev->shiftKey, ev->ctrlKey, ev->altKey, ev->metaKey);
    for (int i = 0; i < ev->numTouches; ++i) {
        const EmscriptenTouchPoint& tp = ev->touches[i];
        if (!tp.isChanged) {
            continue;
        }
        self->events_.touch(static_cast<uint32_t>(tp.identifier),
                            static_cast<double>(tp.targetX),
                            static_cast<double>(tp.targetY),
                            TouchPhase::eMove,
                            mods);
    }
    return EM_TRUE;
}

EM_BOOL WasmWindow::TouchCancelCallback(int /*event_type*/, const EmscriptenTouchEvent* ev, void* ud) {
    auto* self = static_cast<WasmWindow*>(ud);
    if (!self || !ev) {
        return EM_FALSE;
    }
    const uint8_t mods = mapEmscriptenModifiers(ev->shiftKey, ev->ctrlKey, ev->altKey, ev->metaKey);
    for (int i = 0; i < ev->numTouches; ++i) {
        const EmscriptenTouchPoint& tp = ev->touches[i];
        if (!tp.isChanged) {
            continue;
        }
        self->events_.touch(static_cast<uint32_t>(tp.identifier),
                            static_cast<double>(tp.targetX),
                            static_cast<double>(tp.targetY),
                            TouchPhase::eUp,
                            mods);
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
    if (uses_vne_shell_) {
        vne_xwin_shell_set_title(static_cast<int>(id_), title.c_str());
    } else {
        emscripten_set_window_title(title.c_str());
    }
#endif
}

void WasmWindow::setWindowMode(WindowMode mode) {
    desc_.mode = mode;
}

WindowMode WasmWindow::getWindowMode() const noexcept {
    return desc_.mode;
}

void WasmWindow::setFullscreen(bool enabled) {
#ifdef __EMSCRIPTEN__
    if (enabled) {
        emscripten_request_fullscreen(canvasSelector(), 1);
    } else {
        emscripten_exit_fullscreen();
    }
#endif
    fullscreen_ = enabled;
}

bool WasmWindow::isFullscreen() const noexcept {
    return fullscreen_;
}

void WasmWindow::setPosition(int x, int y) {
    desc_.position.x = x;
    desc_.position.y = y;
#ifdef __EMSCRIPTEN__
    if (uses_vne_shell_) {
        vne_xwin_shell_set_position(static_cast<int>(id_), x, y);
    }
#endif
}

WindowPosition WasmWindow::getPosition() const {
    return desc_.position;
}

void WasmWindow::resize(uint32_t width, uint32_t height) {
#ifdef __EMSCRIPTEN__
    applyViewportSize(width, height);
#else
    desc_.size.width = width;
    desc_.size.height = height;
#endif
}

void WasmWindow::close() {
    if (should_close_) {
        return;
    }
    should_close_ = true;
    initialized_ = false;
#ifdef __EMSCRIPTEN__
    unregisterCanvasCallbacks();
    if (uses_vne_shell_) {
        vne_xwin_shell_close_window(static_cast<int>(id_));
    }
#endif
    events_.windowClose();
}

bool WasmWindow::isOpen() const noexcept {
    return !should_close_ && initialized_;
}

NativeWindowHandle WasmWindow::getNativeHandle() const noexcept {
    NativeWindowHandle handle{};
    handle.api = WindowAPI::eWasmWindow;
    handle.canvas_id = canvas_selector_.empty() ? "#canvas" : canvas_selector_.c_str();
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
    if (!canvas_selector_.empty()) {
        int w = 0;
        int h = 0;
        emscripten_get_canvas_element_size(canvasSelector(), &w, &h);
        if (w > 0) {
            return static_cast<uint32_t>(w);
        }
    }
#endif
    return static_cast<uint32_t>(desc_.size.width);
}

uint32_t WasmWindow::getFramebufferHeight() const noexcept {
#ifdef __EMSCRIPTEN__
    if (!canvas_selector_.empty()) {
        int w = 0;
        int h = 0;
        emscripten_get_canvas_element_size(canvasSelector(), &w, &h);
        if (h > 0) {
            return static_cast<uint32_t>(h);
        }
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
    vne_xwin_wasm_apply_cursor_css(canvasSelector(), css);
#else
    (void)cursor;
#endif
}

}  // namespace vne::xwin
