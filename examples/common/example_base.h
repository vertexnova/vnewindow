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

/**
 * @file example_base.h
 *
 * User-facing interface for cross-platform vnewindow examples.
 *
 * Usage:
 *   1. Subclass ExampleBase and override configure() / onInit() / onFrame().
 *   2. Define the factory function:
 *        std::unique_ptr<ExampleBase> createExample();
 *   3. Add the target with vne_add_example() in CMakeLists.txt.
 *
 * The platform entry point (injected by vne_add_example) calls createExample(),
 * drives the lifecycle, and handles OS differences (CADisplayLink on iOS,
 * Emscripten main-loop on Web, blocking while-loop on desktop).
 */

#include "common/logging_guard.h"

#include "vertexnova/xwin/window.h"
#include "vertexnova/xwin/window_manager.h"

#include <memory>
#include <string>

namespace vne::xwin::examples {

struct ExampleConfig {
    std::string title         = "VneWindow Example";
    uint32_t    width         = 800;
    uint32_t    height        = 600;
    bool        enable_events = true;
    bool        enable_input  = true;
};

class ExampleBase {
   public:
    virtual ~ExampleBase() = default;

    /** Return window title, size, and feature flags. Called once before init. */
    virtual ExampleConfig configure() = 0;

    /**
     * Called after the window and window manager are created.
     * Set up event callbacks, load resources, etc.
     */
    virtual void onInit(vne::xwin::IWindow& /*window*/,
                        vne::xwin::IWindowManager& /*mgr*/) {}

    /**
     * Called every frame with the elapsed time in seconds.
     * Return false to request window close.
     */
    virtual bool onFrame(float /*dt_sec*/) { return true; }

    /** Called once when the example is about to be destroyed. */
    virtual void onShutdown() {}
};

}  // namespace vne::xwin::examples

/**
 * Each example's example.cpp defines this factory.
 * The platform entry point calls it to obtain the concrete example instance.
 */
std::unique_ptr<vne::xwin::examples::ExampleBase> createExample();
