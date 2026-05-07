#pragma once
/*
 * Maps macOS NSEvent virtual key codes and modifier flags to
 * vne::events::KeyCode / ModifierKey.
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

namespace vne::xwin {

/**
 * @brief Map an NSEvent.keyCode (CGKeyCode / uint16) to vne::events::KeyCode.
 * Returns KeyCode::eUnknown for unmapped scan codes.
 */
vne::events::KeyCode mapCocoaKeyCode(uint16_t key_code);

/**
 * @brief Map NSEvent.modifierFlags (NSEventModifierFlags bitmask) to the
 * vne::events::ModifierKey packed byte used by the bridge.
 */
uint8_t mapCocoaModifiers(unsigned long modifier_flags);

/** @brief Best-effort reverse of mapCocoaKeyCode (first matching scan code). */
uint64_t mapEventsKeyCodeToCocoaPacked(vne::events::KeyCode key);

/** @brief Synthesize NSEventModifierFlags bits from a vne::events::ModifierKey byte. */
unsigned long mapEventsModifiersToCocoaFlags(uint8_t events_modifiers);

}  // namespace vne::xwin
