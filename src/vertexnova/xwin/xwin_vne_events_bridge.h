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

void xwin_vne_bridge_key_down(Window_I* window,
                              const WindowDescriptor_C& desc,
                              const XWinVneEventCallbacks_C& callbacks,
                              vne::events::KeyCode key,
                              uint8_t modifiers,
                              bool repeat);

void xwin_vne_bridge_key_up(Window_I* window,
                            const WindowDescriptor_C& desc,
                            const XWinVneEventCallbacks_C& callbacks,
                            vne::events::KeyCode key,
                            uint8_t modifiers);

void xwin_vne_bridge_mouse_button(Window_I* window,
                                  const WindowDescriptor_C& desc,
                                  const XWinVneEventCallbacks_C& callbacks,
                                  vne::events::MouseButton button,
                                  bool pressed,
                                  double x,
                                  double y,
                                  uint8_t modifiers);

void xwin_vne_bridge_mouse_move(Window_I* window,
                                const WindowDescriptor_C& desc,
                                const XWinVneEventCallbacks_C& callbacks,
                                double x,
                                double y,
                                uint8_t modifiers);

void xwin_vne_bridge_mouse_scroll(Window_I* window,
                                  const WindowDescriptor_C& desc,
                                  const XWinVneEventCallbacks_C& callbacks,
                                  float x_offset,
                                  float y_offset);

void xwin_vne_bridge_touch(Window_I* window,
                           const WindowDescriptor_C& desc,
                           const XWinVneEventCallbacks_C& callbacks,
                           uint32_t touch_id,
                           double x,
                           double y,
                           XWinTouchPhase_TP phase);

void xwin_vne_bridge_window_resize(Window_I* window,
                                   const WindowDescriptor_C& desc,
                                   const XWinVneEventCallbacks_C& callbacks,
                                   uint32_t width,
                                   uint32_t height);

void xwin_vne_bridge_window_close(Window_I* window,
                                  const WindowDescriptor_C& desc,
                                  const XWinVneEventCallbacks_C& callbacks);

void xwin_vne_bridge_window_focus(Window_I* window,
                                  const WindowDescriptor_C& desc,
                                  const XWinVneEventCallbacks_C& callbacks,
                                  bool focused);

}  // namespace vne::xwin
