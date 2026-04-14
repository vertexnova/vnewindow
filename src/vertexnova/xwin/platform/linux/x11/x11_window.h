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

#include <X11/Xlib.h>

#include <array>
#include <string>

namespace vne::xwin {

class X11WindowManager_C;

class X11Window_C final : public Window_I {
   public:
    X11Window_C();
    ~X11Window_C() override;

    void SetEventOwner(X11WindowManager_C* owner);

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

    void SetDisplay(Display* display, int screen, ::Window root);

   private:
    void destroy();

    Display* _display = nullptr;
    int _screen = 0;
    ::Window _root = 0;
    ::Window _window = 0;
    X11WindowManager_C* _owner = nullptr;
    WindowDescriptor_C _desc{};
    bool _open = false;
    Atom _wm_delete = 0;
    /** Physical keycodes currently held (for KeyPress repeat detection). */
    std::array<bool, 256> _keycode_down{};
};

}  // namespace vne::xwin
