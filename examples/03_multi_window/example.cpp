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
 * 03_multi_window - multi-window management and event routing demo.
 *
 * Opens a primary window plus optional child windows. Every event is logged with
 * the window id and title, resolved via IWindowManager::findWindow().
 *
 * Controls:
 *   N - open a child window
 *   F - focus next window
 *   P - set focused window as primary
 *   C - close focused window
 *   ESC - quit (handled by ExampleRunner)
 */

#include "common/example_base.h"

#include "vertexnova/events/events.h"
#include "vertexnova/xwin/window_descriptor.h"

#include <vertexnova/logging/logging.h>

#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace {

std::string windowTag(vne::xwin::IWindowManager* mgr,
                      vne::events::WindowId id,
                      const std::unordered_map<vne::events::WindowId, std::string>& titles) {
    std::ostringstream ss;
    ss << "[win=" << static_cast<std::uint32_t>(id);
    const auto it = titles.find(id);
    if (it != titles.end()) {
        ss << " title=\"" << it->second << "\"";
    } else if (mgr) {
        const auto w = mgr->findWindow(id);
        if (w) {
            ss << " title=\"(unknown)\"";
        } else {
            ss << " title=\"?\"";
        }
    }
    ss << "]";
    return ss.str();
}

const char* keyLabel(vne::events::KeyCode key) {
    using vne::events::KeyCode;
    switch (key) {
        case KeyCode::eN:
            return "N";
        case KeyCode::eF:
            return "F";
        case KeyCode::eP:
            return "P";
        case KeyCode::eC:
            return "C";
        default:
            break;
    }
    const int code = static_cast<int>(key);
    if (code >= 32 && code <= 126) {
        static thread_local char buf[2] = {};
        buf[0] = static_cast<char>(code);
        return buf;
    }
    return "Key";
}

}  // namespace

class MultiWindowExample final : public vne::xwin::examples::ExampleBase {
   public:
    vne::xwin::examples::ExampleConfig configure() override { return {"Multi-Window Demo", 640, 480}; }

    void onInit(vne::xwin::IWindow& main, vne::xwin::IWindowManager& mgr) override {
        mgr_ = &mgr;
        main_ = mgr.getPrimaryWindow();
        titles_[main.getId()] = "Multi-Window Demo";

        VNE_LOG_INFO << "=== Multi-Window Demo ===";
        logWindowList("startup");
        logHelp();

        if (mgr.getWindowAPI() == vne::xwin::WindowAPI::eIosUikitWindow) {
            VNE_LOG_INFO << "Note: UIKit is typically single-scene; child windows may not be supported.";
            return;
        }
        if (!mgr.supportsMultipleWindows()) {
            VNE_LOG_INFO << "This backend hosts a single window; child windows are skipped.";
            return;
        }
        if (mgr.getWindowAPI() == vne::xwin::WindowAPI::eWasmWindow) {
            VNE_LOG_INFO << "Web: multiple panels inside #vne-desktop (click a panel to focus, then type).";
        }
        openChild();
        openChild();
        if (main_) {
            mgr.focusWindow(main_);
        }
    }

    void onEvent(const vne::events::Event& event) override {
        using namespace vne::events;

        const std::string tag = windowTag(mgr_, event.windowId(), titles_);

        switch (event.type()) {
            case EventType::eKeyPressed: {
                const auto* e = dynamic_cast<const KeyPressedEvent*>(&event);
                if (!e) {
                    return;
                }
                VNE_LOG_INFO << tag << " [KEY] DOWN " << keyLabel(e->keyCode());
                handleCommand(e->keyCode());
                return;
            }
            case EventType::eKeyReleased: {
                const auto* e = dynamic_cast<const KeyReleasedEvent*>(&event);
                if (!e) {
                    return;
                }
                VNE_LOG_INFO << tag << " [KEY] UP " << keyLabel(e->keyCode());
                return;
            }
            case EventType::eMouseButtonPressed: {
                const auto* e = dynamic_cast<const MouseButtonPressedEvent*>(&event);
                if (!e) {
                    return;
                }
                VNE_LOG_INFO << tag << " [MOUSE] BTN DOWN at (" << std::fixed << std::setprecision(0) << e->x() << ", "
                             << e->y() << ")";
                return;
            }
            case EventType::eMouseMoved: {
                const auto* e = dynamic_cast<const MouseMovedEvent*>(&event);
                if (!e || !Input::isMouseButtonPressed(0)) {
                    return;
                }
                VNE_LOG_INFO << tag << " [MOUSE] DRAG at (" << std::fixed << std::setprecision(0) << e->x() << ", "
                             << e->y() << ")";
                return;
            }
            case EventType::eWindowFocus: {
                const auto* e = dynamic_cast<const WindowFocusEvent*>(&event);
                if (!e) {
                    return;
                }
                VNE_LOG_INFO << tag << " [WINDOW] FOCUS " << (e->focused() ? "gained" : "lost");
                return;
            }
            case EventType::eWindowClose: {
                VNE_LOG_INFO << tag << " [WINDOW] CLOSE";
                titles_.erase(event.windowId());
                return;
            }
            case EventType::eWindowResize: {
                const auto* e = dynamic_cast<const WindowResizeEvent*>(&event);
                if (!e) {
                    return;
                }
                VNE_LOG_INFO << tag << " [WINDOW] RESIZE " << e->width() << "x" << e->height();
                return;
            }
            default:
                return;
        }
    }

    bool onFrame(float /*dt*/) override { return true; }

    void onShutdown() override { logWindowList("shutdown"); }

   private:
    void handleCommand(vne::events::KeyCode key) {
        using vne::events::KeyCode;
        switch (key) {
            case KeyCode::eN:
                openChild();
                break;
            case KeyCode::eF:
                cycleFocus();
                break;
            case KeyCode::eP:
                setPrimaryFocused();
                break;
            case KeyCode::eC:
                closeFocused();
                break;
            default:
                break;
        }
    }

    void logHelp() const {
        VNE_LOG_INFO << "Controls: N=new  F=focus next  P=set primary  C=close focused  ESC=quit";
        VNE_LOG_INFO << "Two demo child windows open at startup (cascaded). Press N for more.";
        VNE_LOG_INFO << "Click/type in different windows to see event routing.";
    }

    void logWindowList(const char* phase) const {
        if (!mgr_) {
            return;
        }
        const auto windows = mgr_->getWindows();
        VNE_LOG_INFO << "--- " << phase << " - windows: " << windows.size() << " ---";
        const auto primary = mgr_->getPrimaryWindow();
        const auto focused = mgr_->getFocusedWindow();
        for (const auto& w : windows) {
            if (!w) {
                continue;
            }
            const auto id = w->getId();
            std::string title = "Child";
            const auto it = titles_.find(id);
            if (it != titles_.end()) {
                title = it->second;
            }
            std::string flags;
            if (primary && primary->getId() == id) {
                flags += " primary";
            }
            if (focused && focused->getId() == id) {
                flags += " focused";
            }
            VNE_LOG_INFO << "  [id=" << static_cast<std::uint32_t>(id) << " title=\"" << title << "\"" << flags << "]";
        }
    }

    void openChild() {
        if (!mgr_) {
            return;
        }

        const std::string title = "Child " + std::to_string(++child_counter_);
        vne::xwin::WindowDescriptor desc(title, 400U, 300U);
        desc.focused = false;

        auto child = mgr_->openWindow(desc);
        if (!child) {
            VNE_LOG_WARN << "openWindow(\"" << title
                         << "\") failed - multi-window may be unsupported on this platform.";
            return;
        }

        // Cascade so title bars stay visible. On WASM the shell exits fill-mode and
        // places the primary at (24,24); keep children offset from that origin.
        if (main_) {
            const auto p = main_->getPosition();
            const int base_x = (p.x == 0 && p.y == 0) ? 24 : p.x;
            const int base_y = (p.x == 0 && p.y == 0) ? 24 : p.y;
            const int offset = static_cast<int>(child_counter_) * 40;
            child->setPosition(base_x + offset, base_y + offset);
        }

        titles_[child->getId()] = title;
        VNE_LOG_INFO << "Opened " << title << " id=" << static_cast<std::uint32_t>(child->getId()) << " at ("
                     << child->getPosition().x << ", " << child->getPosition().y << ")";
        logWindowList("after open");
    }

    void cycleFocus() {
        if (!mgr_) {
            return;
        }
        const auto windows = mgr_->getWindows();
        if (windows.size() <= 1U) {
            VNE_LOG_INFO << "Only one window open - nothing to focus.";
            return;
        }

        const auto focused = mgr_->getFocusedWindow();
        auto it = windows.begin();
        if (focused) {
            it = std::find_if(windows.begin(), windows.end(), [&](const std::shared_ptr<vne::xwin::IWindow>& w) {
                return w && w->getId() == focused->getId();
            });
            if (it != windows.end()) {
                ++it;
                if (it == windows.end()) {
                    it = windows.begin();
                }
            } else {
                it = windows.begin();
            }
        }

        while (it != windows.end() && !*it) {
            ++it;
        }
        if (it == windows.end() || !*it) {
            return;
        }

        mgr_->focusWindow(*it);
        VNE_LOG_INFO << "Focused id=" << static_cast<std::uint32_t>((*it)->getId());
        logWindowList("after focus");
    }

    void setPrimaryFocused() {
        if (!mgr_) {
            return;
        }
        const auto focused = mgr_->getFocusedWindow();
        if (!focused) {
            VNE_LOG_INFO << "No focused window.";
            return;
        }
        mgr_->setPrimaryWindow(focused);
        VNE_LOG_INFO << "Primary set to id=" << static_cast<std::uint32_t>(focused->getId());
        logWindowList("after set primary");
    }

    void closeFocused() {
        if (!mgr_) {
            return;
        }
        const auto focused = mgr_->getFocusedWindow();
        if (!focused) {
            VNE_LOG_INFO << "No focused window to close.";
            return;
        }
        if (main_ && focused->getId() == main_->getId()) {
            VNE_LOG_INFO << "Closing primary window - app will exit.";
        }

        const auto id = focused->getId();
        VNE_LOG_INFO << "Closing id=" << static_cast<std::uint32_t>(id);
        mgr_->removeWindow(focused);
        titles_.erase(id);
        logWindowList("after close");
    }

    vne::xwin::IWindowManager* mgr_ = nullptr;
    std::shared_ptr<vne::xwin::IWindow> main_;
    std::unordered_map<vne::events::WindowId, std::string> titles_;
    std::uint32_t child_counter_ = 0U;
};

std::unique_ptr<vne::xwin::examples::ExampleBase> createExample() {
    return std::make_unique<MultiWindowExample>();
}
