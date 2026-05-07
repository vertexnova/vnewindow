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
 * @file example_runner.h
 *
 * Platform-agnostic runtime that drives an ExampleBase lifecycle.
 * Platform entry points (desktop main.cpp, iOS AppDelegate) call these
 * methods — they never need to know which backend is active.
 */

#include "common/example_base.h"
#include "common/logging_guard.h"

#include "vertexnova/xwin/window.h"
#include "vertexnova/xwin/window_manager.h"

#include <memory>

namespace vne::xwin::examples {

class ExampleRunner {
   public:
    explicit ExampleRunner(std::unique_ptr<ExampleBase> example);
    ~ExampleRunner();

    ExampleRunner(const ExampleRunner&)            = delete;
    ExampleRunner& operator=(const ExampleRunner&) = delete;

    /**
     * Create window manager, open window, register listeners, call onInit().
     * Returns false on failure.
     */
    bool initialize();

    /**
     * Execute one frame: processEvents → onFrame(dt) → return running state.
     * Returns false when the runner should stop (window closed / ESC / onFrame
     * returned false / null-backend smoke cycle finished).
     */
    bool tick();

    /**
     * Call onShutdown(), destroy the window and shut down the manager.
     * Safe to call multiple times.
     */
    void shutdown();

    /**
     * Desktop convenience: blocks, calling tick() until done, then shutdown().
     * Returns 0 on clean exit, 1 on init failure.
     */
    int run();

    // Accessors for platform entry points (e.g. iOS AppDelegate)
    IWindow*        window()        const { return window_.get(); }
    IWindowManager* windowManager() const { return manager_.get(); }

   private:
    void onCloseRequest();

    std::unique_ptr<ExampleBase> example_;
    LoggingGuard                 logging_guard_;

    std::shared_ptr<IWindowManager> manager_;
    std::shared_ptr<IWindow>        window_;

    // Event listener (inner class defined in .cpp)
    class RunnerListener;
    std::shared_ptr<RunnerListener> listener_;

    bool initialized_ = false;
    bool running_     = false;
    double last_time_ = 0.0;
};

}  // namespace vne::xwin::examples
