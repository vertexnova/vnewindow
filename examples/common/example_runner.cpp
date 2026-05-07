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

#include "common/example_runner.h"

#include "vertexnova/events/events.h"
#include "vertexnova/xwin/window_descriptor.h"
#include "vertexnova/xwin/window_factory.h"
#include "vertexnova/xwin/xwin_types.h"

#include <vertexnova/logging/logging.h>

namespace vne::xwin::examples {

// ---------------------------------------------------------------------------
// Inner event listener
// ---------------------------------------------------------------------------

class ExampleRunner::RunnerListener : public vne::events::EventListener {
   public:
    explicit RunnerListener(ExampleRunner* runner) : runner_(runner) {}

    void onEvent(const vne::events::Event& event) override {
        if (!runner_) {
            return;
        }
        if (event.type() == vne::events::EventType::eWindowClose) {
            runner_->onCloseRequest();
            return;
        }
        if (event.type() == vne::events::EventType::eKeyPressed) {
            const auto* ke = dynamic_cast<const vne::events::KeyPressedEvent*>(&event);
            if (ke && ke->keyCode() == vne::events::KeyCode::eEscape) {
                VNE_LOG_INFO << "[ExampleRunner] ESC pressed — closing.";
                runner_->onCloseRequest();
            }
        }
    }

   private:
    ExampleRunner* runner_ = nullptr;
};

// ---------------------------------------------------------------------------
// ExampleRunner
// ---------------------------------------------------------------------------

ExampleRunner::ExampleRunner(std::unique_ptr<ExampleBase> example)
    : example_(std::move(example)) {}

ExampleRunner::~ExampleRunner() {
    shutdown();
}

bool ExampleRunner::initialize() {
    if (initialized_) {
        return true;
    }

    ExampleConfig cfg = example_->configure();

    // Build descriptor from ExampleConfig
    WindowDescriptor desc(cfg.title, cfg.width, cfg.height);
    desc.enable_events = cfg.enable_events;
    desc.enable_input  = cfg.enable_input;

    // Auto-select best backend for the current platform
    manager_ = WindowFactory::createWindowManager();
    if (!manager_) {
        VNE_LOG_ERROR << "[ExampleRunner] createWindowManager failed: "
                      << WindowFactory::getLastError();
        return false;
    }

    if (!manager_->initialize()) {
        VNE_LOG_ERROR << "[ExampleRunner] manager initialize() failed.";
        return false;
    }

    window_ = manager_->openWindow(desc);
    if (!window_) {
        VNE_LOG_ERROR << "[ExampleRunner] openWindow() failed.";
        manager_->shutdown();
        return false;
    }

    // Register close + escape listeners
    listener_ = std::make_shared<RunnerListener>(this);
    auto& ev   = vne::events::EventManager::instance();
    ev.registerListener(vne::events::EventType::eWindowClose, listener_);
    ev.registerListener(vne::events::EventType::eKeyPressed,  listener_);

    initialized_ = true;
    running_     = true;
    last_time_   = manager_->getPlatformTime();

    // Null backend: run one clean smoke cycle then stop
    if (window_->getWindowAPI() == WindowAPI::eNullWindow) {
        VNE_LOG_INFO << "[ExampleRunner] Null backend — smoke cycle, then exit.";
        manager_->removeWindow(window_);
        window_.reset();
        running_ = false;
        // Still call onInit so the example logs its banner even in CI
        // (window is gone, so pass a dummy — skip onInit for null)
        return true;
    }

    example_->onInit(*window_, *manager_);
    return true;
}

bool ExampleRunner::tick() {
    if (!initialized_ || !running_) {
        return false;
    }

    // processEvents from platform + vne::events
    if (manager_) {
        manager_->processEvents();
    }
    vne::events::EventManager::instance().processEvents();
    vne::events::Input::nextFrame();

    // Check close conditions
    if (!running_
        || (manager_ && manager_->shouldClose())
        || (!window_ || !window_->isOpen())
        || (manager_ && manager_->getWindowCount() == 0)) {
        return false;
    }

    // Compute delta time in seconds
    double now = manager_->getPlatformTime();
    float  dt  = static_cast<float>(now - last_time_);
    last_time_ = now;

    // Clamp to avoid spiral-of-death on hitches (e.g. breakpoint, resume)
    if (dt > 0.25f) {
        dt = 0.25f;
    }

    bool keep_running = example_->onFrame(dt);
    if (!keep_running) {
        onCloseRequest();
        return false;
    }

    return true;
}

void ExampleRunner::shutdown() {
    if (!initialized_) {
        return;
    }
    initialized_ = false;
    running_      = false;

    example_->onShutdown();

    if (listener_) {
        auto& ev = vne::events::EventManager::instance();
        ev.unregisterListener(vne::events::EventType::eWindowClose, listener_.get());
        ev.unregisterListener(vne::events::EventType::eKeyPressed,  listener_.get());
        listener_.reset();
    }

    window_.reset();
    if (manager_) {
        manager_->shutdown();
        manager_.reset();
    }
}

int ExampleRunner::run() {
    if (!initialized_) {
        return 1;
    }
    // Null backend already cleaned up in initialize(); nothing to loop.
    if (!running_) {
        shutdown();
        return 0;
    }

    while (tick()) {
        manager_->sleep(16); // ~60 fps cap; platform backends may vsync tighter
    }

    shutdown();
    return 0;
}

void ExampleRunner::onCloseRequest() {
    running_ = false;
    if (window_) {
        window_->close();
    }
}

}  // namespace vne::xwin::examples
