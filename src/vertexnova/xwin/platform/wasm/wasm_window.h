#pragma once
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

#include "vertexnova/xwin/window.h"
#include "event_emitter.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#endif

#include <string>

namespace vne::xwin {

class WasmWindowManager;

class WasmWindow final : public IWindow {
   public:
    WasmWindow();
    ~WasmWindow() override;

    void setEventOwner(WasmWindowManager* owner);

    void prepareInitialize(bool is_primary) noexcept { is_primary_ = is_primary; }

    void initialize(const WindowDescriptor& descriptor) override;
    void pollEvents() override;
    void swapBuffers() override;
    void setTitle(const std::string& title) override;
    void setWindowMode(WindowMode mode) override;
    [[nodiscard]] WindowMode getWindowMode() const noexcept override;
    void setFullscreen(bool enabled) override;
    [[nodiscard]] bool isFullscreen() const noexcept override;
    void setPosition(int x, int y) override;
    [[nodiscard]] WindowPosition getPosition() const override;
    void resize(uint32_t width, uint32_t height) override;
    void minimize() override;
    void maximize() override;
    void restore() override;
    void setWindowLimits(const WindowLimits& limits) override;
    void setCursor(WindowCursor cursor) override;
    void close() override;
    [[nodiscard]] bool isOpen() const noexcept override;
    [[nodiscard]] vne::events::WindowId getId() const noexcept override { return id_; }
    [[nodiscard]] NativeWindowHandle getNativeHandle() const noexcept override;
    [[nodiscard]] WindowAPI getWindowAPI() const noexcept override;
    [[nodiscard]] int getWidth() const noexcept override;
    [[nodiscard]] int getHeight() const noexcept override;
    [[nodiscard]] uint32_t getFramebufferWidth() const noexcept override;
    [[nodiscard]] uint32_t getFramebufferHeight() const noexcept override;
    [[nodiscard]] float getDpiScale() const noexcept override;

    void dispatchKeyDown(vne::events::KeyCode key, uint8_t mods, bool repeat);
    void dispatchKeyUp(vne::events::KeyCode key, uint8_t mods);
    void emitWindowFocus(bool focused);
    [[nodiscard]] bool isPrimary() const noexcept { return is_primary_; }
    [[nodiscard]] bool usesVneShell() const noexcept { return uses_vne_shell_; }

#ifdef __EMSCRIPTEN__
    void applyViewportSize(uint32_t css_width, uint32_t css_height);
    [[nodiscard]] static bool queryBrowserViewport(int& out_width, int& out_height);
    [[nodiscard]] static bool detectVneShell() noexcept;
    [[nodiscard]] static bool detectLegacyCanvasShell() noexcept;

    static EM_BOOL MouseDownCallback(int event_type, const EmscriptenMouseEvent* ev, void* ud);
    static EM_BOOL MouseUpCallback(int event_type, const EmscriptenMouseEvent* ev, void* ud);
    static EM_BOOL MouseMoveCallback(int event_type, const EmscriptenMouseEvent* ev, void* ud);
    static EM_BOOL WheelCallback(int event_type, const EmscriptenWheelEvent* ev, void* ud);
    static EM_BOOL TouchStartCallback(int event_type, const EmscriptenTouchEvent* ev, void* ud);
    static EM_BOOL TouchEndCallback(int event_type, const EmscriptenTouchEvent* ev, void* ud);
    static EM_BOOL TouchMoveCallback(int event_type, const EmscriptenTouchEvent* ev, void* ud);
    static EM_BOOL TouchCancelCallback(int event_type, const EmscriptenTouchEvent* ev, void* ud);
    static EM_BOOL FullscreenChangeCallback(int event_type, const EmscriptenFullscreenChangeEvent* ev, void* ud);
#endif

   private:
#ifdef __EMSCRIPTEN__
    [[nodiscard]] const char* canvasSelector() const noexcept { return canvas_selector_.c_str(); }
    void registerCanvasCallbacks();
    void unregisterCanvasCallbacks();
#endif

    const vne::events::WindowId id_ = IWindow::nextId();
    WasmWindowManager* owner_ = nullptr;
    WindowDescriptor desc_{};
    bool initialized_ = false;
    bool should_close_ = false;
    bool fullscreen_ = false;
    bool is_primary_ = false;
    bool uses_vne_shell_ = false;
    float applied_dpr_ = 0.0F;
    std::string canvas_selector_;

    // Declared last: binds to desc_ by reference.
    EventEmitter events_{this, desc_};
};

}  // namespace vne::xwin
