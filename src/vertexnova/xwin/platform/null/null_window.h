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

namespace vne::xwin {

class NullWindow_C final : public Window_I {
   public:
    NullWindow_C() = default;
    ~NullWindow_C() override = default;

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
    NativeWindowHandle_C GetNativeHandle() const override;
    WindowAPI_TP GetWindowAPI() const override;
    int GetWidth() const override;
    int GetHeight() const override;

   private:
    WindowDescriptor_C _descriptor{};
    bool _open = false;
};

}  // namespace vne::xwin
