#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Internal bridge from native window paths to vne::events and optional callbacks.
 * ----------------------------------------------------------------------
 */

#include "vertexnova/xwin/window_descriptor.h"
#include "vertexnova/xwin/xwin_vne_event_callbacks.h"

#include <vertexnova/events/types.h>

namespace vne::xwin {

class Window_I;

void xwinVneBridgeKeyDown(Window_I* window,
                          const WindowDescriptor_C& desc,
                          const XWinVneEventCallbacks_C& callbacks,
                          vne::events::KeyCode key,
                          uint8_t modifiers,
                          bool repeat);

void xwinVneBridgeKeyUp(Window_I* window,
                        const WindowDescriptor_C& desc,
                        const XWinVneEventCallbacks_C& callbacks,
                        vne::events::KeyCode key,
                        uint8_t modifiers);

void xwinVneBridgeMouseButton(Window_I* window,
                              const WindowDescriptor_C& desc,
                              const XWinVneEventCallbacks_C& callbacks,
                              vne::events::MouseButton button,
                              bool pressed,
                              double x,
                              double y,
                              uint8_t modifiers);

void xwinVneBridgeMouseMove(Window_I* window,
                            const WindowDescriptor_C& desc,
                            const XWinVneEventCallbacks_C& callbacks,
                            double x,
                            double y,
                            uint8_t modifiers);

void xwinVneBridgeMouseScroll(Window_I* window,
                              const WindowDescriptor_C& desc,
                              const XWinVneEventCallbacks_C& callbacks,
                              float x_offset,
                              float y_offset);

void xwinVneBridgeTouch(Window_I* window,
                        const WindowDescriptor_C& desc,
                        const XWinVneEventCallbacks_C& callbacks,
                        uint32_t touch_id,
                        double x,
                        double y,
                        XWinTouchPhase_TP phase);

void xwinVneBridgeWindowResize(Window_I* window,
                               const WindowDescriptor_C& desc,
                               const XWinVneEventCallbacks_C& callbacks,
                               uint32_t width,
                               uint32_t height);

void xwinVneBridgeWindowClose(Window_I* window,
                              const WindowDescriptor_C& desc,
                              const XWinVneEventCallbacks_C& callbacks);

void xwinVneBridgeWindowFocus(Window_I* window,
                              const WindowDescriptor_C& desc,
                              const XWinVneEventCallbacks_C& callbacks,
                              bool focused);

}  // namespace vne::xwin
