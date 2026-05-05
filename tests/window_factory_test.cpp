/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   April 2026
 *
 * Autodoc:   yes
 * Tests for src/vertexnova/xwin/window_factory.cpp (factory + desktop smoke via factory).
 * ----------------------------------------------------------------------
 */

#include <cstdlib>
#include <gtest/gtest.h>

#include "vertexnova/xwin/window_factory.h"
#include "vertexnova/xwin/xwin_types.h"

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

using vne::xwin::WindowAPI_TP;
using vne::xwin::WindowFactory_C;

class WindowFactory_CTest : public ::testing::Test {};

TEST_F(WindowFactory_CTest, GetBuildInfo) {
    const std::string info = WindowFactory_C::GetBuildInfo();
    EXPECT_FALSE(info.empty());
}

TEST_F(WindowFactory_CTest, NullBackendCreatesOpenWindow) {
    auto mgr = WindowFactory_C::CreateWindowManager(WindowAPI_TP::NULL_WINDOW);
    ASSERT_NE(mgr, nullptr);
    EXPECT_TRUE(mgr->Initialize());
    auto w = mgr->CreateWindow("test", 64, 48);
    ASSERT_NE(w, nullptr);
    EXPECT_TRUE(w->IsOpen());
    EXPECT_EQ(w->GetWindowAPI(), WindowAPI_TP::NULL_WINDOW);
    mgr->Shutdown();
}

namespace {

bool try_desktop_smoke(WindowAPI_TP api) {
    auto mgr = WindowFactory_C::CreateWindowManager(api);
    if (!mgr) {
        return false;
    }
    if (!mgr->Initialize()) {
        return false;
    }
    auto w = mgr->CreateWindow("vnexwin_smoke", 32, 32);
    if (!w || !w->IsOpen()) {
        mgr->Shutdown();
        return false;
    }

    w->PollEvents();
    w->Resize(64, 48);
    (void)w->GetNativeWindow();
    const auto handle = w->GetNativeHandle();
    EXPECT_EQ(handle.api, api);
    (void)w->GetDPIScale();
    (void)w->GetFramebufferWidth();
    (void)w->GetFramebufferHeight();

    mgr->DestroyWindow(w);
    mgr->Shutdown();
    return true;
}

}  // namespace

TEST_F(WindowFactory_CTest, DesktopLifecycleWhenBackendAvailable) {
#if defined(__APPLE__) && TARGET_OS_IPHONE
    GTEST_SKIP() << "Desktop smoke targets macOS/Win/Linux hosts; iOS uses UIKit.";
#endif

#if defined(__APPLE__) && TARGET_OS_OSX
    if (try_desktop_smoke(WindowAPI_TP::COCOA_WINDOW)) {
        return;
    }
#elif defined(_WIN32)
    if (try_desktop_smoke(WindowAPI_TP::WIN32_WINDOW)) {
        return;
    }
#elif defined(__linux__)
    const char* display = std::getenv("DISPLAY");
    const char* wayland = std::getenv("WAYLAND_DISPLAY");
    if (display != nullptr && display[0] != '\0') {
        if (try_desktop_smoke(WindowAPI_TP::X11_WINDOW)) {
            return;
        }
    }
    if (wayland != nullptr && wayland[0] != '\0') {
        if (try_desktop_smoke(WindowAPI_TP::WAYLAND_WINDOW)) {
            return;
        }
    }
    if (try_desktop_smoke(WindowAPI_TP::X11_WINDOW)) {
        return;
    }
    if (try_desktop_smoke(WindowAPI_TP::WAYLAND_WINDOW)) {
        return;
    }
#endif

    GTEST_SKIP()
        << "No desktop window backend initialized (not built, or no DISPLAY/WAYLAND_DISPLAY / Cocoa unavailable).";
}
