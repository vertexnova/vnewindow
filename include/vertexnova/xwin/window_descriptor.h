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

/** @file window_descriptor.h Creation-time settings for a window. */

#include "vertexnova/xwin/xwin_types.h"

#include <string>

namespace vne::xwin {

struct WindowInputMapping;

struct WindowDescriptor {
    std::string title = "VneXWin";
    WindowSize size = {800, 600};
    WindowPosition position = {100, 100};
    WindowMode mode = WindowMode::eWindowed;
    WindowState state = WindowState::eNormal;
    WindowVisibility visibility = WindowVisibility::eVisible;
    bool resizable = true;
    bool decorated = true;
    bool always_on_top = false;
    bool visible = true;
    bool focused = true;
    bool vsync_enabled = true;
    WindowTransparency transparency = WindowTransparency::eOpaque;
    WindowCursor cursor = WindowCursor::eNormal;
    WindowLimits limits;
    GraphicsBackend graphics_backend = GraphicsBackend::eVulkan;
    void* platform_data = nullptr;
    size_t platform_data_size = 0;
    bool enable_events = true;
    bool enable_input = true;

    /** Optional per-window native ↔ vne::events mapping; must outlive the window if non-null.
     *  Used for desktop KM and for UIKit pointer/keyboard; Android touch-first builds often omit this. */
    const WindowInputMapping* input_mapping = nullptr;

    WindowDescriptor() = default;

    WindowDescriptor(const std::string& in_title, uint32_t width, uint32_t height)
        : title(in_title)
        , size({width, height}) {}

    WindowDescriptor(const std::string& in_title,
                     const WindowSize& in_size,
                     WindowMode in_mode = WindowMode::eWindowed,
                     bool in_resizable = true,
                     bool in_decorated = true)
        : title(in_title)
        , size(in_size)
        , mode(in_mode)
        , resizable(in_resizable)
        , decorated(in_decorated) {}
};

}  // namespace vne::xwin
