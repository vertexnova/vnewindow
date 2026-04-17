#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   April 2026
 *
 * Maps Emscripten KeyboardEvent.code strings and mouse button indices
 * to vne::events::KeyCode / MouseButton.
 * ----------------------------------------------------------------------
 */

#include <vertexnova/events/types.h>

#include <cstdint>

namespace vne::xwin {

/**
 * @brief Map a DOM KeyboardEvent.code string (e.g. "KeyA", "ArrowLeft") to
 *        vne::events::KeyCode. Returns KeyCode::eUnknown for unmapped codes.
 */
vne::events::KeyCode mapEmscriptenKey(const char* code);

/**
 * @brief Map Emscripten mouse button index (0=left,1=middle,2=right) to
 *        vne::events::MouseButton.
 */
vne::events::MouseButton mapEmscriptenMouseButton(unsigned short button);

/**
 * @brief Map Emscripten keyboard modifier fields to vne::events::ModifierKey byte.
 */
uint8_t mapEmscriptenModifiers(bool shift, bool ctrl, bool alt, bool meta);

}  // namespace vne::xwin
