#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Win32 virtual-key / message helpers to vne::events key and mouse enums.
 * ----------------------------------------------------------------------
 */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <vertexnova/events/types.h>

namespace vne::xwin {

/** @brief Map GetKeyState-style modifier bits to vne::events::ModifierKey flags. */
std::uint8_t xwin_map_win32_modifier_flags();

/** @brief Map WM_KEY* / WM_SYSKEY* wParam/lParam to KeyCode (eUnknown if unmapped). */
vne::events::KeyCode xwin_map_win32_key(WPARAM vk, LPARAM lParam);

/** @brief Left/right/middle/extra from WM_*BUTTON* / double-click messages. */
vne::events::MouseButton xwin_map_win32_mouse_button_from_message(UINT msg, WPARAM wParam);

}  // namespace vne::xwin
