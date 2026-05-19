#pragma once
/*
 * UIKit views are main-thread only. UIKitWindow owns a UIWindow + content view
 * when no host window is supplied; hosts can adopt handle.ui_window instead.
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

#include <string>

namespace vne::xwin {

class UIKitWindowManager;

class UIKitWindow final : public IWindow {
   public:
    UIKitWindow();
    ~UIKitWindow() override;

    void setEventOwner(UIKitWindowManager* owner);

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

    // Called from VneXWinUIView
    void handleTouch(uint32_t touch_id, double x, double y, EventBridgeTouchPhase phase);

    [[nodiscard]] const WindowInputMapping* inputMapping() const noexcept { return desc_.input_mapping.get(); }

   private:
    void destroyNative();

    UIKitWindowManager* owner_ = nullptr;
    WindowDescriptor desc_{};
    bool open_ = false;
    void* ui_view_ = nullptr;
    void* ui_window_ = nullptr;

    EventBridgeCallbacks empty_callbacks_{};
};

}  // namespace vne::xwin
