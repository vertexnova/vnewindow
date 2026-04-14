/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Bridges native xwin paths to vne::events (Input + EventManager) and optional
 * per-window callbacks. Application should call vne::events::Input::nextFrame()
 * once per frame after game logic; call EventManager::processEvents() when
 * draining the queue is desired.
 * ----------------------------------------------------------------------
 */

#include "xwin_vne_events_bridge.h"

#include "vertexnova/xwin/window.h"

#include <vertexnova/events/events.h>

#include <memory>

namespace vne::xwin {

namespace {

void push_key_pressed(vne::events::KeyCode key, uint8_t modifiers) {
    vne::events::EventManager::instance().pushEvent(std::make_unique<vne::events::KeyPressedEvent>(key, modifiers));
}

void push_key_repeat(vne::events::KeyCode key, uint8_t modifiers) {
    vne::events::EventManager::instance().pushEvent(
        std::make_unique<vne::events::KeyRepeatEvent>(key, 1U));
    (void)modifiers;
}

void push_key_released(vne::events::KeyCode key, uint8_t modifiers) {
    vne::events::EventManager::instance().pushEvent(std::make_unique<vne::events::KeyReleasedEvent>(key, modifiers));
}

}  // namespace

void xwin_vne_bridge_key_down(Window_I* window,
                              const WindowDescriptor_C& desc,
                              const XWinVneEventCallbacks_C& callbacks,
                              vne::events::KeyCode key,
                              uint8_t modifiers,
                              bool repeat) {
    if (key == vne::events::KeyCode::eUnknown) {
        return;
    }
    const int ik = static_cast<int>(key);
    if (desc.enable_input) {
        vne::events::Input::updateKeyState(ik, true);
    }
    if (desc.enable_events) {
        if (repeat) {
            push_key_repeat(key, modifiers);
        } else {
            push_key_pressed(key, modifiers);
        }
    }
    if (callbacks.on_key_down) {
        callbacks.on_key_down(window, key, modifiers, repeat);
    }
}

void xwin_vne_bridge_key_up(Window_I* window,
                            const WindowDescriptor_C& desc,
                            const XWinVneEventCallbacks_C& callbacks,
                            vne::events::KeyCode key,
                            uint8_t modifiers) {
    if (key == vne::events::KeyCode::eUnknown) {
        return;
    }
    const int ik = static_cast<int>(key);
    if (desc.enable_input) {
        vne::events::Input::updateKeyState(ik, false);
    }
    if (desc.enable_events) {
        push_key_released(key, modifiers);
    }
    if (callbacks.on_key_up) {
        callbacks.on_key_up(window, key, modifiers);
    }
}

void xwin_vne_bridge_mouse_button(Window_I* window,
                                  const WindowDescriptor_C& desc,
                                  const XWinVneEventCallbacks_C& callbacks,
                                  vne::events::MouseButton button,
                                  bool pressed,
                                  double x,
                                  double y,
                                  uint8_t modifiers) {
    const int ib = static_cast<int>(button);
    if (desc.enable_input) {
        vne::events::Input::updateMouseButtonState(ib, pressed);
    }
    if (desc.enable_events) {
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

void xwin_vne_bridge_mouse_move(Window_I* window,
                                const WindowDescriptor_C& desc,
                                const XWinVneEventCallbacks_C& callbacks,
                                double x,
                                double y,
                                uint8_t modifiers) {
    if (desc.enable_input) {
        vne::events::Input::updateMousePosition(static_cast<int>(x), static_cast<int>(y));
    }
    if (desc.enable_events) {
        vne::events::EventManager::instance().pushEvent(std::make_unique<vne::events::MouseMovedEvent>(x, y, modifiers));
    }
    if (callbacks.on_mouse_move) {
        callbacks.on_mouse_move(window, x, y, modifiers);
    }
}

void xwin_vne_bridge_mouse_scroll(Window_I* window,
                                  const WindowDescriptor_C& desc,
                                  const XWinVneEventCallbacks_C& callbacks,
                                  float x_offset,
                                  float y_offset) {
    if (desc.enable_input) {
        vne::events::Input::updateMouseScroll(x_offset, y_offset);
    }
    if (desc.enable_events) {
        vne::events::EventManager::instance().pushEvent(
            std::make_unique<vne::events::MouseScrolledEvent>(static_cast<double>(x_offset),
                                                             static_cast<double>(y_offset)));
    }
    if (callbacks.on_mouse_scroll) {
        callbacks.on_mouse_scroll(window, x_offset, y_offset);
    }
}

void xwin_vne_bridge_touch(Window_I* window,
                           const WindowDescriptor_C& desc,
                           const XWinVneEventCallbacks_C& callbacks,
                           uint32_t touch_id,
                           double x,
                           double y,
                           XWinTouchPhase_TP phase) {
    if (!desc.enable_events && !callbacks.on_touch) {
        return;
    }
    if (desc.enable_events) {
        switch (phase) {
            case XWinTouchPhase_TP::Down:
                vne::events::EventManager::instance().pushEvent(
                    std::make_unique<vne::events::TouchPressEvent>(touch_id, x, y));
                break;
            case XWinTouchPhase_TP::Up:
                vne::events::EventManager::instance().pushEvent(
                    std::make_unique<vne::events::TouchReleaseEvent>(touch_id, x, y));
                break;
            case XWinTouchPhase_TP::Move:
                vne::events::EventManager::instance().pushEvent(
                    std::make_unique<vne::events::TouchMoveEvent>(touch_id, x, y));
                break;
        }
    }
    if (callbacks.on_touch) {
        callbacks.on_touch(window, touch_id, x, y, phase);
    }
}

void xwin_vne_bridge_window_resize(Window_I* window,
                                   const WindowDescriptor_C& desc,
                                   const XWinVneEventCallbacks_C& callbacks,
                                   uint32_t width,
                                   uint32_t height) {
    if (desc.enable_input) {
        vne::events::Input::updateWindowSize(static_cast<int>(width), static_cast<int>(height));
    }
    if (desc.enable_events) {
        vne::events::EventManager::instance().pushEvent(
            std::make_unique<vne::events::WindowResizeEvent>(width, height));
    }
    (void)window;
    (void)callbacks;
}

void xwin_vne_bridge_window_close(Window_I* window,
                                  const WindowDescriptor_C& desc,
                                  const XWinVneEventCallbacks_C& callbacks) {
    if (desc.enable_events) {
        vne::events::EventManager::instance().pushEvent(std::make_unique<vne::events::WindowCloseEvent>());
    }
    (void)window;
    (void)callbacks;
}

void xwin_vne_bridge_window_focus(Window_I* window,
                                  const WindowDescriptor_C& desc,
                                  const XWinVneEventCallbacks_C& callbacks,
                                  bool focused) {
    (void)desc;
    if (callbacks.on_window_focus) {
        callbacks.on_window_focus(window, focused);
    }
}

}  // namespace vne::xwin
