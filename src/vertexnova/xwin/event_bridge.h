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

/** @file event_bridge.h Internal C++ bridge from platform windows to vne::events. */

#include "vertexnova/xwin/event_bridge_callbacks.h"
#include "vertexnova/xwin/window_descriptor.h"

#include <vertexnova/events/types.h>

namespace vne::xwin {

class IWindow;

void eventBridgeKeyDown(IWindow* window,
                        const WindowDescriptor& descriptor,
                        const EventBridgeCallbacks& callbacks,
                        vne::events::KeyCode key,
                        uint8_t modifiers,
                        bool repeat);

void eventBridgeKeyUp(IWindow* window,
                      const WindowDescriptor& descriptor,
                      const EventBridgeCallbacks& callbacks,
                      vne::events::KeyCode key,
                      uint8_t modifiers);

void eventBridgeMouseButton(IWindow* window,
                            const WindowDescriptor& descriptor,
                            const EventBridgeCallbacks& callbacks,
                            vne::events::MouseButton button,
                            bool pressed,
                            double x,
                            double y,
                            uint8_t modifiers);

void eventBridgeMouseMove(IWindow* window,
                          const WindowDescriptor& descriptor,
                          const EventBridgeCallbacks& callbacks,
                          double x,
                          double y,
                          uint8_t modifiers);

void eventBridgeMouseScroll(IWindow* window,
                            const WindowDescriptor& descriptor,
                            const EventBridgeCallbacks& callbacks,
                            float x_offset,
                            float y_offset);

void eventBridgeTouch(IWindow* window,
                      const WindowDescriptor& descriptor,
                      const EventBridgeCallbacks& callbacks,
                      uint32_t touch_id,
                      double x,
                      double y,
                      EventBridgeTouchPhase phase);

void eventBridgeWindowResize(IWindow* window,
                             const WindowDescriptor& descriptor,
                             const EventBridgeCallbacks& callbacks,
                             uint32_t width,
                             uint32_t height);

void eventBridgeWindowClose(IWindow* window,
                            const WindowDescriptor& descriptor,
                            const EventBridgeCallbacks& callbacks);

void eventBridgeWindowFocus(IWindow* window,
                            const WindowDescriptor& descriptor,
                            const EventBridgeCallbacks& callbacks,
                            bool focused);
void eventBridgeTextInput(IWindow* window,
                          const WindowDescriptor& descriptor,
                          const EventBridgeCallbacks& callbacks,
                          const char* utf8_text);

}  // namespace vne::xwin
