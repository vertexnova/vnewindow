#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   April 2026
 *
 * Autodoc:   yes
 * Pass ANativeWindow* via WindowDescriptor_C::platform_data from JNI / GameActivity.
 * ----------------------------------------------------------------------
 */

#include "vertexnova/xwin/window.h"

#include <string>

namespace vne::xwin {

class AndroidWindow_C final : public Window_I {
   public:
    AndroidWindow_C();
    ~AndroidWindow_C() override;

    void Initialize(const WindowDescriptor_C& descriptor) override;
    void PollEvents() override;
    void SwapBuffers() override;
    void SetTitle(const std::string& title) override;
    void SetWindowMode(WindowMode_TP mode) override;
    WindowMode_TP GetWindowMode() const override;
    void SetFullscreen(bool enabled) override;
    bool IsFullscreen() const override;
    void SetPosition(int x, int y) override;
    void GetPosition(int& x, int& y) const override;
    void Resize(uint32_t width, uint32_t height) override;
    void Close() override;
    bool IsOpen() const override;
    void* GetNativeWindow() const override;
    WindowAPI_TP GetWindowAPI() const override;
    int GetWidth() const override;
    int GetHeight() const override;

   private:
    WindowDescriptor_C _desc{};
    bool _open = false;
    void* _native = nullptr;
};

}  // namespace vne::xwin
