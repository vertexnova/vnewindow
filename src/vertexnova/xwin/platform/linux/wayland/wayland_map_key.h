#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   April 2026
 *
 * Maps XKB keysyms and modifier state to vne::events types.
 * XKB keysym values are numerically identical to X11 KeySym values.
 * ----------------------------------------------------------------------
 */

#include <vertexnova/events/types.h>

#include <cstdint>

namespace vne::xwin {

/** Map an xkb_keysym_t (uint32_t) to vne::events::KeyCode. */
vne::events::KeyCode mapWaylandKeysym(uint32_t sym);

/** Map XKB modifier index masks to the vne::events::ModifierKey packed byte. */
uint8_t mapWaylandModifiers(uint32_t depressed, uint32_t latched, uint32_t locked);

}  // namespace vne::xwin
