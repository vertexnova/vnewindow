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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <string>

namespace vne::xwin {

class Win32WindowManager_C;

class Win32Window_C final : public Window_I {
   public:
    Win32Window_C();
    ~Win32Window_C() override;

    void Initialize(const WindowDescriptor_C& descriptor) override;
    void PollEvents() override;
    void SwapBuffers() override;
    void SetTitle(const std::string& title) override;
    void SetWindowMode(WindowMode_TP mode) override;
    WindowMode_TP GetWindowMode() const override;
    void SetFullscreen(bool enabled) override;
    bool IsFullscreen() const override;
    void Minimize() override;
    void Maximize() override;
    void Restore() override;
    void SetWindowLimits(const WindowLimits_C& limits) override;
    void SetCursor(WindowCursor_TP cursor) override;
    void SetPosition(int x, int y) override;
    void GetPosition(int& x, int& y) const override;
    void Resize(uint32_t width, uint32_t height) override;
    void Close() override;
    bool IsOpen() const override;
    void* GetNativeWindow() const override;
    WindowAPI_TP GetWindowAPI() const override;
    int GetWidth() const override;
    int GetHeight() const override;
    float GetDPIScale() const override;

    /** @brief Used by Win32WindowManager_C to deliver manager-level callbacks. */
    void SetEventOwner(Win32WindowManager_C* owner);

    LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

   private:
    static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void create_window(const WindowDescriptor_C& descriptor);
    void destroy_window();

    HWND _hwnd = nullptr;
    Win32WindowManager_C* _event_owner = nullptr;
    WindowDescriptor_C _desc{};
    bool _open = false;
    bool _fullscreen = false;
    WindowMode_TP _mode = WindowMode_TP::WINDOWED;

    // Saved pre-fullscreen state
    DWORD _saved_style = 0;
    RECT _saved_rect = {};
};

}  // namespace vne::xwin
