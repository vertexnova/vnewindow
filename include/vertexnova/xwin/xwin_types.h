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

#include <cstdint>
#include <string>

namespace vne::xwin {

/**
 * @brief Window API implementation (no GLFW in this library).
 */
enum class WindowAPI_TP {
    NULL_WINDOW = 0,

    WIN32_WINDOW = 20,
    COCOA_WINDOW = 21,
    X11_WINDOW = 30,
    WAYLAND_WINDOW = 31,

    IOS_UIKIT_WINDOW = 40,
    ANDROID_SURFACE_WINDOW = 50,

    WASM_WINDOW = 60,
    WEBGPU_WINDOW = 61,
};

enum class GraphicsBackend_TP {
    OPENGL = 0,
    OPENGLES = 1,
    VULKAN = 2,
    METAL = 3,
    DIRECTX11 = 4,
    DIRECTX12 = 5,
    WEBGL = 6,
    WEBGPU = 7,
    UNKNOWN = 99
};

enum class WindowMode_TP { WINDOWED = 0, FULLSCREEN = 1, BORDERLESS = 2, MAXIMIZED = 3 };

enum class WindowState_TP { NORMAL = 0, MINIMIZED = 1, MAXIMIZED = 2, FULLSCREEN = 3 };

enum class WindowVisibility_TP { VISIBLE = 0, HIDDEN = 1, MINIMIZED = 2 };

enum class WindowFocus_TP { FOCUSED = 0, UNFOCUSED = 1 };

enum class WindowResize_TP { RESIZABLE = 0, FIXED_SIZE = 1 };

enum class WindowDecoration_TP { DECORATED = 0, BORDERLESS = 1 };

enum class WindowTransparency_TP { OPAQUE = 0, TRANSLUCENT = 1 };

enum class WindowCursor_TP { NORMAL = 0, HIDDEN = 1, DISABLED = 2 };

enum class WindowEventType_TP {
    CLOSE = 0,
    RESIZE = 1,
    MOVE = 2,
    FOCUS = 3,
    MINIMIZE = 4,
    MAXIMIZE = 5,
    RESTORE = 6,
    SHOW = 7,
    HIDE = 8,
    TAP = 10,
    PAN = 11,
    PINCH = 12,
    ORIENTATION = 13,
    STATUS_BAR = 14
};

enum class EventAction_TP { BEGAN = 0, CHANGED = 1, ENDED = 2, CANCELLED = 3 };

struct WindowPosition_C {
    int32_t x = 0;
    int32_t y = 0;
};

struct WindowSize_C {
    uint32_t width = 0;
    uint32_t height = 0;
};

struct WindowBounds_C {
    WindowPosition_C position;
    WindowSize_C size;
};

struct WindowLimits_C {
    WindowSize_C min_size;
    WindowSize_C max_size;
    bool has_min_size = false;
    bool has_max_size = false;
};

struct WindowEventData_C {
    WindowEventType_TP type{};
    WindowSize_C size{};
    WindowPosition_C position{};
    bool focused = false;
    bool minimized = false;
    bool maximized = false;
    uint32_t touch_count = 0;
    float velocity_x = 0.0F;
    float velocity_y = 0.0F;
    float scale = 1.0F;
    float velocity = 0.0F;
    EventAction_TP action = EventAction_TP::BEGAN;
    int orientation = 0;
    float status_bar_height = 0.0F;
};

struct WindowProperties_C {
    std::string title;
    WindowSize_C size;
    WindowPosition_C position;
    WindowMode_TP mode = WindowMode_TP::WINDOWED;
    WindowState_TP state = WindowState_TP::NORMAL;
    WindowVisibility_TP visibility = WindowVisibility_TP::VISIBLE;
    WindowFocus_TP focus = WindowFocus_TP::FOCUSED;
    WindowResize_TP resize = WindowResize_TP::RESIZABLE;
    WindowDecoration_TP decoration = WindowDecoration_TP::DECORATED;
    WindowTransparency_TP transparency = WindowTransparency_TP::OPAQUE;
    WindowCursor_TP cursor = WindowCursor_TP::NORMAL;
    WindowLimits_C limits;
    bool vsync_enabled = true;
    bool always_on_top = false;
    bool resizable = true;
    bool decorated = true;
    bool visible = true;
    bool focused = true;
};

}  // namespace vne::xwin
