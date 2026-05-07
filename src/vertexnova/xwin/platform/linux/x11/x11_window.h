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
namespace {
constexpr size_t kMaxX11Keycodes = 256;
}  // namespace

class X11WindowManager;

class X11Window final : public IWindow {
   public:
    X11Window();
    ~X11Window() override;

    void setEventOwner(X11WindowManager* owner);
    void setDisplay(Display* display, int screen, ::Window root, void* xcb_connection);

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
    [[nodiscard]] WindowPosition getPosition() const override;
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
    std::array<bool, kMaxX11Keycodes> keycode_down_{};
};

}  // namespace vne::xwin
