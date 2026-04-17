#pragma once
/*
 * Optional per-channel callbacks for native window input routed alongside
 * vne::events. All slots are optional; unset functions are not invoked.
 */
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

#include <vertexnova/events/types.h>

#include <cstdint>
#include <functional>

namespace vne::xwin {

class Window_I;

enum class EventBridgeTouchPhase : std::uint8_t { eDown = 0, eUp = 1, eMove = 2 };

/**
 * @brief Optional callbacks invoked after vne::events Input/EventManager updates
 *        (when enabled on the window descriptor), one concern per slot.
 */
struct EventBridgeCallbacks {
    std::function<void(Window_I*, vne::events::KeyCode key, uint8_t modifiers, bool repeat)> onKeyDown;
    std::function<void(Window_I*, vne::events::KeyCode key, uint8_t modifiers)> onKeyUp;
    std::function<void(Window_I*, vne::events::MouseButton button, bool pressed, double x, double y, uint8_t modifiers)>
        onMouseButton;
    std::function<void(Window_I*, double x, double y, uint8_t modifiers)> onMouseMove;
    std::function<void(Window_I*, float x_offset, float y_offset)> onMouseScroll;
    std::function<void(Window_I*, uint32_t touch_id, double x, double y, EventBridgeTouchPhase phase)> onTouch;
    std::function<void(Window_I*, bool focused)> onWindowFocus;
};

}  // namespace vne::xwin
