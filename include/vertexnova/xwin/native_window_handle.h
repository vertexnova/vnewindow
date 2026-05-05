#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   May 2026
 *
 * Autodoc:   yes
 * ----------------------------------------------------------------------
 */

#include "vertexnova/xwin/xwin_types.h"

#include <cstdint>

namespace vne::xwin {

struct NativeWindowHandle_C {
    WindowAPI_TP api = WindowAPI_TP::NULL_WINDOW;

    void* hwnd = nullptr;

    void* ns_view = nullptr;
    void* ns_window = nullptr;

    void* ca_layer = nullptr;
    void* ui_view = nullptr;

    uint32_t x11_window_id = 0;
    void* x11_display = nullptr;
    void* xcb_connection = nullptr;
    uint32_t xcb_window_id = 0;

    void* wl_display = nullptr;
    void* wl_surface = nullptr;

    void* a_native_window = nullptr;

    const char* canvas_id = nullptr;
};

}  // namespace vne::xwin
