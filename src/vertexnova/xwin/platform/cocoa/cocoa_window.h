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

class CocoaWindowManager_C;

class CocoaWindow_C final : public Window_I {
   public:
    CocoaWindow_C();
    ~CocoaWindow_C() override;

    void SetEventOwner(CocoaWindowManager_C* owner);

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
    void SetPosition(int x, int y) override;
    void GetPosition(int& x, int& y) const override;
    void Resize(uint32_t width, uint32_t height) override;
    void SetWindowLimits(const WindowLimits_C& limits) override;
    void SetCursor(WindowCursor_TP cursor) override;
    void Close() override;
    bool IsOpen() const override;
    void* GetNativeWindow() const override;
    WindowAPI_TP GetWindowAPI() const override;
    int GetWidth() const override;
    int GetHeight() const override;
    float GetDPIScale() const override;

    // Called from ObjC helper classes
    void handleKeyDown(vne::events::KeyCode key, uint8_t mods, bool repeat);
    void handleKeyUp(vne::events::KeyCode key, uint8_t mods);
    void handleMouseButton(vne::events::MouseButton button, bool pressed, double x, double y, uint8_t mods);
    void handleMouseMove(double x, double y, uint8_t mods);
    void handleMouseScroll(float dx, float dy);
    void handleWindowClose();
    void handleWindowResize(uint32_t w, uint32_t h);
    void handleWindowFocus(bool focused);
    void setFullscreenState(bool fs);

   private:
    void destroy_native();

    CocoaWindowManager_C* _owner = nullptr;
    WindowDescriptor_C _desc{};
    bool _open = false;
    bool _fullscreen = false;
    void* _ns_window = nullptr;
    void* _ns_view = nullptr;
    void* _ns_delegate = nullptr;

    EventBridgeCallbacks _empty_callbacks{};
};

}  // namespace vne::xwin
