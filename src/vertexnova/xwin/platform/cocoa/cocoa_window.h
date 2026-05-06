#pragma once
/*
 * Main-thread-only AppKit usage; caller should drive NSApplication for full event delivery.
 */
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
#include "vertexnova/xwin/event_bridge_callbacks.h"

#include <vertexnova/events/types.h>

#include <string>

namespace vne::xwin {

struct WindowInputMapping;

class CocoaWindowManager;

class CocoaWindow final : public IWindow {
   public:
    CocoaWindow();
    ~CocoaWindow() override;

    void setEventOwner(CocoaWindowManager* owner);

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
    void SetPosition(int x, int y) override;
    void GetPosition(int& x, int& y) const override;
    void Resize(uint32_t width, uint32_t height) override;
    void SetWindowLimits(const WindowLimits& limits) override;
    void SetCursor(WindowCursor cursor) override;
    void Close() override;
    [[nodiscard]] bool IsOpen() const noexcept override;
    [[nodiscard]] void* GetNativeWindow() const noexcept override;
    [[nodiscard]] NativeWindowHandle GetNativeHandle() const noexcept override;
    [[nodiscard]] WindowAPI GetWindowAPI() const noexcept override;
    [[nodiscard]] int GetWidth() const noexcept override;
    [[nodiscard]] int GetHeight() const noexcept override;
    [[nodiscard]] float GetDPIScale() const noexcept override;

    // Called from ObjC helper classes
    void handleKeyDown(vne::events::KeyCode key, uint8_t mods, bool repeat);
    void handleKeyUp(vne::events::KeyCode key, uint8_t mods);
    void handleMouseButton(vne::events::MouseButton button, bool pressed, double x, double y, uint8_t mods);
    void handleMouseMove(double x, double y, uint8_t mods);
    void handleMouseScroll(float dx, float dy);
    void handleTextInput(const char* utf8_text);
    void handleWindowClose();
    void handleWindowResize(uint32_t w, uint32_t h);
    void handleWindowFocus(bool focused);
    void setFullscreenState(bool fs);

    [[nodiscard]] const WindowInputMapping* inputMapping() const noexcept { return desc_.input_mapping; }

    [[nodiscard]] std::string GetClipboardText() const override;
    void SetClipboardText(const std::string& text) override;

   private:
    void destroyNative();

    CocoaWindowManager* owner_ = nullptr;
    WindowDescriptor desc_{};
    bool open_ = false;
    bool fullscreen_ = false;
    void* ns_window_ = nullptr;
    void* ns_view_ = nullptr;
    void* ns_delegate_ = nullptr;

    EventBridgeCallbacks empty_callbacks_{};
};

}  // namespace vne::xwin
