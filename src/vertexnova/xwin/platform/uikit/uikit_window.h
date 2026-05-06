#pragma once
/*
 * UIKit views are main-thread only. Host app typically owns UIWindow; this view is a surface host.
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

class UIKitWindowManager_C;

class UIKitWindow_C final : public IWindow {
   public:
    UIKitWindow_C();
    ~UIKitWindow_C() override;

    void SetEventOwner(UIKitWindowManager_C* owner);

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

    // Called from VneXWinUIView
    void handleTouch(uint32_t touch_id, double x, double y, EventBridgeTouchPhase phase);
    void handleMouseButton(vne::events::MouseButton button, bool pressed, double x, double y, uint8_t mods);
    void handleMouseMove(double x, double y, uint8_t mods);

    const WindowInputMapping* input_mapping() const { return desc_.input_mapping; }

   private:
    void destroy_native();

    UIKitWindowManager_C* owner_ = nullptr;
    WindowDescriptor desc_{};
    bool open_ = false;
    void* ui_view_ = nullptr;

    EventBridgeCallbacks empty_callbacks_{};
};

}  // namespace vne::xwin
