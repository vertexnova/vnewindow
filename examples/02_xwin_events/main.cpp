/*
 * Demonstrates vne::events integration: optional EventBridgeCallbacks struct,
 * EventManager::processEvents(), and Input::nextFrame() after ProcessEvents().
 * Close the window to exit (null backend exits immediately).
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

    auto mgr = WindowFactory::CreateWindowManager();
    if (!mgr || !mgr->Initialize()) {
        VNE_LOG_ERROR << "No window manager (see WindowFactory::GetLastError)";
        return 1;
    }

    vne::xwin::WindowDescriptor desc("xwin_events", 640, 480);
    desc.enable_events = true;
    desc.enable_input = true;

    auto w = mgr->CreateWindow(desc);
    if (!w) {
        VNE_LOG_ERROR << "CreateWindow failed";
        mgr->Shutdown();
        return 1;
    }

    vne::xwin::EventBridgeCallbacks hooks{};
    hooks.onKeyDown = [](vne::xwin::IWindow* win, vne::events::KeyCode key, std::uint8_t /*mods*/, bool repeat) {
        VNE_LOG_INFO << "onKeyDown window=" << win << " key=" << static_cast<int>(key) << " repeat=" << repeat;
    };

    mgr->setEventBridgeCallbacks(std::move(hooks));

    if (w->GetWindowAPI() == vne::xwin::WindowAPI::eNullWindow) {
        VNE_LOG_INFO << "Null backend has no native input; closing window for a clean smoke run.";
        mgr->DestroyWindow(w);
    }

    if (mgr->GetWindowCount() == 0U) {
        mgr->Shutdown();
        return 0;
    }

    VNE_LOG_INFO << "Event loop: move mouse / type keys; close window to quit. "
                    "Call vne::events::Input::nextFrame() once per frame after your update logic.";

    while (mgr->GetWindowCount() > 0U && !mgr->ShouldClose()) {
        mgr->ProcessEvents();
        vne::events::EventManager::instance().processEvents();
        vne::events::Input::nextFrame();
        mgr->Sleep(16);
    }

    mgr->Shutdown();
    return 0;
}
