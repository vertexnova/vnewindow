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
 * 02_xwin_events — vne::events integration example.
 *
 * Demonstrates EventBridgeCallbacks for raw key / mouse events and the
 * per-frame Input::nextFrame() contract. Works on all platforms; on iOS and
 * other touch-first platforms the touch callbacks are routed through the same
 * event bridge infrastructure.
 */

#include "common/example_base.h"

#include "vertexnova/events/events.h"
#include "vertexnova/xwin/event_bridge_callbacks.h"
#include "vertexnova/xwin/xwin_types.h"

#include <vertexnova/logging/logging.h>
#include <cstdint>

class XwinEventsExample final : public vne::xwin::examples::ExampleBase {
   public:
    vne::xwin::examples::ExampleConfig configure() override { return {"XWin Events", 800, 600}; }

    void onInit(vne::xwin::IWindow& /*window*/, vne::xwin::IWindowManager& mgr) override {
        // Wire raw event callbacks; these fire synchronously inside processEvents()
        vne::xwin::EventBridgeCallbacks hooks{};

        hooks.on_key_down = [](vne::xwin::IWindow* win, vne::events::KeyCode key, std::uint8_t mods, bool repeat) {
            VNE_LOG_INFO << "key_down  win=" << win << " key=" << static_cast<int>(key)
                         << " mods=" << static_cast<int>(mods) << " repeat=" << repeat;
        };

        hooks.on_key_up = [](vne::xwin::IWindow* win, vne::events::KeyCode key, std::uint8_t mods) {
            VNE_LOG_INFO << "key_up    win=" << win << " key=" << static_cast<int>(key)
                         << " mods=" << static_cast<int>(mods);
        };

        hooks.on_mouse_move = [](vne::xwin::IWindow* win, double x, double y, std::uint8_t /*mods*/) {
            (void)win;
            (void)x;
            (void)y;  // uncomment to enable verbose mouse-move logs
        };

        hooks.on_mouse_button = [](vne::xwin::IWindow* win,
                                   vne::events::MouseButton btn,
                                   bool pressed,
                                   double x,
                                   double y,
                                   std::uint8_t /*mods*/) {
            VNE_LOG_INFO << "mouse_btn win=" << win << " btn=" << static_cast<int>(btn) << " pressed=" << pressed
                         << " pos=(" << x << "," << y << ")";
        };

        hooks.on_touch = [](vne::xwin::IWindow* /*win*/,
                            std::uint32_t touch_id,
                            double x,
                            double y,
                            vne::xwin::EventBridgeTouchPhase phase) {
            VNE_LOG_INFO << "touch id=" << touch_id << " phase=" << static_cast<int>(phase) << " pos=(" << x << "," << y
                         << ")";
        };

        mgr.setEventBridgeCallbacks(std::move(hooks));

        VNE_LOG_INFO << "02_xwin_events ready. Type / click / touch. ESC or close to exit.";
        VNE_LOG_INFO << "Input::nextFrame() is called automatically each frame by ExampleRunner.";
    }

    bool onFrame(float /*dt*/) override {
        // vne::events::Input queries can be polled here each frame, e.g.:
        // if (vne::events::Input::isKeyDown(vne::events::KeyCode::eSpace)) { ... }
        return true;
    }
};

std::unique_ptr<vne::xwin::examples::ExampleBase> createExample() {
    return std::make_unique<XwinEventsExample>();
}
