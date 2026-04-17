#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Optional per-channel callbacks for native window input routed alongside
 * vne::events. All slots are optional; unset functions are not invoked.
 * ----------------------------------------------------------------------
 */

#include <vertexnova/events/types.h>

#include <cstdint>
#include <functional>

namespace vne::xwin {

class Window_I;

enum class EventBridgeTouchPhase_C : std::uint8_t {
    eDown = 0,
    eUp = 1,
    eMove = 2
};

/**
 * @brief Optional callbacks invoked after vne::events Input/EventManager updates
 *        (when enabled on the window descriptor), one concern per slot.
 */
struct EventBridgeCallbacks_C {
    std::function<void(Window_I*, vne::events::KeyCode key, uint8_t modifiers, bool repeat)> on_key_down;
    std::function<void(Window_I*, vne::events::KeyCode key, uint8_t modifiers)> on_key_up;
    std::function<void(Window_I*, vne::events::MouseButton button, bool pressed, double x, double y, uint8_t modifiers)>
        on_mouse_button;
    std::function<void(Window_I*, double x, double y, uint8_t modifiers)> on_mouse_move;
    std::function<void(Window_I*, float x_offset, float y_offset)> on_mouse_scroll;
    std::function<void(Window_I*, uint32_t touch_id, double x, double y, EventBridgeTouchPhase_C phase)> on_touch;
    std::function<void(Window_I*, bool focused)> on_window_focus;
};

}  // namespace vne::xwin
