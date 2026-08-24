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

/**
 * 02_xwin_events - vne::xwin + vne::events integration example.
 *
 * Logs keyboard, mouse, touch, window-state and application-lifecycle events.
 *
 * There is one event path: every native event arrives as a vne::events::Event carrying the id of
 * the window that produced it.
 */

#include "common/example_base.h"

#include "vertexnova/events/events.h"

#include <vertexnova/logging/logging.h>

#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace {

std::string keyName(vne::events::KeyCode key) {
    using vne::events::KeyCode;
    switch (key) {
        case KeyCode::eSpace:
            return "Space";
        case KeyCode::eEnter:
            return "Enter";
        case KeyCode::eEscape:
            return "Escape";
        case KeyCode::eTab:
            return "Tab";
        case KeyCode::eBackspace:
            return "Backspace";
        case KeyCode::eLeft:
            return "Left";
        case KeyCode::eRight:
            return "Right";
        case KeyCode::eUp:
            return "Up";
        case KeyCode::eDown:
            return "Down";
        case KeyCode::eLeftShift:
        case KeyCode::eRightShift:
            return "Shift";
        case KeyCode::eLeftControl:
        case KeyCode::eRightControl:
            return "Ctrl";
        case KeyCode::eLeftAlt:
        case KeyCode::eRightAlt:
            return "Alt";
        case KeyCode::eF1:
            return "F1";
        case KeyCode::eF2:
            return "F2";
        case KeyCode::eF3:
            return "F3";
        case KeyCode::eF4:
            return "F4";
        case KeyCode::eF5:
            return "F5";
        case KeyCode::eF6:
            return "F6";
        case KeyCode::eF7:
            return "F7";
        case KeyCode::eF8:
            return "F8";
        case KeyCode::eF9:
            return "F9";
        case KeyCode::eF10:
            return "F10";
        case KeyCode::eF11:
            return "F11";
        case KeyCode::eF12:
            return "F12";
        default:
            break;
    }

    const int code = static_cast<int>(key);
    if (code >= 32 && code <= 126) {
        return std::string(1, static_cast<char>(code));
    }

    return "Key(" + std::to_string(code) + ")";
}

std::string modNames(std::uint8_t mods) {
    using vne::events::eModAlt;
    using vne::events::eModCmd;
    using vne::events::eModCtrl;
    using vne::events::eModMeta;
    using vne::events::eModShift;
    using vne::events::eModSuper;

    std::string out;
    const auto append = [&out](std::string_view n) {
        if (!out.empty()) {
            out += "+";
        }
        out += n;
    };

    if ((mods & eModCtrl) != 0U) {
        append("Ctrl");
    }
    if ((mods & eModShift) != 0U) {
        append("Shift");
    }
    if ((mods & eModAlt) != 0U) {
        append("Alt");
    }
    if ((mods & eModMeta) != 0U) {
        append("Meta");
    }
    if ((mods & eModSuper) != 0U) {
        append("Super");
    }
    if ((mods & eModCmd) != 0U) {
        append("Cmd");
    }

    return out.empty() ? "none" : out;
}

const char* btnName(vne::events::MouseButton btn) {
    using vne::events::MouseButton;
    switch (btn) {
        case MouseButton::eLeft:
            return "Left";
        case MouseButton::eRight:
            return "Right";
        case MouseButton::eMiddle:
            return "Middle";
        default:
            return nullptr;
    }
}

std::string btnDisplayName(vne::events::MouseButton btn) {
    if (const char* common = btnName(btn)) {
        return common;
    }
    return "Button" + std::to_string(static_cast<int>(btn));
}

std::string formatPoint(double x, double y) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(1) << x << ", " << y;
    return ss.str();
}

}  // namespace

class XwinEventsExample final : public vne::xwin::examples::ExampleBase {
   public:
    vne::xwin::examples::ExampleConfig configure() override {
        return {"02 XWin Events - Comprehensive Input", 1000, 700};
    }

    void onInit(vne::xwin::IWindow& /*window*/, vne::xwin::IWindowManager& mgr) override {
        VNE_LOG_INFO << "02_xwin_events demo ready.";
        VNE_LOG_INFO << "Try keyboard, mouse, touch, resize/focus/minimize, and hold movement keys.";
        VNE_LOG_INFO << "Every line below is one vne::events event.";
        VNE_LOG_INFO << "Runner API: window=" << static_cast<int>(mgr.getPrimaryWindow()->getWindowAPI())
                     << " platform=" << mgr.getPlatformInfo();
    }

    void onEvent(const vne::events::Event& event) override {
        using namespace vne::events;

        switch (event.type()) {
            case EventType::eWindowClose: {
                VNE_LOG_INFO << "[WINDOW] CLOSE";
                return;
            }
            case EventType::eKeyPressed: {
                const auto* e = dynamic_cast<const KeyPressedEvent*>(&event);
                if (!e) {
                    return;
                }
                VNE_LOG_INFO << "[KEY   ] " << std::left << std::setw(8) << "DOWN" << " " << std::setw(12)
                             << keyName(e->keyCode()) << " mods: " << std::setw(14) << modNames(e->modifiers())
                             << " repeat: no";
                return;
            }
            case EventType::eKeyReleased: {
                const auto* e = dynamic_cast<const KeyReleasedEvent*>(&event);
                if (!e) {
                    return;
                }
                VNE_LOG_INFO << "[KEY   ] " << std::left << std::setw(8) << "UP" << " " << keyName(e->keyCode());
                return;
            }
            case EventType::eKeyRepeat: {
                const auto* e = dynamic_cast<const KeyRepeatEvent*>(&event);
                if (!e) {
                    return;
                }
                VNE_LOG_INFO << "[KEY   ] " << std::left << std::setw(8) << "REPEAT" << " " << std::setw(12)
                             << keyName(e->keyCode()) << " count: " << e->repeatCount();
                return;
            }
            case EventType::eTextInput: {
                const auto* e = dynamic_cast<const TextInputEvent*>(&event);
                if (!e) {
                    return;
                }
                VNE_LOG_INFO << "[TEXT  ] " << std::left << std::setw(8) << "INPUT" << " \"" << e->text() << "\"";
                return;
            }
            case EventType::eMouseButtonPressed:
            case EventType::eMouseButtonReleased:
            case EventType::eMouseButtonDoubleClicked: {
                const auto* pressed = dynamic_cast<const MouseButtonPressedEvent*>(&event);
                const auto* released = dynamic_cast<const MouseButtonReleasedEvent*>(&event);
                const auto* dbl = dynamic_cast<const MouseButtonDoubleClickedEvent*>(&event);
                const MouseButtonEvent* e = pressed ? static_cast<const MouseButtonEvent*>(pressed)
                                                    : (released ? static_cast<const MouseButtonEvent*>(released)
                                                                : static_cast<const MouseButtonEvent*>(dbl));
                if (!e) {
                    return;
                }
                const char* state = (event.type() == EventType::eMouseButtonPressed)    ? "DOWN"
                                    : (event.type() == EventType::eMouseButtonReleased) ? "UP"
                                                                                        : "DOUBLE";
                if (e->hasPosition()) {
                    VNE_LOG_INFO << "[MOUSE ] BTN      " << std::setw(7) << btnDisplayName(e->button()) << " "
                                 << std::setw(6) << state << " at (" << formatPoint(e->x(), e->y()) << ")";
                } else {
                    VNE_LOG_INFO << "[MOUSE ] BTN      " << std::setw(7) << btnDisplayName(e->button()) << " " << state;
                }
                return;
            }
            case EventType::eMouseMoved: {
                const auto* e = dynamic_cast<const MouseMovedEvent*>(&event);
                if (!e) {
                    return;
                }
                if (!vne::events::Input::isMouseButtonPressed(0)) {
                    return;
                }
                VNE_LOG_INFO << "[MOUSE ] MOVED    x=" << std::fixed << std::setprecision(1) << e->x()
                             << "  y=" << e->y();
                return;
            }
            case EventType::eMouseScrolled: {
                const auto* e = dynamic_cast<const MouseScrolledEvent*>(&event);
                if (!e) {
                    return;
                }
                VNE_LOG_INFO << "[MOUSE ] SCROLL   dx=" << std::fixed << std::setprecision(1) << e->xOffset()
                             << "  dy=" << e->yOffset();
                return;
            }
            case EventType::eTouchPress:
            case EventType::eTouchMove:
            case EventType::eTouchRelease: {
                const auto* press = dynamic_cast<const TouchPressEvent*>(&event);
                const auto* move = dynamic_cast<const TouchMoveEvent*>(&event);
                const auto* release = dynamic_cast<const TouchReleaseEvent*>(&event);
                const TouchEvent* e =
                    press ? static_cast<const TouchEvent*>(press)
                          : (move ? static_cast<const TouchEvent*>(move) : static_cast<const TouchEvent*>(release));
                if (!e) {
                    return;
                }
                const char* phase = (event.type() == EventType::eTouchPress)     ? "BEGIN"
                                    : (event.type() == EventType::eTouchRelease) ? "END"
                                                                                 : "MOVED";
                VNE_LOG_INFO << "[TOUCH ] " << std::left << std::setw(8) << phase << " id=" << e->touchId() << " at ("
                             << formatPoint(e->x(), e->y()) << ")";
                return;
            }
            case EventType::eWindowResize: {
                const auto* e = dynamic_cast<const WindowResizeEvent*>(&event);
                if (!e) {
                    return;
                }
                VNE_LOG_INFO << "[WINDOW] RESIZE   " << e->width() << "x" << e->height();
                return;
            }
            case EventType::eWindowFocus: {
                const auto* e = dynamic_cast<const WindowFocusEvent*>(&event);
                if (!e) {
                    return;
                }
                VNE_LOG_INFO << "[WINDOW] FOCUS    " << (e->focused() ? "gained" : "lost");
                return;
            }
            case EventType::eWindowMinimize: {
                VNE_LOG_INFO << "[WINDOW] MINIMIZE";
                return;
            }
            case EventType::eWindowRestore: {
                VNE_LOG_INFO << "[WINDOW] RESTORE";
                return;
            }
            case EventType::eWindowMove: {
                const auto* e = dynamic_cast<const WindowMoveEvent*>(&event);
                if (!e) {
                    return;
                }
                VNE_LOG_INFO << "[WINDOW] MOVE     (" << e->x() << ", " << e->y() << ")";
                return;
            }
            case EventType::eWindowDpiChanged: {
                const auto* e = dynamic_cast<const WindowDpiChangedEvent*>(&event);
                if (!e) {
                    return;
                }
                VNE_LOG_INFO << "[WINDOW] DPI      scale=" << e->scale();
                return;
            }
            case EventType::eWindowSafeAreaChanged: {
                const auto* e = dynamic_cast<const WindowSafeAreaChangedEvent*>(&event);
                if (!e) {
                    return;
                }
                VNE_LOG_INFO << "[WINDOW] SAFEAREA top=" << e->top() << " left=" << e->left()
                             << " bottom=" << e->bottom() << " right=" << e->right();
                return;
            }
            case EventType::eApplicationPause: {
                VNE_LOG_INFO << "[APP   ] PAUSE";
                return;
            }
            case EventType::eApplicationResume: {
                VNE_LOG_INFO << "[APP   ] RESUME";
                return;
            }
            case EventType::eApplicationLowMemory: {
                VNE_LOG_INFO << "[APP   ] LOW MEMORY";
                return;
            }
            default:
                return;
        }
    }

    bool onFrame(float /*dt*/) override {
        if (should_exit_) {
            return false;
        }

        ++frame_count_;
        if ((frame_count_ % 60U) != 0U) {
            return true;
        }

        using vne::events::Input;
        using vne::events::KeyCode;

        const std::vector<std::pair<KeyCode, const char*>> watch = {
            {KeyCode::eSpace, "Space"},
            {KeyCode::eW, "W"},
            {KeyCode::eA, "A"},
            {KeyCode::eS, "S"},
            {KeyCode::eD, "D"},
            {KeyCode::eLeft, "Left"},
            {KeyCode::eRight, "Right"},
            {KeyCode::eUp, "Up"},
            {KeyCode::eDown, "Down"},
            {KeyCode::eLeftShift, "Shift"},
            {KeyCode::eLeftControl, "Ctrl"},
        };

        std::string held;
        for (const auto& [key, name] : watch) {
            if (Input::isKeyPressed(static_cast<int>(key))) {
                if (!held.empty()) {
                    held += "  ";
                }
                held += name;
            }
        }

        if (!held.empty()) {
            VNE_LOG_INFO << "[POLL  ] Held: " << held;
        }
        return true;
    }

   private:
    std::uint32_t frame_count_ = 0U;
    bool should_exit_ = false;
};

std::unique_ptr<vne::xwin::examples::ExampleBase> createExample() {
    return std::make_unique<XwinEventsExample>();
}
