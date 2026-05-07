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

#include "common/logging_guard.h"

#include "vertexnova/events/events.h"
#include "vertexnova/xwin/window_descriptor.h"
#include "vertexnova/xwin/window_factory.h"

#include <memory>
#include <optional>
#include <string>

namespace vne::xwin::examples {

class ExampleApp {
   public:
    explicit ExampleApp(std::string app_name)
        : app_name_(std::move(app_name)) {}

    bool initialize(const vne::xwin::WindowDescriptor& descriptor,
                    std::optional<vne::xwin::WindowAPI> api = std::nullopt) {
        window_manager_ = api.has_value() ? vne::xwin::WindowFactory::createWindowManager(*api)
                                          : vne::xwin::WindowFactory::createWindowManager();
        if (!window_manager_) {
            VNE_LOG_ERROR << "[" << app_name_
                          << "] createWindowManager failed: " << vne::xwin::WindowFactory::getLastError();
            return false;
        }

        if (!window_manager_->initialize()) {
            VNE_LOG_ERROR << "[" << app_name_ << "] initialize failed";
            return false;
        }

        window_ = window_manager_->openWindow(descriptor);
        if (!window_) {
            VNE_LOG_ERROR << "[" << app_name_ << "] openWindow failed";
            shutdown();
            return false;
        }

        app_listener_ = std::make_shared<Listener>(this);
        auto& ev_mgr = vne::events::EventManager::instance();
        ev_mgr.registerListener(vne::events::EventType::eWindowClose, app_listener_);
        ev_mgr.registerListener(vne::events::EventType::eKeyPressed, app_listener_);

        is_running_ = true;
        return true;
    }

    int run(uint32_t sleep_ms = 16U) {
        if (!window_manager_ || !is_running_) {
            return 1;
        }

        if (window_ && window_->getWindowAPI() == vne::xwin::WindowAPI::eNullWindow) {
            VNE_LOG_INFO << "[" << app_name_ << "] Null backend: run a clean smoke cycle and exit.";
            window_manager_->removeWindow(window_);
            window_.reset();
        }

        while (is_running_ && window_manager_->getWindowCount() > 0U && !window_manager_->shouldClose()) {
            window_manager_->processEvents();
            vne::events::EventManager::instance().processEvents();
            vne::events::Input::nextFrame();
            window_manager_->sleep(sleep_ms);
        }

        shutdown();
        return 0;
    }

    std::shared_ptr<vne::xwin::IWindowManager> windowManager() const { return window_manager_; }

    std::shared_ptr<vne::xwin::IWindow> window() const { return window_; }

    void shutdown() {
        is_running_ = false;

        if (app_listener_) {
            auto& ev_mgr = vne::events::EventManager::instance();
            ev_mgr.unregisterListener(vne::events::EventType::eWindowClose, app_listener_.get());
            ev_mgr.unregisterListener(vne::events::EventType::eKeyPressed, app_listener_.get());
            app_listener_.reset();
        }

        window_.reset();
        if (window_manager_) {
            window_manager_->shutdown();
        }
    }

    void requestClose() {
        is_running_ = false;
        if (window_) {
            window_->close();
            window_manager_->removeWindow(window_);
            window_.reset();
        }
    }

   private:
    class Listener : public vne::events::EventListener {
       public:
        explicit Listener(ExampleApp* app)
            : app_(app) {}

        void onEvent(const vne::events::Event& event) override {
            if (!app_) {
                return;
            }

            if (event.type() == vne::events::EventType::eWindowClose) {
                app_->requestClose();
                return;
            }

            if (event.type() == vne::events::EventType::eKeyPressed) {
                const auto* key_event = dynamic_cast<const vne::events::KeyPressedEvent*>(&event);
                if (key_event && key_event->keyCode() == vne::events::KeyCode::eEscape) {
                    VNE_LOG_INFO << "Escape pressed: closing example window.";
                    app_->requestClose();
                }
            }
        }

       private:
        ExampleApp* app_ = nullptr;
    };

    std::string app_name_;
    LoggingGuard logging_guard_{};
    std::shared_ptr<Listener> app_listener_;
    std::shared_ptr<vne::xwin::IWindowManager> window_manager_;
    std::shared_ptr<vne::xwin::IWindow> window_;
    bool is_running_ = false;
};

}  // namespace vne::xwin::examples
