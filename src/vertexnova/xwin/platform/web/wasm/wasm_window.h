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
#include "vertexnova/xwin/xwin_vne_event_callbacks.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#endif

#include <string>

namespace vne::xwin {

class WasmWindowManager_C;

class WasmWindow_C final : public Window_I {
   public:
    WasmWindow_C();
    ~WasmWindow_C() override;

    void SetEventOwner(WasmWindowManager_C* owner);

    void Initialize(const WindowDescriptor_C& descriptor) override;
    void PollEvents() override;
    void SwapBuffers() override;
    void SetTitle(const std::string& title) override;
    void SetWindowMode(WindowMode_TP mode) override;
    WindowMode_TP GetWindowMode() const override;
    void SetFullscreen(bool enabled) override;
    bool IsFullscreen() const override;
    void SetPosition(int x, int y) override;
    void GetPosition(int& x, int& y) const override;
    void Resize(uint32_t width, uint32_t height) override;
    void Close() override;
    bool IsOpen() const override;
    void* GetNativeWindow() const override;
    WindowAPI_TP GetWindowAPI() const override;
    int GetWidth() const override;
    int GetHeight() const override;
    uint32_t GetFramebufferWidth() const override;
    uint32_t GetFramebufferHeight() const override;

#ifdef __EMSCRIPTEN__
    static EM_BOOL ResizeCallback(int event_type, const EmscriptenUiEvent* event, void* user_data);
    static EM_BOOL KeyDownCallback(int event_type, const EmscriptenKeyboardEvent* ev, void* ud);
    static EM_BOOL KeyUpCallback(int event_type, const EmscriptenKeyboardEvent* ev, void* ud);
    static EM_BOOL MouseDownCallback(int event_type, const EmscriptenMouseEvent* ev, void* ud);
    static EM_BOOL MouseUpCallback(int event_type, const EmscriptenMouseEvent* ev, void* ud);
    static EM_BOOL MouseMoveCallback(int event_type, const EmscriptenMouseEvent* ev, void* ud);
    static EM_BOOL WheelCallback(int event_type, const EmscriptenWheelEvent* ev, void* ud);
    static EM_BOOL TouchStartCallback(int event_type, const EmscriptenTouchEvent* ev, void* ud);
    static EM_BOOL TouchEndCallback(int event_type, const EmscriptenTouchEvent* ev, void* ud);
    static EM_BOOL TouchMoveCallback(int event_type, const EmscriptenTouchEvent* ev, void* ud);
    static EM_BOOL FullscreenChangeCallback(int event_type, const EmscriptenFullscreenChangeEvent* ev, void* ud);
#endif

   private:
    const XWinVneEventCallbacks_C& vneCallbacks() const;

    WasmWindowManager_C* _owner = nullptr;
    WindowDescriptor_C _desc{};
    bool _initialized = false;
    bool _should_close = false;
    bool _fullscreen = false;
    void* _canvas_tag = nullptr;

    XWinVneEventCallbacks_C _empty_callbacks{};
};

}  // namespace vne::xwin
