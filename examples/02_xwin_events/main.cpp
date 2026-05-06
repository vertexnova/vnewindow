/*
 * Demonstrates vne::events integration: optional EventBridgeCallbacks struct,
 * EventManager::processEvents(), and Input::nextFrame() after processEvents().
 * close the window to exit (null backend exits immediately).
 */
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

#include "common/logging_guard.h"

#include "vertexnova/events/events.h"
#include "vertexnova/xwin/window_factory.h"
#include "vertexnova/xwin/window_descriptor.h"
#include "vertexnova/xwin/xwin_types.h"
#include "vertexnova/xwin/event_bridge_callbacks.h"

#include <cstdint>

int main() {
    vne::xwin::examples::LoggingGuard logging_guard;

    using vne::xwin::WindowFactory;

    auto mgr = WindowFactory::createWindowManager();
    if (!mgr || !mgr->initialize()) {
        VNE_LOG_ERROR << "No window manager (see WindowFactory::getLastError)";
        return 1;
    }

    vne::xwin::WindowDescriptor desc("xwin_events", 640, 480);
    desc.enable_events = true;
    desc.enable_input = true;

    auto w = mgr->openWindow(desc);
    if (!w) {
        VNE_LOG_ERROR << "openWindow failed";
        mgr->shutdown();
        return 1;
    }

    vne::xwin::EventBridgeCallbacks hooks{};
    hooks.on_key_down = [](vne::xwin::IWindow* win, vne::events::KeyCode key, std::uint8_t /*mods*/, bool repeat) {
        VNE_LOG_INFO << "on_key_down window=" << win << " key=" << static_cast<int>(key) << " repeat=" << repeat;
    };

    mgr->setEventBridgeCallbacks(std::move(hooks));

    if (w->getWindowAPI() == vne::xwin::WindowAPI::eNullWindow) {
        VNE_LOG_INFO << "Null backend has no native input; closing window for a clean smoke run.";
        mgr->removeWindow(w);
    }

    if (mgr->getWindowCount() == 0U) {
        mgr->shutdown();
        return 0;
    }

    VNE_LOG_INFO << "Event loop: move mouse / type keys; close window to quit. "
                    "Call vne::events::Input::nextFrame() once per frame after your update logic.";

    while (mgr->getWindowCount() > 0U && !mgr->shouldClose()) {
        mgr->processEvents();
        vne::events::EventManager::instance().processEvents();
        vne::events::Input::nextFrame();
        mgr->sleep(16);
    }

    mgr->shutdown();
    return 0;
}
