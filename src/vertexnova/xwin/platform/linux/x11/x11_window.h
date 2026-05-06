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

class X11WindowManager;

class X11Window final : public IWindow {
   public:
    X11Window();
    ~X11Window() override;

    void setEventOwner(X11WindowManager* owner);
    void setDisplay(Display* display, int screen, ::Window root, void* xcb_connection);

    void Initialize(const WindowDescriptor& descriptor) override;
    void PollEvents() override;
    void SwapBuffers() override;
    void SetTitle(const std::string& title) override;
    void SetWindowMode(WindowMode mode) override;
    [[nodiscard]] WindowMode GetWindowMode() const noexcept override;
    void SetFullscreen(bool enabled) override;
    [[nodiscard]] bool IsFullscreen() const noexcept override;
    void Minimize() override;
    void Maximize() override;
    void Restore() override;
    void SetWindowLimits(const WindowLimits& limits) override;
    void SetCursor(WindowCursor cursor) override;
    void SetPosition(int x, int y) override;
    void GetPosition(int& x, int& y) const override;
    void Resize(uint32_t width, uint32_t height) override;
    void Close() override;
    [[nodiscard]] bool IsOpen() const noexcept override;
    [[nodiscard]] void* GetNativeWindow() const noexcept override;
    [[nodiscard]] NativeWindowHandle GetNativeHandle() const noexcept override;
    [[nodiscard]] WindowAPI GetWindowAPI() const noexcept override;
    [[nodiscard]] int GetWidth() const noexcept override;
    [[nodiscard]] int GetHeight() const noexcept override;
    [[nodiscard]] float GetDPIScale() const noexcept override;
    [[nodiscard]] std::string GetClipboardText() const override;
    void SetClipboardText(const std::string& text) override;
    void SetWindowIcon(const uint8_t* rgba_pixels, uint32_t width, uint32_t height) override;

   private:
    void destroyNative();
    void sendEwmhState(bool add, Atom atom1, Atom atom2 = 0);
    void handleSelectionRequest(const XSelectionRequestEvent& req);

    Display* display_ = nullptr;
    int screen_ = 0;
    ::Window root_ = 0;
    ::Window window_ = 0;
    void* xcb_connection_ = nullptr;
    X11WindowManager* owner_ = nullptr;
    WindowDescriptor desc_{};
    bool open_ = false;
    bool fullscreen_ = false;
    Atom wm_delete_ = 0;
    Cursor blank_cursor_ = None;
    mutable std::string clipboard_text_;  ///< text we own as selection owner
    /** Physical keycodes currently held (for KeyPress repeat detection). */
    std::array<bool, 256> keycode_down_{};
};

}  // namespace vne::xwin
