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
#include "vertexnova/xwin/event_bridge_callbacks.h"

#include <vertexnova/events/types.h>

#include <string>

namespace vne::xwin {

 /*
 * Pass ANativeWindow* via WindowDescriptor::platform_data from JNI / GameActivity.
 *
 * Input delivery pattern
 * ----------------------
 * Android input is driven by the host app's JNI / GameActivity loop, NOT by
 * PollEvents() (which remains a no-op). The host should call injectTouchEvent()
 * and injectKeyEvent() from its AInputQueue / GameActivity input callback after
 * forwarding each AInputEvent to xwin. This routes events through the standard
 * vne::events bridge (EventManager queue + Input state + optional callbacks).
 */
class AndroidWindow final : public IWindow {
   public:
    AndroidWindow();
    ~AndroidWindow() override;

    void Initialize(const WindowDescriptor& descriptor) override;
    void PollEvents() override;
    void SwapBuffers() override;
    void SetTitle(const std::string& title) override;
    void SetWindowMode(WindowMode mode) override;
    [[nodiscard]] WindowMode GetWindowMode() const noexcept override;
    void SetFullscreen(bool enabled) override;
    [[nodiscard]] bool IsFullscreen() const noexcept override;
    void SetPosition(int x, int y) override;
    void GetPosition(int& x, int& y) const override;
    void Resize(uint32_t width, uint32_t height) override;
    void Minimize() override;
    void Maximize() override;
    void Restore() override;
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

    /**
     * @brief Inject a touch event from the host AInputQueue / GameActivity loop.
     * Call once per finger per AInputEvent after reading AINPUT_EVENT_TYPE_MOTION.
     */
    void injectTouchEvent(uint32_t touch_id, double x, double y, EventBridgeTouchPhase phase);

    /**
     * @brief Inject a key event from the host AInputQueue / GameActivity loop.
     * Call once per AInputEvent after reading AINPUT_EVENT_TYPE_KEY.
     */
    void injectKeyEvent(vne::events::KeyCode key, bool down, uint8_t modifiers);

    /** @brief Inject a window resize notification (call when ANativeWindow resizes). */
    void injectResizeEvent(uint32_t width, uint32_t height);

    /** @brief Provide optional granular callbacks (mirrors WindowManager::setEventBridgeCallbacks). */
    void setEventBridgeCallbacks(EventBridgeCallbacks callbacks);

   private:
    WindowDescriptor desc_{};
    bool open_ = false;
    void* native_ = nullptr;
    EventBridgeCallbacks event_bridge_callbacks_{};
};

}  // namespace vne::xwin
