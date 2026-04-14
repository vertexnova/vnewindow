#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * X11 KeySym / state to vne::events types.
 * ----------------------------------------------------------------------
 */

#include <vertexnova/events/types.h>

#include <X11/Xlib.h>

namespace vne::xwin {

std::uint8_t xwin_map_x11_modifiers(unsigned int state);

/** @brief Primary keysym from a key event (index 0). */
vne::events::KeyCode xwin_map_x11_keysym(KeySym sym);

}  // namespace vne::xwin
