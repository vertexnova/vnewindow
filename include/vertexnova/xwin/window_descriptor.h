#pragma once
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

#include "vertexnova/xwin/xwin_types.h"

#include <string>

namespace vne::xwin {

struct WindowDescriptor_C {
    std::string title = "VneXWin";
    WindowSize_C size = {800, 600};
    WindowPosition_C position = {100, 100};
    WindowMode_TP mode = WindowMode_TP::WINDOWED;
    WindowState_TP state = WindowState_TP::NORMAL;
    WindowVisibility_TP visibility = WindowVisibility_TP::VISIBLE;
    bool resizable = true;
    bool decorated = true;
    bool always_on_top = false;
    bool visible = true;
    bool focused = true;
    bool vsync_enabled = true;
    WindowTransparency_TP transparency = WindowTransparency_TP::OPAQUE;
    WindowCursor_TP cursor = WindowCursor_TP::NORMAL;
    WindowLimits_C limits;
    GraphicsBackend_TP graphics_backend = GraphicsBackend_TP::VULKAN;
    void* platform_data = nullptr;
    size_t platform_data_size = 0;
    bool enable_events = true;
    bool enable_input = true;

    WindowDescriptor_C() = default;

    WindowDescriptor_C(const std::string& in_title, uint32_t width, uint32_t height)
        : title(in_title)
        , size({width, height}) {}

    WindowDescriptor_C(const std::string& in_title,
                       const WindowSize_C& in_size,
                       WindowMode_TP in_mode = WindowMode_TP::WINDOWED,
                       bool in_resizable = true,
                       bool in_decorated = true)
        : title(in_title)
        , size(in_size)
        , mode(in_mode)
        , resizable(in_resizable)
        , decorated(in_decorated) {}
};

}  // namespace vne::xwin
