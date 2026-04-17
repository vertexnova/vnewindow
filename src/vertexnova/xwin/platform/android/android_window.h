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
 *
 * Input delivery pattern
 * ----------------------
 * Android input is driven by the host app's JNI / GameActivity loop, NOT by
 * PollEvents() (which remains a no-op). The host should call InjectTouchEvent()
 * and InjectKeyEvent() from its AInputQueue / GameActivity input callback after
 * forwarding each AInputEvent to xwin. This routes events through the standard
 * vne::events bridge (EventManager queue + Input state + optional callbacks).
 * ----------------------------------------------------------------------
 */

#include "vertexnova/xwin/window.h"
#include "vertexnova/xwin/xwin_vne_event_callbacks.h"

#include <vertexnova/events/types.h>

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

    /**
     * @brief Inject a touch event from the host AInputQueue / GameActivity loop.
     * Call once per finger per AInputEvent after reading AINPUT_EVENT_TYPE_MOTION.
     */
    void InjectTouchEvent(uint32_t touch_id, double x, double y, XWinTouchPhase_TP phase);

    /**
     * @brief Inject a key event from the host AInputQueue / GameActivity loop.
     * Call once per AInputEvent after reading AINPUT_EVENT_TYPE_KEY.
     */
    void InjectKeyEvent(vne::events::KeyCode key, bool down, uint8_t modifiers);

    /** @brief Inject a window resize notification (call when ANativeWindow resizes). */
    void InjectResizeEvent(uint32_t width, uint32_t height);

    /** @brief Provide optional granular callbacks (mirrors WindowManager::SetVneEventCallbacks). */
    void SetVneEventCallbacks(XWinVneEventCallbacks_C callbacks);

   private:
    WindowDescriptor_C _desc{};
    bool _open = false;
    void* _native = nullptr;
    XWinVneEventCallbacks_C _callbacks{};
};

}  // namespace vne::xwin
