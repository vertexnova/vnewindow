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

class Win32WindowManager;

class Win32Window final : public IWindow {
   public:
    Win32Window();
    ~Win32Window() override;

    void initialize(const WindowDescriptor& descriptor) override;
    void pollEvents() override;
    void swapBuffers() override;
    void setTitle(const std::string& title) override;
    void setWindowMode(WindowMode mode) override;
    [[nodiscard]] WindowMode getWindowMode() const noexcept override;
    void setFullscreen(bool enabled) override;
    [[nodiscard]] bool isFullscreen() const noexcept override;
    void minimize() override;
    void maximize() override;
    void restore() override;
    void setWindowLimits(const WindowLimits& limits) override;
    void setCursor(WindowCursor cursor) override;
    void setPosition(int x, int y) override;
    void getPosition(int& x, int& y) const override;
    void resize(uint32_t width, uint32_t height) override;
    void close() override;
    [[nodiscard]] bool isOpen() const noexcept override;
    [[nodiscard]] NativeWindowHandle getNativeHandle() const noexcept override;
    [[nodiscard]] WindowAPI getWindowAPI() const noexcept override;
    [[nodiscard]] int getWidth() const noexcept override;
    [[nodiscard]] int getHeight() const noexcept override;
    [[nodiscard]] float getDpiScale() const noexcept override;
    [[nodiscard]] std::string getClipboardText() const override;
    void setClipboardText(const std::string& text) override;
    void setWindowIcon(std::span<const uint8_t> rgba_pixels, uint32_t width, uint32_t height) override;

    /** @brief Used by Win32WindowManager to deliver manager-level callbacks. */
    void setEventOwner(Win32WindowManager* owner);

    LRESULT handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

   private:
    static LRESULT CALLBACK staticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void createWindow(const WindowDescriptor& descriptor);
    void destroyWindow();

    HWND hwnd_ = nullptr;
    Win32WindowManager* owner_ = nullptr;
    WindowDescriptor desc_{};
    bool open_ = false;
    bool fullscreen_ = false;
    WindowMode mode_ = WindowMode::eWindowed;

    // Saved pre-fullscreen state
    DWORD saved_style_ = 0;
    RECT saved_rect_ = {};
    wchar_t pending_high_surrogate_ = 0;
};

}  // namespace vne::xwin
