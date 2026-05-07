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
 * pollEvents() (which remains a no-op). The host should call injectTouchEvent()
 * and injectKeyEvent() from its AInputQueue / GameActivity input callback after
 * forwarding each AInputEvent to xwin. This routes events through the standard
 * vne::events bridge (EventManager queue + Input state + optional callbacks).
 */
class AndroidWindow final : public IWindow {
   public:
    AndroidWindow();
    ~AndroidWindow() override;

    void initialize(const WindowDescriptor& descriptor) override;
    void pollEvents() override;
    void swapBuffers() override;
    void setTitle(const std::string& title) override;
    void setWindowMode(WindowMode mode) override;
    [[nodiscard]] WindowMode getWindowMode() const noexcept override;
    void setFullscreen(bool enabled) override;
    [[nodiscard]] bool isFullscreen() const noexcept override;
    void setPosition(int x, int y) override;
    [[nodiscard]] WindowPosition getPosition() const override;
    void resize(uint32_t width, uint32_t height) override;
    void minimize() override;
    void maximize() override;
    void restore() override;
    void setWindowLimits(const WindowLimits& limits) override;
    void setCursor(WindowCursor cursor) override;
    void close() override;
    [[nodiscard]] bool isOpen() const noexcept override;
    [[nodiscard]] NativeWindowHandle getNativeHandle() const noexcept override;
    [[nodiscard]] WindowAPI getWindowAPI() const noexcept override;
    [[nodiscard]] int getWidth() const noexcept override;
    [[nodiscard]] int getHeight() const noexcept override;
    [[nodiscard]] float getDpiScale() const noexcept override;

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
