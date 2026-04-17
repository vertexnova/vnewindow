#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   April 2026
 *
 * Autodoc:   yes
 * UIKit views are main-thread only. Host app typically owns UIWindow; this view is a surface host.
 * ----------------------------------------------------------------------
 */

#include "vertexnova/xwin/window.h"
#include "vertexnova/xwin/event_bridge_callbacks.h"

#include <vertexnova/events/types.h>

#include <string>

namespace vne::xwin {

class UIKitWindowManager_C;

class UIKitWindow_C final : public Window_I {
   public:
    UIKitWindow_C();
    ~UIKitWindow_C() override;

    void SetEventOwner(UIKitWindowManager_C* owner);

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

    // Called from VneXWinUIView
    void handleTouch(uint32_t touch_id, double x, double y, EventBridgeTouchPhase phase);

   private:
    void destroy_native();

    UIKitWindowManager_C* _owner = nullptr;
    WindowDescriptor_C _desc{};
    bool _open = false;
    void* _ui_view = nullptr;

    EventBridgeCallbacks _empty_callbacks{};
};

}  // namespace vne::xwin
