#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Maps macOS NSEvent virtual key codes and modifier flags to
 * vne::events::KeyCode / ModifierKey.
 * ----------------------------------------------------------------------
 */

#include <vertexnova/events/types.h>

#include <cstdint>

namespace vne::xwin {

/**
 * @brief Map an NSEvent.keyCode (CGKeyCode / uint16) to vne::events::KeyCode.
 * Returns KeyCode::eUnknown for unmapped scan codes.
 */
vne::events::KeyCode xwinMapCocoaKeyCode(uint16_t key_code);

/**
 * @brief Map NSEvent.modifierFlags (NSEventModifierFlags bitmask) to the
 * vne::events::ModifierKey packed byte used by the bridge.
 */
uint8_t xwinMapCocoaModifiers(unsigned long modifier_flags);

}  // namespace vne::xwin
