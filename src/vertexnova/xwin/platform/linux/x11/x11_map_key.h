#pragma once
/* X11 KeySym / state to vne::events types. */
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

#include <X11/Xlib.h>

namespace vne::xwin {

std::uint8_t mapX11Modifiers(unsigned int state);

/** @brief Primary keysym from a key event (index 0). */
vne::events::KeyCode mapX11Keysym(KeySym sym);

/** @brief Best-effort reverse (first keysym that maps to key). */
std::uint64_t mapEventsKeyCodeToX11Keysym(vne::events::KeyCode key);

}  // namespace vne::xwin
