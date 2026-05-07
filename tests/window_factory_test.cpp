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

using vne::xwin::WindowAPI;
using vne::xwin::WindowFactory;

class WindowFactoryTest : public ::testing::Test {};

TEST_F(WindowFactoryTest, getBuildInfo) {
    const std::string info = WindowFactory::getBuildInfo();
    EXPECT_FALSE(info.empty());
}

TEST_F(WindowFactoryTest, NullBackendOpenWindow) {
    auto mgr = WindowFactory::createWindowManager(WindowAPI::eNullWindow);
    ASSERT_NE(mgr, nullptr);
    EXPECT_TRUE(mgr->initialize());
    auto w = mgr->openWindow("test", 64, 48);
    ASSERT_NE(w, nullptr);
    EXPECT_TRUE(w->isOpen());
    EXPECT_EQ(w->getWindowAPI(), WindowAPI::eNullWindow);
    mgr->shutdown();
}

namespace {

bool try_desktop_smoke(WindowAPI api) {
    auto mgr = WindowFactory::createWindowManager(api);
    if (!mgr) {
        return false;
    }
    if (!mgr->initialize()) {
        return false;
    }
    auto w = mgr->openWindow("vnexwin_smoke", 32, 32);
    if (!w || !w->isOpen()) {
        mgr->shutdown();
        return false;
    }

    w->pollEvents();
    w->resize(64, 48);
    const auto handle = w->getNativeHandle();
    EXPECT_EQ(handle.api, api);
    (void)w->getDpiScale();
    (void)w->getFramebufferWidth();
    (void)w->getFramebufferHeight();

    mgr->removeWindow(w);
    mgr->shutdown();
    return true;
}

}  // namespace

TEST_F(WindowFactoryTest, DesktopLifecycleWhenBackendAvailable) {
#if defined(__APPLE__) && TARGET_OS_IPHONE
    GTEST_SKIP() << "Desktop smoke targets macOS/Win/Linux hosts; iOS uses UIKit.";
#endif

#if defined(__APPLE__) && TARGET_OS_OSX
    if (try_desktop_smoke(WindowAPI::eCocoaWindow)) {
        return;
    }
#elif defined(_WIN32)
    if (try_desktop_smoke(WindowAPI::eWin32Window)) {
        return;
    }
#elif defined(__linux__)
    const char* display = std::getenv("DISPLAY");
    const char* wayland = std::getenv("WAYLAND_DISPLAY");
    if (display != nullptr && display[0] != '\0') {
        if (try_desktop_smoke(WindowAPI::eX11Window)) {
            return;
        }
    }
    if (wayland != nullptr && wayland[0] != '\0') {
        if (try_desktop_smoke(WindowAPI::eWaylandWindow)) {
            return;
        }
    }
    if (try_desktop_smoke(WindowAPI::eX11Window)) {
        return;
    }
    if (try_desktop_smoke(WindowAPI::eWaylandWindow)) {
        return;
    }
#endif

    GTEST_SKIP()
        << "No desktop window backend initialized (not built, or no DISPLAY/WAYLAND_DISPLAY / Cocoa unavailable).";
}
