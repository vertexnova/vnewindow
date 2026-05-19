/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   April 2026
 *
 * Autodoc:   yes
 * Tests for src/vertexnova/xwin/platform/null/null_window.cpp (via eNullWindow factory).
 * ----------------------------------------------------------------------
 */

#include <memory>

#include <gtest/gtest.h>

#include "vertexnova/xwin/window_factory.h"
#include "vertexnova/xwin/xwin_types.h"

using vne::xwin::WindowAPI;
using vne::xwin::WindowFactory;
using vne::xwin::WindowMode;

namespace {

std::shared_ptr<vne::xwin::IWindowManager> MakeInitializedNullManager() {
    auto mgr = WindowFactory::createWindowManager(WindowAPI::eNullWindow);
    EXPECT_NE(mgr, nullptr);
    if (!mgr) {
        return nullptr;
    }
    EXPECT_TRUE(mgr->initialize());
    return mgr;
}

}  // namespace

class NullWindowTest : public ::testing::Test {};

TEST_F(NullWindowTest, NativeHandleFieldsAreDefaulted) {
    auto mgr = MakeInitializedNullManager();
    ASSERT_NE(mgr, nullptr);
    auto w = mgr->openWindow("test", 64, 48);
    ASSERT_NE(w, nullptr);
    const auto handle = w->getNativeHandle();
    EXPECT_EQ(handle.api, WindowAPI::eNullWindow);
    EXPECT_EQ(handle.hwnd, nullptr);
    EXPECT_EQ(handle.ns_view, nullptr);
    EXPECT_EQ(handle.ns_window, nullptr);
    EXPECT_EQ(handle.ca_layer, nullptr);
    EXPECT_EQ(handle.ui_view, nullptr);
    EXPECT_EQ(handle.ui_window, nullptr);
    EXPECT_EQ(handle.x11_display, nullptr);
    EXPECT_EQ(handle.xcb_connection, nullptr);
    EXPECT_EQ(handle.wl_display, nullptr);
    EXPECT_EQ(handle.wl_surface, nullptr);
    EXPECT_EQ(handle.a_native_window, nullptr);
    EXPECT_EQ(handle.canvas_id, nullptr);
    EXPECT_EQ(handle.x11_window_id, 0U);
    EXPECT_EQ(handle.xcb_window_id, 0U);
    w->close();
    mgr->shutdown();
}

TEST_F(NullWindowTest, CreatePollResizeClose) {
    auto mgr = MakeInitializedNullManager();
    ASSERT_NE(mgr, nullptr);
    auto w = mgr->openWindow("life", 128, 96);
    ASSERT_NE(w, nullptr);
    w->pollEvents();
    w->swapBuffers();
    w->resize(200, 100);
    EXPECT_EQ(w->getWidth(), 200);
    EXPECT_EQ(w->getHeight(), 100);
    w->close();
    EXPECT_FALSE(w->isOpen());
    mgr->shutdown();
}

TEST_F(NullWindowTest, SetModeAndFullscreen) {
    auto mgr = MakeInitializedNullManager();
    ASSERT_NE(mgr, nullptr);
    auto w = mgr->openWindow("mode", 100, 80);
    ASSERT_NE(w, nullptr);
    EXPECT_EQ(w->getWindowMode(), WindowMode::eWindowed);

    w->setWindowMode(WindowMode::eFullscreen);
    EXPECT_EQ(w->getWindowMode(), WindowMode::eFullscreen);
    w->setFullscreen(true);
    EXPECT_TRUE(w->isFullscreen());

    w->setWindowMode(WindowMode::eBorderless);
    EXPECT_EQ(w->getWindowMode(), WindowMode::eBorderless);

    w->setWindowMode(WindowMode::eWindowed);
    w->setFullscreen(false);
    EXPECT_EQ(w->getWindowMode(), WindowMode::eWindowed);
    EXPECT_FALSE(w->isFullscreen());

    w->close();
    mgr->shutdown();
}

TEST_F(NullWindowTest, MinimizeMaximizeRestoreNoop) {
    auto mgr = MakeInitializedNullManager();
    ASSERT_NE(mgr, nullptr);
    auto w = mgr->openWindow("mmr", 32, 32);
    ASSERT_NE(w, nullptr);
    w->minimize();
    w->maximize();
    w->restore();
    w->close();
    mgr->shutdown();
}

TEST_F(NullWindowTest, DpiAndFramebufferDefaults) {
    auto mgr = MakeInitializedNullManager();
    ASSERT_NE(mgr, nullptr);
    auto w = mgr->openWindow("dpi", 100, 50);
    ASSERT_NE(w, nullptr);
    EXPECT_FLOAT_EQ(w->getDpiScale(), 1.0F);
    EXPECT_EQ(w->getWidth(), 100);
    EXPECT_EQ(w->getHeight(), 50);
    EXPECT_EQ(w->getFramebufferWidth(), 100U);
    EXPECT_EQ(w->getFramebufferHeight(), 50U);

    w->resize(200, 40);
    EXPECT_EQ(w->getFramebufferWidth(), 200U);
    EXPECT_EQ(w->getFramebufferHeight(), 40U);

    w->close();
    mgr->shutdown();
}

TEST_F(NullWindowTest, ClipboardRoundTripDefaultNoOp) {
    auto mgr = MakeInitializedNullManager();
    ASSERT_NE(mgr, nullptr);
    auto w = mgr->openWindow("clip", 16, 16);
    ASSERT_NE(w, nullptr);
    EXPECT_TRUE(w->getClipboardText().empty());
    w->setClipboardText("hello");
    EXPECT_TRUE(w->getClipboardText().empty());
    w->close();
    mgr->shutdown();
}
