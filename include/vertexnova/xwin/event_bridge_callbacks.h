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

/** @file event_bridge_callbacks.h Optional per-window hooks after vne::events updates (key/mouse/touch/text). */

#include <vertexnova/events/types.h>

#include <cstdint>
#include <functional>

namespace vne::xwin {

class IWindow;

enum class EventBridgeTouchPhase : std::uint8_t { eDown = 0, eUp = 1, eMove = 2 };

/**
 * @brief Optional callbacks invoked after vne::events Input/EventManager updates
 *        (when enabled on the window descriptor), one concern per slot.
 */
struct EventBridgeCallbacks {
    std::function<void(IWindow*, vne::events::KeyCode key, uint8_t modifiers, bool repeat)> onKeyDown;
    std::function<void(IWindow*, vne::events::KeyCode key, uint8_t modifiers)> onKeyUp;
    std::function<void(IWindow*, vne::events::MouseButton button, bool pressed, double x, double y, uint8_t modifiers)>
        onMouseButton;
    std::function<void(IWindow*, double x, double y, uint8_t modifiers)> onMouseMove;
    std::function<void(IWindow*, float xOffset, float yOffset)> onMouseScroll;
    std::function<void(IWindow*, uint32_t touch_id, double x, double y, EventBridgeTouchPhase phase)> onTouch;
    std::function<void(IWindow*, bool focused)> onWindowFocus;
    std::function<void(IWindow*, const char*)> onTextInput;
};

}  // namespace vne::xwin
