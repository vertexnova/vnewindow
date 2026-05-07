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

class CocoaWindowManager;

class CocoaWindow final : public IWindow {
   public:
    CocoaWindow();
    ~CocoaWindow() override;

    void setEventOwner(CocoaWindowManager* owner);

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
    void setPosition(int x, int y) override;
    [[nodiscard]] WindowPosition getPosition() const override;
    void resize(uint32_t width, uint32_t height) override;
    void setWindowLimits(const WindowLimits& limits) override;
    void setCursor(WindowCursor cursor) override;
    void close() override;
    [[nodiscard]] bool isOpen() const noexcept override;
    [[nodiscard]] NativeWindowHandle getNativeHandle() const noexcept override;
    [[nodiscard]] WindowAPI getWindowAPI() const noexcept override;
    [[nodiscard]] int getWidth() const noexcept override;
    [[nodiscard]] int getHeight() const noexcept override;
    [[nodiscard]] float getDpiScale() const noexcept override;

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

    [[nodiscard]] const WindowInputMapping* inputMapping() const noexcept { return desc_.input_mapping.get(); }

    [[nodiscard]] std::string getClipboardText() const override;
    void setClipboardText(const std::string& text) override;

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
