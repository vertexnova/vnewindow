#pragma once
/* Internal bridge from native window paths to vne::events and optional callbacks. */
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

#include "vertexnova/xwin/event_bridge_callbacks.h"
#include "vertexnova/xwin/window_descriptor.h"

#include <vertexnova/events/types.h>

namespace vne::xwin {

class Window_I;

void eventBridgeKeyDown(Window_I* window,
                        const WindowDescriptor_C& descriptor,
                        const EventBridgeCallbacks& callbacks,
                        vne::events::KeyCode key,
                        uint8_t modifiers,
                        bool repeat);

void eventBridgeKeyUp(Window_I* window,
                      const WindowDescriptor_C& descriptor,
                      const EventBridgeCallbacks& callbacks,
                      vne::events::KeyCode key,
                      uint8_t modifiers);

void eventBridgeMouseButton(Window_I* window,
                            const WindowDescriptor_C& descriptor,
                            const EventBridgeCallbacks& callbacks,
                            vne::events::MouseButton button,
                            bool pressed,
                            double x,
                            double y,
                            uint8_t modifiers);

void eventBridgeMouseMove(Window_I* window,
                          const WindowDescriptor_C& descriptor,
                          const EventBridgeCallbacks& callbacks,
                          double x,
                          double y,
                          uint8_t modifiers);

void eventBridgeMouseScroll(Window_I* window,
                            const WindowDescriptor_C& descriptor,
                            const EventBridgeCallbacks& callbacks,
                            float x_offset,
                            float y_offset);

void eventBridgeTouch(Window_I* window,
                      const WindowDescriptor_C& descriptor,
                      const EventBridgeCallbacks& callbacks,
                      uint32_t touch_id,
                      double x,
                      double y,
                      EventBridgeTouchPhase phase);

void eventBridgeWindowResize(Window_I* window,
                             const WindowDescriptor_C& descriptor,
                             const EventBridgeCallbacks& callbacks,
                             uint32_t width,
                             uint32_t height);

void eventBridgeWindowClose(Window_I* window,
                            const WindowDescriptor_C& descriptor,
                            const EventBridgeCallbacks& callbacks);

void eventBridgeWindowFocus(Window_I* window,
                            const WindowDescriptor_C& descriptor,
                            const EventBridgeCallbacks& callbacks,
                            bool focused);

}  // namespace vne::xwin
