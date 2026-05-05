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

/** @file xwin_types.h Shared enums and plain structs for window state and events. */

#include <cstdint>
#include <string>

namespace vne::xwin {

/**
 * @brief Window API implementation (no GLFW in this library).
 *
 * Enumerators use `e` + PascalCase with explicit underlying values (VertexNova coding guidelines).
 */
enum class WindowAPI {
    eNullWindow = 0,

    eWin32Window = 20,
    eCocoaWindow = 21,
    eX11Window = 30,
    eWaylandWindow = 31,

    eIosUikitWindow = 40,
    eTvosUikitWindow = 41,
    eAndroidSurfaceWindow = 50,

    eWasmWindow = 60,
    eWebgpuWindow = 61,
};

enum class GraphicsBackend {
    eOpenGL = 0,
    eOpenGLES = 1,
    eVulkan = 2,
    eMetal = 3,
    eDirectX11 = 4,
    eDirectX12 = 5,
    eWebGL = 6,
    eWebGPU = 7,
    eUnknown = 99
};

enum class WindowMode { eWindowed = 0, eFullscreen = 1, eBorderless = 2, eMaximized = 3 };

enum class WindowState { eNormal = 0, eMinimized = 1, eMaximized = 2, eFullscreen = 3 };

enum class WindowVisibility { eVisible = 0, eHidden = 1, eMinimized = 2 };

enum class WindowFocus { eFocused = 0, eUnfocused = 1 };

enum class WindowResize { eResizable = 0, eFixedSize = 1 };

enum class WindowDecoration { eDecorated = 0, eBorderless = 1 };

enum class WindowTransparency { eOpaque = 0, eTranslucent = 1 };

enum class WindowCursor { eNormal = 0, eHidden = 1, eDisabled = 2 };

enum class WindowEventType {
    eClose = 0,
    eResize = 1,
    eMove = 2,
    eFocus = 3,
    eMinimize = 4,
    eMaximize = 5,
    eRestore = 6,
    eShow = 7,
    eHide = 8,
    eTap = 10,
    ePan = 11,
    ePinch = 12,
    eOrientation = 13,
    eStatusBar = 14
};

enum class EventAction { eBegan = 0, eChanged = 1, eEnded = 2, eCancelled = 3 };

struct WindowPosition {
    int32_t x = 0;
    int32_t y = 0;
};

struct WindowSize {
    uint32_t width = 0;
    uint32_t height = 0;
};

struct WindowBounds {
    WindowPosition position;
    WindowSize size;
};

struct WindowLimits {
    WindowSize min_size;
    WindowSize max_size;
    bool has_min_size = false;
    bool has_max_size = false;
};

struct WindowEventData {
    WindowEventType type{};
    WindowSize size{};
    WindowPosition position{};
    bool focused = false;
    bool minimized = false;
    bool maximized = false;
    uint32_t touch_count = 0;
    float velocity_x = 0.0F;
    float velocity_y = 0.0F;
    float scale = 1.0F;
    float velocity = 0.0F;
    EventAction action = EventAction::eBegan;
    int orientation = 0;
    float status_bar_height = 0.0F;
};

struct WindowProperties {
    std::string title;
    WindowSize size;
    WindowPosition position;
    WindowMode mode = WindowMode::eWindowed;
    WindowState state = WindowState::eNormal;
    WindowVisibility visibility = WindowVisibility::eVisible;
    WindowFocus focus = WindowFocus::eFocused;
    WindowResize resize = WindowResize::eResizable;
    WindowDecoration decoration = WindowDecoration::eDecorated;
    WindowTransparency transparency = WindowTransparency::eOpaque;
    WindowCursor cursor = WindowCursor::eNormal;
    WindowLimits limits;
    bool vsync_enabled = true;
    bool always_on_top = false;
    bool resizable = true;
    bool decorated = true;
    bool visible = true;
    bool focused = true;
};

}  // namespace vne::xwin
