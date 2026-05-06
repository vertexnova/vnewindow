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

/* Bridges native xwin paths to vne::events (Input + EventManager) and optional
 * per-window callbacks. Application should call vne::events::Input::nextFrame()
 * once per frame after game logic; call EventManager::processEvents() when
 * draining the queue is desired.
 */

#include "event_bridge.h"

#include "vertexnova/xwin/window.h"

#include <vertexnova/events/events.h>
#include <vertexnova/events/window_event.h>

namespace vne::xwin {

namespace {

void pushKeyPressed(vne::events::KeyCode key, uint8_t modifiers) {
    vne::events::EventManager::instance().pushEvent(std::make_unique<vne::events::KeyPressedEvent>(key, modifiers));
}

void pushKeyRepeat(vne::events::KeyCode key, uint8_t modifiers) {
    vne::events::EventManager::instance().pushEvent(std::make_unique<vne::events::KeyRepeatEvent>(key, 1U));
    (void)modifiers;
}

void pushKeyReleased(vne::events::KeyCode key, uint8_t modifiers) {
    vne::events::EventManager::instance().pushEvent(std::make_unique<vne::events::KeyReleasedEvent>(key, modifiers));
}

}  // namespace

void eventBridgeKeyDown(IWindow* window,
                        const WindowDescriptor& descriptor,
                        const EventBridgeCallbacks& callbacks,
                        vne::events::KeyCode key,
                        uint8_t modifiers,
                        bool repeat) {
    if (key == vne::events::KeyCode::eUnknown) {
        return;
    }
    const int ik = static_cast<int>(key);
    if (descriptor.enable_input) {
        vne::events::Input::updateKeyState(ik, true);
    }
    if (descriptor.enable_events) {
        if (repeat) {
            pushKeyRepeat(key, modifiers);
        } else {
            pushKeyPressed(key, modifiers);
        }
    }
    if (callbacks.on_key_down) {
        callbacks.on_key_down(window, key, modifiers, repeat);
    }
}

void eventBridgeKeyUp(IWindow* window,
                      const WindowDescriptor& descriptor,
                      const EventBridgeCallbacks& callbacks,
                      vne::events::KeyCode key,
                      uint8_t modifiers) {
    if (key == vne::events::KeyCode::eUnknown) {
        return;
    }
    const int ik = static_cast<int>(key);
    if (descriptor.enable_input) {
        vne::events::Input::updateKeyState(ik, false);
    }
    if (descriptor.enable_events) {
        pushKeyReleased(key, modifiers);
    }
    if (callbacks.on_key_up) {
        callbacks.on_key_up(window, key, modifiers);
    }
}

void eventBridgeMouseButton(IWindow* window,
                            const WindowDescriptor& descriptor,
                            const EventBridgeCallbacks& callbacks,
                            vne::events::MouseButton button,
                            bool pressed,
                            double x,
                            double y,
                            uint8_t modifiers) {
    const int ib = static_cast<int>(button);
    if (descriptor.enable_input) {
        vne::events::Input::updateMouseButtonState(ib, pressed);
    }
    if (descriptor.enable_events) {
        if (pressed) {
            vne::events::EventManager::instance().pushEvent(
                std::make_unique<vne::events::MouseButtonPressedEvent>(button, modifiers, x, y));
        } else {
            vne::events::EventManager::instance().pushEvent(
                std::make_unique<vne::events::MouseButtonReleasedEvent>(button, modifiers, x, y));
        }
    }
    if (callbacks.on_mouse_button) {
        callbacks.on_mouse_button(window, button, pressed, x, y, modifiers);
    }
}

void eventBridgeMouseMove(IWindow* window,
                          const WindowDescriptor& descriptor,
                          const EventBridgeCallbacks& callbacks,
                          double x,
                          double y,
                          uint8_t modifiers) {
    if (descriptor.enable_input) {
        vne::events::Input::updateMousePosition(static_cast<int>(x), static_cast<int>(y));
    }
    if (descriptor.enable_events) {
        vne::events::EventManager::instance().pushEvent(
            std::make_unique<vne::events::MouseMovedEvent>(x, y, modifiers));
    }
    if (callbacks.on_mouse_move) {
        callbacks.on_mouse_move(window, x, y, modifiers);
    }
}

void eventBridgeMouseScroll(IWindow* window,
                            const WindowDescriptor& descriptor,
                            const EventBridgeCallbacks& callbacks,
                            float x_offset,
                            float y_offset) {
    if (descriptor.enable_input) {
        vne::events::Input::updateMouseScroll(x_offset, y_offset);
    }
    if (descriptor.enable_events) {
        vne::events::EventManager::instance().pushEvent(
            std::make_unique<vne::events::MouseScrolledEvent>(static_cast<double>(x_offset),
                                                              static_cast<double>(y_offset)));
    }
    if (callbacks.on_mouse_scroll) {
        callbacks.on_mouse_scroll(window, x_offset, y_offset);
    }
}

void eventBridgeTouch(IWindow* window,
                      const WindowDescriptor& descriptor,
                      const EventBridgeCallbacks& callbacks,
                      uint32_t touch_id,
                      double x,
                      double y,
                      EventBridgeTouchPhase phase) {
    if (!descriptor.enable_events && !callbacks.on_touch) {
        return;
    }
    if (descriptor.enable_events) {
        switch (phase) {
            case EventBridgeTouchPhase::eDown:
                vne::events::EventManager::instance().pushEvent(
                    std::make_unique<vne::events::TouchPressEvent>(touch_id, x, y));
                break;
            case EventBridgeTouchPhase::eUp:
                vne::events::EventManager::instance().pushEvent(
                    std::make_unique<vne::events::TouchReleaseEvent>(touch_id, x, y));
                break;
            case EventBridgeTouchPhase::eMove:
                vne::events::EventManager::instance().pushEvent(
                    std::make_unique<vne::events::TouchMoveEvent>(touch_id, x, y));
                break;
        }
    }
    if (callbacks.on_touch) {
        callbacks.on_touch(window, touch_id, x, y, phase);
    }
}

void eventBridgeWindowResize(IWindow* window,
                             const WindowDescriptor& descriptor,
                             const EventBridgeCallbacks& callbacks,
                             uint32_t width,
                             uint32_t height) {
    if (descriptor.enable_input) {
        vne::events::Input::updateWindowSize(static_cast<int>(width), static_cast<int>(height));
    }
    if (descriptor.enable_events) {
        vne::events::EventManager::instance().pushEvent(
            std::make_unique<vne::events::WindowResizeEvent>(width, height));
    }
    (void)window;
    (void)callbacks;
}

void eventBridgeWindowClose(IWindow* window,
                            const WindowDescriptor& descriptor,
                            const EventBridgeCallbacks& callbacks) {
    if (descriptor.enable_events) {
        vne::events::EventManager::instance().pushEvent(std::make_unique<vne::events::WindowCloseEvent>());
    }
    (void)window;
    (void)callbacks;
}

void eventBridgeWindowFocus(IWindow* window,
                            const WindowDescriptor& descriptor,
                            const EventBridgeCallbacks& callbacks,
                            bool focused) {
    if (descriptor.enable_events) {
        vne::events::EventManager::instance().pushEvent(std::make_unique<vne::events::WindowFocusEvent>(focused));
    }
    if (callbacks.on_window_focus) {
        callbacks.on_window_focus(window, focused);
    }
}

void eventBridgeTextInput(IWindow* window,
                          const WindowDescriptor& descriptor,
                          const EventBridgeCallbacks& callbacks,
                          const char* utf8_text) {
    if (!utf8_text || utf8_text[0] == '\0') {
        return;
    }
    (void)descriptor;
    if (callbacks.on_text_input) {
        callbacks.on_text_input(window, utf8_text);
    }
}

}  // namespace vne::xwin
