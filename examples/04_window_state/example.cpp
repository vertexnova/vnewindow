/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   August 2026
 *
 * Autodoc:   yes
 * ----------------------------------------------------------------------
 */

/**
 * 04_window_state - Drive every window-state setter and watch what it emits.
 *
 * Each key issues one IWindow call; every vne::events event that results is logged with the id
 * of the window that produced it. That pairing is the point: it shows, per backend, which state
 * transitions actually surface as events and which are silently applied.
 *
 * Coverage differs by platform and that is expected - mobile and web have no concept of a moved
 * or maximized window, so those keys are no-ops there.
 */

#include "common/example_base.h"

#include "vertexnova/xwin/window.h"
#include "vertexnova/xwin/window_manager.h"
#include "vertexnova/xwin/xwin_types.h"

#include <vertexnova/events/events.h>
#include <vertexnova/logging/logging.h>

#include <cstdint>
#include <string>

namespace {

const char* toString(vne::xwin::WindowMode mode) {
    using vne::xwin::WindowMode;
    switch (mode) {
        case WindowMode::eWindowed:
            return "windowed";
        case WindowMode::eFullscreen:
            return "fullscreen";
        case WindowMode::eBorderless:
            return "borderless";
        case WindowMode::eMaximized:
            return "maximized";
    }
    return "unknown";
}

}  // namespace

class WindowStateExample final : public vne::xwin::examples::ExampleBase {
   public:
    vne::xwin::examples::ExampleConfig configure() override { return {"04 Window State & Control", 900, 640}; }

    void onInit(vne::xwin::IWindow& window, vne::xwin::IWindowManager& mgr) override {
        window_ = &window;
        mgr_ = &mgr;
        logHelp();
        logState("startup");
    }

    bool onFrame(float /*dt*/) override { return true; }

    void onEvent(const vne::events::Event& event) override {
        using vne::events::EventType;

        // Commands first, so a key that changes state still logs the event it caused below.
        if (event.type() == EventType::eKeyPressed) {
            const auto& key = static_cast<const vne::events::KeyPressedEvent&>(event);
            handleCommand(key.keyCode());
        }

        switch (event.type()) {
            case EventType::eWindowResize: {
                const auto& e = static_cast<const vne::events::WindowResizeEvent&>(event);
                logEvent("RESIZE", event, std::to_string(e.width()) + "x" + std::to_string(e.height()));
                break;
            }
            case EventType::eWindowMove: {
                const auto& e = static_cast<const vne::events::WindowMoveEvent&>(event);
                logEvent("MOVE", event, "(" + std::to_string(e.x()) + ", " + std::to_string(e.y()) + ")");
                break;
            }
            case EventType::eWindowMinimize:
                logEvent("MINIMIZE", event, "");
                break;
            case EventType::eWindowRestore:
                logEvent("RESTORE", event, "");
                break;
            case EventType::eWindowFocus: {
                const auto& e = static_cast<const vne::events::WindowFocusEvent&>(event);
                logEvent("FOCUS", event, e.focused() ? "gained" : "lost");
                break;
            }
            case EventType::eWindowDpiChanged: {
                const auto& e = static_cast<const vne::events::WindowDpiChangedEvent&>(event);
                logEvent("DPI", event, "scale=" + std::to_string(e.scale()));
                break;
            }
            case EventType::eWindowSafeAreaChanged: {
                const auto& e = static_cast<const vne::events::WindowSafeAreaChangedEvent&>(event);
                logEvent("SAFEAREA", event, "top=" + std::to_string(e.top()) + " bottom=" + std::to_string(e.bottom()));
                break;
            }
            default:
                break;
        }
    }

   private:
    static void logHelp() {
        VNE_LOG_INFO << "Keys: [F] fullscreen  [B] borderless  [W] windowed  [M] maximize";
        VNE_LOG_INFO << "      [N] minimize    [R] restore     [L] size limits toggle";
        VNE_LOG_INFO << "      [arrows] move   [+/-] resize    [V] vsync   [T] transparent";
        VNE_LOG_INFO << "      [S] print state [ESC] quit";
        VNE_LOG_INFO << "Each command logs the events it produced; silence means the backend "
                        "applied the change without surfacing one.";
    }

    static void logEvent(const char* label, const vne::events::Event& event, const std::string& detail) {
        VNE_LOG_INFO << "  <- [" << label << "] window=" << static_cast<std::uint32_t>(event.windowId())
                     << (detail.empty() ? "" : "  ") << detail;
    }

    void logState(const char* when) const {
        if (!window_) {
            return;
        }
        const auto pos = window_->getPosition();
        VNE_LOG_INFO << "state (" << when << "): mode=" << toString(window_->getWindowMode())
                     << " fullscreen=" << (window_->isFullscreen() ? "yes" : "no") << " size=" << window_->getWidth()
                     << "x" << window_->getHeight() << " fb=" << window_->getFramebufferWidth() << "x"
                     << window_->getFramebufferHeight() << " pos=(" << pos.x << ", " << pos.y << ")"
                     << " dpi=" << window_->getDpiScale() << " vsync=" << (window_->isVSyncEnabled() ? "on" : "off")
                     << " transparent=" << (window_->isTransparent() ? "yes" : "no");
    }

    void handleCommand(vne::events::KeyCode key) {
        using vne::events::KeyCode;
        using vne::xwin::WindowMode;
        if (!window_) {
            return;
        }

        switch (key) {
            case KeyCode::eF:
                command("setFullscreen", !window_->isFullscreen());
                window_->setFullscreen(!window_->isFullscreen());
                break;
            case KeyCode::eB:
                command("setWindowMode", "borderless");
                window_->setWindowMode(WindowMode::eBorderless);
                break;
            case KeyCode::eW:
                command("setWindowMode", "windowed");
                window_->setWindowMode(WindowMode::eWindowed);
                break;
            case KeyCode::eM:
                command("maximize", "");
                window_->maximize();
                break;
            case KeyCode::eN:
                command("minimize", "");
                window_->minimize();
                break;
            case KeyCode::eR:
                command("restore", "");
                window_->restore();
                break;
            case KeyCode::eL:
                toggleLimits();
                break;
            case KeyCode::eV:
                command("setVSync", !window_->isVSyncEnabled());
                window_->setVSync(!window_->isVSyncEnabled());
                break;
            case KeyCode::eT:
                command("setTransparent", !window_->isTransparent());
                window_->setTransparent(!window_->isTransparent());
                break;
            case KeyCode::eS:
                logState("on demand");
                break;
            case KeyCode::eLeft:
            case KeyCode::eRight:
            case KeyCode::eUp:
            case KeyCode::eDown:
                nudge(key);
                break;
            case KeyCode::eEqual:
                resizeBy(kResizeStep);
                break;
            case KeyCode::eMinus:
                resizeBy(-kResizeStep);
                break;
            default:
                break;
        }
    }

    template<typename ValueT>
    static void command(const char* name, ValueT value) {
        VNE_LOG_INFO << "-> " << name << "(" << value << ")";
    }
    static void command(const char* name, const char* value) { VNE_LOG_INFO << "-> " << name << "(" << value << ")"; }

    void toggleLimits() {
        limits_on_ = !limits_on_;
        vne::xwin::WindowLimits limits;
        if (limits_on_) {
            limits.min_size = {400U, 300U};
            limits.max_size = {1600U, 900U};
            limits.has_min_size = true;
            limits.has_max_size = true;
        }
        command("setWindowLimits", limits_on_ ? "400x300 .. 1600x900" : "cleared");
        window_->setWindowLimits(limits);
    }

    void nudge(vne::events::KeyCode key) {
        using vne::events::KeyCode;
        const auto pos = window_->getPosition();
        int dx = 0;
        int dy = 0;
        if (key == KeyCode::eLeft) {
            dx = -kMoveStep;
        } else if (key == KeyCode::eRight) {
            dx = kMoveStep;
        } else if (key == KeyCode::eUp) {
            dy = -kMoveStep;
        } else {
            dy = kMoveStep;
        }
        VNE_LOG_INFO << "-> setPosition(" << (pos.x + dx) << ", " << (pos.y + dy) << ")";
        window_->setPosition(pos.x + dx, pos.y + dy);
    }

    void resizeBy(int delta) {
        const int w = window_->getWidth() + delta;
        const int h = window_->getHeight() + delta;
        if (w <= 0 || h <= 0) {
            return;
        }
        VNE_LOG_INFO << "-> resize(" << w << ", " << h << ")";
        window_->resize(static_cast<uint32_t>(w), static_cast<uint32_t>(h));
    }

    static constexpr int kMoveStep = 40;
    static constexpr int kResizeStep = 80;

    vne::xwin::IWindow* window_ = nullptr;
    vne::xwin::IWindowManager* mgr_ = nullptr;
    bool limits_on_ = false;
};

std::unique_ptr<vne::xwin::examples::ExampleBase> createExample() {
    return std::make_unique<WindowStateExample>();
}
