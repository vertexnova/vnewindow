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
#include "event_emitter.h"

#include <vertexnova/events/types.h>

#include <string>

namespace vne::xwin {

/*
 * Pass ANativeWindow* via WindowDescriptor::platform_data from JNI / GameActivity.
 *
 * Input delivery pattern
 * ----------------------
 * Android input is driven by the host app's JNI / GameActivity loop, NOT by
 * its own event loop. The host should call injectTouchEvent()
 * and injectKeyEvent() from its AInputQueue / GameActivity input callback after
 * forwarding each AInputEvent to xwin. This routes events through the standard
 * vne::events bridge (EventManager queue + Input state + optional callbacks).
 */
class AndroidWindow final : public IWindow {
   public:
    AndroidWindow();
    ~AndroidWindow() override;

    void initialize(const WindowDescriptor& descriptor) override;
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
    [[nodiscard]] vne::events::WindowId getId() const noexcept override { return id_; }
    [[nodiscard]] NativeWindowHandle getNativeHandle() const noexcept override;
    [[nodiscard]] WindowAPI getWindowAPI() const noexcept override;
    [[nodiscard]] int getWidth() const noexcept override;
    [[nodiscard]] int getHeight() const noexcept override;
    [[nodiscard]] float getDpiScale() const noexcept override;

    /**
     * @brief Inject a touch event from the host AInputQueue / GameActivity loop.
     * Call once per finger per AInputEvent after reading AINPUT_EVENT_TYPE_MOTION.
     */
    void injectTouchEvent(uint32_t touch_id, double x, double y, TouchPhase phase, uint8_t modifiers = 0);

    /**
     * @brief Inject a key event from the host AInputQueue / GameActivity loop.
     * Call once per AInputEvent after reading AINPUT_EVENT_TYPE_KEY.
     * @param repeat Match Android KeyEvent: `getRepeatCount() > 0` for key-repeat.
     */
    void injectKeyEvent(vne::events::KeyCode key, bool down, uint8_t modifiers, bool repeat = false);

    /** @brief Inject committed UTF-8 text (call from the IME / InputConnection path). */
    void injectTextInput(const char* utf8_text);

    /** @brief Inject a window resize notification (call when ANativeWindow resizes). */
    void injectResizeEvent(uint32_t width, uint32_t height);

    /** @brief Inject a focus change (call on APP_CMD_GAINED_FOCUS / LOST_FOCUS). */
    void injectFocusEvent(bool focused);

    /** @brief Inject a density change (call on APP_CMD_CONFIG_CHANGED). */
    void injectDpiChanged(float scale);

    /** @brief Inject safe-area insets, in logical pixels, from WindowInsets. */
    void injectSafeAreaChanged(float top, float left, float bottom, float right);

   private:
    const vne::events::WindowId id_ = IWindow::nextId();
    WindowDescriptor desc_{};
    bool open_ = false;
    void* native_ = nullptr;

    // Declared last: binds to desc_ by reference.
    EventEmitter events_{this, desc_};
};

}  // namespace vne::xwin
