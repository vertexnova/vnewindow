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

class Win32Window_C final : public IWindow {
   public:
    Win32Window_C();
    ~Win32Window_C() override;

    void Initialize(const WindowDescriptor& descriptor) override;
    void PollEvents() override;
    void SwapBuffers() override;
    void SetTitle(const std::string& title) override;
    void SetWindowMode(WindowMode mode) override;
    WindowMode GetWindowMode() const override;
    void SetFullscreen(bool enabled) override;
    bool IsFullscreen() const override;
    void Minimize() override;
    void Maximize() override;
    void Restore() override;
    void SetWindowLimits(const WindowLimits& limits) override;
    void SetCursor(WindowCursor cursor) override;
    void SetPosition(int x, int y) override;
    void GetPosition(int& x, int& y) const override;
    void Resize(uint32_t width, uint32_t height) override;
    void Close() override;
    bool IsOpen() const override;
    void* GetNativeWindow() const override;
    NativeWindowHandle GetNativeHandle() const override;
    WindowAPI GetWindowAPI() const override;
    int GetWidth() const override;
    int GetHeight() const override;
    float GetDPIScale() const override;
    std::string GetClipboardText() const override;
    void SetClipboardText(const std::string& text) override;
    void SetWindowIcon(const uint8_t* rgba_pixels, uint32_t width, uint32_t height) override;

    /** @brief Used by Win32WindowManager_C to deliver manager-level callbacks. */
    void SetEventOwner(Win32WindowManager_C* owner);

    LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

   private:
    static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void create_window(const WindowDescriptor& descriptor);
    void destroy_window();

    HWND hwnd_ = nullptr;
    Win32WindowManager_C* event_owner_ = nullptr;
    WindowDescriptor desc_{};
    bool open_ = false;
    bool fullscreen_ = false;
    WindowMode mode_ = WindowMode::eWindowed;

    // Saved pre-fullscreen state
    DWORD saved_style_ = 0;
    RECT saved_rect_ = {};
};

}  // namespace vne::xwin
