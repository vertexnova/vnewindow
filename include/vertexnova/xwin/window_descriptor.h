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

#include "vertexnova/xwin/input_mapping.h"
#include "vertexnova/xwin/xwin_types.h"

#include <functional>
#include <memory>
#include <string>
#include <utility>

namespace vne::xwin {
namespace {
constexpr uint32_t kDefaultWindowWidth = 800U;
constexpr uint32_t kDefaultWindowHeight = 600U;
constexpr int kDefaultWindowPosX = 100;
constexpr int kDefaultWindowPosY = 100;
}  // namespace

struct WindowDescriptor {
    std::string title = "VneXWin";
    WindowSize size = {kDefaultWindowWidth, kDefaultWindowHeight};
    WindowPosition position = {kDefaultWindowPosX, kDefaultWindowPosY};
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

    /** Optional per-window native <-> vne::events mapping; shared ownership keeps storage valid after copies of
     *  this descriptor (e.g. into backend state). Android touch-first builds often omit this. */
    std::shared_ptr<WindowInputMapping> input_mapping;

    /**
     * Called on the window-owning thread from inside the OS's modal live-resize loop: after
     * each size change while the user drags an edge, and at display rate while they hold still.
     *
     * That is the thread that drives the platform window (AppKit/Win32 message affinity), which
     * is often but not necessarily the process main thread -- only hosts that create and pump the
     * window there get main-thread callbacks.
     *
     * The application's own event pump does not run during that loop (AppKit's tracking loop,
     * Win32's size/move loop), so without this hook nothing can react until the mouse is released
     * and the frame then snaps to the new size. Hosts use it to publish the new size -- and, when
     * rendering on that same owning thread, to draw a frame. Must not pump events. Optional;
     * macOS and Win32 call it, other platforms have no modal resize loop.
     */
    std::function<void()> on_live_resize;

    WindowDescriptor() = default;

    WindowDescriptor(std::string in_title, uint32_t width, uint32_t height)
        : title(std::move(in_title))
        , size({width, height}) {}

    WindowDescriptor(std::string in_title,
                     const WindowSize& in_size,
                     WindowMode in_mode = WindowMode::eWindowed,
                     bool in_resizable = true,
                     bool in_decorated = true)
        : title(std::move(in_title))
        , size(in_size)
        , mode(in_mode)
        , resizable(in_resizable)
        , decorated(in_decorated) {}
};

}  // namespace vne::xwin
