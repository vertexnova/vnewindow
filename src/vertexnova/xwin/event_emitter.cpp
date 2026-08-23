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

/* One native event in, one vne::events event out (plus the Input mirror).
 *
 * The application drives consumption: call vne::events::EventManager::processEvents() once per
 * frame to dispatch the queue, then vne::events::Input::nextFrame() after the simulation step.
 * xwin calls neither for you.
 */

#include "event_emitter.h"

#include "vertexnova/xwin/input_mapping.h"
#include "vertexnova/xwin/window.h"

#include <vertexnova/events/events.h>

#include <memory>
#include <string>
#include <utility>

namespace vne::xwin {

namespace {

/** @brief Queues an event on the global manager. Ownership of the payload transfers. */
template<typename EventT, typename... ArgsT>
void push(ArgsT&&... args) {
    vne::events::EventManager::instance().pushEvent(std::make_unique<EventT>(std::forward<ArgsT>(args)...));
}

}  // namespace

vne::events::WindowId EventEmitter::windowId() const noexcept {
    return window_ ? window_->getId() : vne::events::kInvalidWindowId;
}

// ---------------------------------------------------------------------------
// Keyboard
// ---------------------------------------------------------------------------

void EventEmitter::keyDown(vne::events::KeyCode key, std::uint8_t modifiers, bool repeat) const {
    if (key == vne::events::KeyCode::eUnknown) {
        return;
    }
    if (descriptor_->enable_input) {
        vne::events::Input::updateKeyState(static_cast<int>(key), true);
    }
    if (descriptor_->enable_events) {
        if (repeat) {
            push<vne::events::KeyRepeatEvent>(key, 1U, modifiers, windowId());
        } else {
            push<vne::events::KeyPressedEvent>(key, modifiers, windowId());
        }
    }
}

void EventEmitter::keyUp(vne::events::KeyCode key, std::uint8_t modifiers) const {
    if (key == vne::events::KeyCode::eUnknown) {
        return;
    }
    if (descriptor_->enable_input) {
        vne::events::Input::updateKeyState(static_cast<int>(key), false);
    }
    if (descriptor_->enable_events) {
        push<vne::events::KeyReleasedEvent>(key, modifiers, windowId());
    }
}

void EventEmitter::textInput(const char* utf8_text) const {
    if (utf8_text == nullptr || utf8_text[0] == '\0') {
        return;
    }
    if (descriptor_->enable_events) {
        push<vne::events::TextInputEvent>(std::string(utf8_text), windowId());
    }
}

// ---------------------------------------------------------------------------
// Pointer
// ---------------------------------------------------------------------------

void EventEmitter::mouseButton(
    vne::events::MouseButton button, bool pressed, double x, double y, std::uint8_t modifiers) const {
    if (button == vne::events::MouseButton::eUnknown) {
        return;
    }
    if (descriptor_->enable_input) {
        vne::events::Input::updateMouseButtonState(static_cast<int>(button), pressed);
    }
    if (descriptor_->enable_events) {
        if (pressed) {
            push<vne::events::MouseButtonPressedEvent>(button, modifiers, x, y, windowId());
        } else {
            push<vne::events::MouseButtonReleasedEvent>(button, modifiers, x, y, windowId());
        }
    }
}

void EventEmitter::mouseMove(double x, double y, std::uint8_t modifiers) const {
    if (descriptor_->enable_input) {
        vne::events::Input::updateMousePosition(static_cast<int>(x), static_cast<int>(y));
    }
    if (descriptor_->enable_events) {
        push<vne::events::MouseMovedEvent>(x, y, modifiers, windowId());
    }
}

void EventEmitter::mouseScroll(float x_offset, float y_offset, double x, double y, std::uint8_t modifiers) const {
    if (descriptor_->enable_input) {
        vne::events::Input::updateMouseScroll(x_offset, y_offset);
    }
    if (descriptor_->enable_events) {
        push<vne::events::MouseScrolledEvent>(static_cast<double>(x_offset),
                                              static_cast<double>(y_offset),
                                              x,
                                              y,
                                              modifiers,
                                              windowId());
    }
}

// ---------------------------------------------------------------------------
// Touch
// ---------------------------------------------------------------------------

void EventEmitter::touch(std::uint32_t touch_id, double x, double y, TouchPhase phase, std::uint8_t modifiers) const {
    if (!descriptor_->enable_events) {
        return;
    }
    switch (phase) {
        case TouchPhase::eDown:
            push<vne::events::TouchPressEvent>(touch_id, x, y, modifiers, windowId());
            break;
        case TouchPhase::eUp:
            push<vne::events::TouchReleaseEvent>(touch_id, x, y, modifiers, windowId());
            break;
        case TouchPhase::eMove:
            push<vne::events::TouchMoveEvent>(touch_id, x, y, modifiers, windowId());
            break;
    }
}

// ---------------------------------------------------------------------------
// Window state
// ---------------------------------------------------------------------------

void EventEmitter::windowResize(std::uint32_t width, std::uint32_t height) const {
    if (descriptor_->enable_input) {
        vne::events::Input::updateWindowSize(static_cast<int>(width), static_cast<int>(height));
    }
    if (descriptor_->enable_events) {
        push<vne::events::WindowResizeEvent>(width, height, windowId());
    }
}

void EventEmitter::windowClose() const {
    if (descriptor_->enable_events) {
        push<vne::events::WindowCloseEvent>(windowId());
    }
}

void EventEmitter::windowFocus(bool focused) const {
    if (descriptor_->enable_events) {
        push<vne::events::WindowFocusEvent>(focused, windowId());
    }
}

void EventEmitter::windowMinimize() const {
    if (descriptor_->enable_events) {
        push<vne::events::WindowMinimizeEvent>(windowId());
    }
}

void EventEmitter::windowRestore() const {
    if (descriptor_->enable_events) {
        push<vne::events::WindowRestoreEvent>(windowId());
    }
}

void EventEmitter::windowMove(std::int32_t x, std::int32_t y) const {
    if (descriptor_->enable_events) {
        push<vne::events::WindowMoveEvent>(x, y, windowId());
    }
}

void EventEmitter::windowDpiChanged(float scale) const {
    if (descriptor_->enable_events) {
        push<vne::events::WindowDpiChangedEvent>(scale, windowId());
    }
}

void EventEmitter::windowSafeAreaChanged(float top, float left, float bottom, float right) const {
    if (descriptor_->enable_events) {
        push<vne::events::WindowSafeAreaChangedEvent>(top, left, bottom, right, windowId());
    }
}

// ---------------------------------------------------------------------------
// Application lifecycle (process-scoped)
// ---------------------------------------------------------------------------

void EventEmitter::applicationLifecycle(ApplicationLifecycle transition) {
    switch (transition) {
        case ApplicationLifecycle::ePause:
            push<vne::events::ApplicationPauseEvent>();
            break;
        case ApplicationLifecycle::eResume:
            push<vne::events::ApplicationResumeEvent>();
            break;
        case ApplicationLifecycle::eLowMemory:
            push<vne::events::ApplicationLowMemoryEvent>();
            break;
    }
}

}  // namespace vne::xwin
