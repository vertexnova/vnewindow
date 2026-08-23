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

#include "vertexnova/common/macros.h"
#include "vertexnova/events/events.h"
#include "vertexnova/xwin/window_descriptor.h"
#include "vertexnova/xwin/window_factory.h"
#include "vertexnova/xwin/xwin_types.h"

#include "vertexnova/logging/logging.h"

#include <array>

namespace vne::xwin::examples {

namespace {

constexpr std::array kForwardedEventTypes = {
    vne::events::EventType::eWindowClose,
    vne::events::EventType::eWindowResize,
    vne::events::EventType::eWindowFocus,
    vne::events::EventType::eKeyPressed,
    vne::events::EventType::eKeyReleased,
    vne::events::EventType::eKeyRepeat,
    vne::events::EventType::eTextInput,
    vne::events::EventType::eMouseButtonPressed,
    vne::events::EventType::eMouseButtonReleased,
    vne::events::EventType::eMouseButtonDoubleClicked,
    vne::events::EventType::eMouseMoved,
    vne::events::EventType::eMouseScrolled,
    vne::events::EventType::eTouchPress,
    vne::events::EventType::eTouchRelease,
    vne::events::EventType::eTouchMove,
    vne::events::EventType::eWindowMinimize,
    vne::events::EventType::eWindowRestore,
    vne::events::EventType::eWindowMove,
    vne::events::EventType::eWindowDpiChanged,
    vne::events::EventType::eWindowSafeAreaChanged,
    vne::events::EventType::eApplicationPause,
    vne::events::EventType::eApplicationResume,
    vne::events::EventType::eApplicationLowMemory,
};

}  // namespace

// ---------------------------------------------------------------------------
// Inner event listener
// ---------------------------------------------------------------------------

class ExampleRunner::RunnerListener : public vne::events::EventListener {
   public:
    explicit RunnerListener(ExampleRunner* runner)
        : runner_(runner) {}

    void onEvent(const vne::events::Event& event) override {
        if (!runner_) {
            return;
        }
        if (event.type() == vne::events::EventType::eWindowClose) {
            // Only exit when the runner's primary window closes; child windows may close independently.
            if (runner_->window_ && event.windowId() == runner_->window_->getId()) {
                runner_->onCloseRequest();
            }
        }
        if (event.type() == vne::events::EventType::eKeyPressed) {
            const auto* ke = dynamic_cast<const vne::events::KeyPressedEvent*>(&event);
            if (ke && ke->keyCode() == vne::events::KeyCode::eEscape) {
                VNE_LOG_INFO << "[ExampleRunner] ESC pressed — closing.";
                runner_->onCloseRequest();
            }
        }
        if (runner_->example_) {
            runner_->example_->onEvent(event);
        }
    }

   private:
    ExampleRunner* runner_ = nullptr;
};

// ---------------------------------------------------------------------------
// ExampleRunner
// ---------------------------------------------------------------------------

ExampleRunner::ExampleRunner(std::unique_ptr<ExampleBase> example)
    : example_(std::move(example)) {
    VNE_ASSERT_MSG(example_ != nullptr, "ExampleRunner requires a non-null ExampleBase");
}

ExampleRunner::~ExampleRunner() {
    shutdown();
}

bool ExampleRunner::initialize() {
    if (is_initialized_) {
        return true;
    }

    if (!example_) {
        VNE_LOG_ERROR << "[ExampleRunner] initialize() failed: example is null.";
        return false;
    }

    const ExampleConfig cfg = example_->configure();

    // Build descriptor from ExampleConfig
    WindowDescriptor desc(cfg.title, cfg.width, cfg.height);
    desc.enable_events = cfg.enable_events;
    desc.enable_input = cfg.enable_input;
    desc.platform_data = platform_data_;
    desc.platform_data_size = platform_data_size_;

    // Auto-select best backend for the current platform
    manager_ = WindowFactory::createWindowManager();
    if (!manager_) {
        VNE_LOG_ERROR << "[ExampleRunner] createWindowManager failed: " << WindowFactory::getLastError();
        return false;
    }

    if (!manager_->initialize()) {
        VNE_LOG_ERROR << "[ExampleRunner] manager initialize() failed.";
        manager_->shutdown();
        manager_.reset();
        return false;
    }

    window_ = manager_->openWindow(desc);
    if (!window_) {
        VNE_LOG_ERROR << "[ExampleRunner] openWindow() failed.";
        manager_->shutdown();
        manager_.reset();
        return false;
    }

    // Register close + escape listeners and forward all structured events.
    listener_ = std::make_shared<RunnerListener>(this);
    auto& ev = vne::events::EventManager::instance();
    for (const auto t : kForwardedEventTypes) {
        ev.registerListener(t, listener_);
    }

    is_initialized_ = true;
    is_running_ = true;
    last_time_ = manager_->getPlatformTime();

    // Null backend: run one clean smoke cycle then stop
    if (window_->getWindowAPI() == WindowAPI::eNullWindow) {
        VNE_LOG_INFO << "[ExampleRunner] Null backend — smoke cycle, then exit.";
        manager_->removeWindow(window_);
        window_.reset();
        is_running_ = false;
        // onInit is deliberately skipped for the null backend: no window exists.
        return true;
    }

    example_->onInit(*window_, *manager_);
    return true;
}

bool ExampleRunner::tick() {
    if (!is_initialized_ || !is_running_) {
        return false;
    }

    // processEvents from platform + vne::events
    if (manager_) {
        manager_->processEvents();
    }
    vne::events::EventManager::instance().processEvents();

    // Check close conditions
    const bool any_open = manager_ && manager_->getWindowCount() > 0;
    const bool primary_closed = window_ && !window_->isOpen();
    if (!is_running_ || !any_open || primary_closed || (manager_ && manager_->shouldCloseAll())) {
        vne::events::Input::nextFrame();
        return false;
    }

    // Compute delta time in seconds
    const double now = manager_->getPlatformTime();
    float dt = static_cast<float>(now - last_time_);
    last_time_ = now;

    // Clamp to avoid spiral-of-death on hitches (e.g. breakpoint, resume)
    if (dt > 0.25f) {
        dt = 0.25f;
    }

    const bool keep_running = example_->onFrame(dt);
    vne::events::Input::nextFrame();
    if (!keep_running) {
        onCloseRequest();
        return false;
    }

    return true;
}

void ExampleRunner::shutdown() {
    if (!is_initialized_) {
        return;
    }
    is_initialized_ = false;
    is_running_ = false;

    if (example_) {
        example_->onShutdown();
    }

    if (listener_) {
        auto& ev = vne::events::EventManager::instance();
        for (const auto t : kForwardedEventTypes) {
            ev.unregisterListener(t, listener_.get());
        }
        listener_.reset();
    }

    window_.reset();
    if (manager_) {
        manager_->destroyAllWindows();
        manager_->shutdown();
        manager_.reset();
    }
}

int ExampleRunner::run() {
    if (!is_initialized_) {
        return 1;
    }
    // Null backend already cleaned up in initialize(); nothing to loop.
    if (!is_running_) {
        shutdown();
        return 0;
    }

    while (tick()) {
        manager_->sleep(16U);  // ~60 fps cap; platform backends may vsync tighter
    }

    shutdown();
    return 0;
}

void ExampleRunner::onCloseRequest() {
    is_running_ = false;
    if (manager_) {
        manager_->destroyAllWindows();
    }
    window_.reset();
}

}  // namespace vne::xwin::examples
