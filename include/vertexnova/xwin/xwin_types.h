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

    eIosUikitWindow = 40,  // iOS, visionOS, and tvOS (UIKit)
    eAndroidSurfaceWindow = 50,

    eWasmWindow = 60,  // Emscripten canvas; use RHI WebGPU for presentation
};

enum class GraphicsBackend {
    eOpenGL = 0,
    eOpenGLES = 1,
    eVulkan = 2,
    eMetal = 3,
    eDirectX11 = 4,
    eDirectX12 = 5,
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

/**
 * @brief Phase of a touch point, as delivered to the event bridge and the host injection APIs.
 *
 * Platform "cancelled" phases (UIKit touchesCancelled, browser touchcancel) collapse to eUp:
 * consumers must release the touch either way, and the distinction has no consumer today.
 */
enum class TouchPhase : std::uint8_t { eDown = 0, eUp = 1, eMove = 2 };

/** @brief Process-wide application lifecycle transitions, injected by the platform host. */
enum class ApplicationLifecycle : std::uint8_t { ePause = 0, eResume = 1, eLowMemory = 2 };

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

}  // namespace vne::xwin
