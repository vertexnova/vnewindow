/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   April 2026
 *
 * Autodoc:   yes
 * Tests for src/vertexnova/xwin/platform/null/null_window.cpp (via NULL_WINDOW factory).
 * ----------------------------------------------------------------------
 */

#include <memory>

#include <gtest/gtest.h>

#include "vertexnova/xwin/window_factory.h"
#include "vertexnova/xwin/xwin_types.h"

using vne::xwin::WindowAPI_TP;
using vne::xwin::WindowFactory_C;
using vne::xwin::WindowMode_TP;

namespace {

std::shared_ptr<vne::xwin::WindowManager_I> MakeInitializedNullManager() {
    auto mgr = WindowFactory_C::CreateWindowManager(WindowAPI_TP::NULL_WINDOW);
    EXPECT_NE(mgr, nullptr);
    if (!mgr) {
        return nullptr;
    }
    EXPECT_TRUE(mgr->Initialize());
    return mgr;
}

}  // namespace

class NullWindow_CTest : public ::testing::Test {};

TEST_F(NullWindow_CTest, NativeHandleFieldsAreDefaulted) {
    auto mgr = MakeInitializedNullManager();
    ASSERT_NE(mgr, nullptr);
    auto w = mgr->CreateWindow("test", 64, 48);
    ASSERT_NE(w, nullptr);
    const auto handle = w->GetNativeHandle();
    EXPECT_EQ(handle.api, WindowAPI_TP::NULL_WINDOW);
    EXPECT_EQ(handle.hwnd, nullptr);
    EXPECT_EQ(handle.ns_view, nullptr);
    EXPECT_EQ(handle.ns_window, nullptr);
    EXPECT_EQ(handle.ca_layer, nullptr);
    EXPECT_EQ(handle.ui_view, nullptr);
    EXPECT_EQ(handle.x11_display, nullptr);
    EXPECT_EQ(handle.xcb_connection, nullptr);
    EXPECT_EQ(handle.wl_display, nullptr);
    EXPECT_EQ(handle.wl_surface, nullptr);
    EXPECT_EQ(handle.a_native_window, nullptr);
    EXPECT_EQ(handle.canvas_id, nullptr);
    EXPECT_EQ(handle.x11_window_id, 0U);
    EXPECT_EQ(handle.xcb_window_id, 0U);
    w->Close();
    mgr->Shutdown();
}

TEST_F(NullWindow_CTest, CreatePollResizeClose) {
    auto mgr = MakeInitializedNullManager();
    ASSERT_NE(mgr, nullptr);
    auto w = mgr->CreateWindow("life", 128, 96);
    ASSERT_NE(w, nullptr);
    w->PollEvents();
    w->SwapBuffers();
    w->Resize(200, 100);
    EXPECT_EQ(w->GetWidth(), 200);
    EXPECT_EQ(w->GetHeight(), 100);
    w->Close();
    EXPECT_FALSE(w->IsOpen());
    mgr->Shutdown();
}

TEST_F(NullWindow_CTest, SetModeAndFullscreen) {
    auto mgr = MakeInitializedNullManager();
    ASSERT_NE(mgr, nullptr);
    auto w = mgr->CreateWindow("mode", 100, 80);
    ASSERT_NE(w, nullptr);
    EXPECT_EQ(w->GetWindowMode(), WindowMode_TP::WINDOWED);

    w->SetWindowMode(WindowMode_TP::FULLSCREEN);
    EXPECT_EQ(w->GetWindowMode(), WindowMode_TP::FULLSCREEN);
    w->SetFullscreen(true);
    EXPECT_TRUE(w->IsFullscreen());

    w->SetWindowMode(WindowMode_TP::BORDERLESS);
    EXPECT_EQ(w->GetWindowMode(), WindowMode_TP::BORDERLESS);

    w->SetWindowMode(WindowMode_TP::WINDOWED);
    w->SetFullscreen(false);
    EXPECT_EQ(w->GetWindowMode(), WindowMode_TP::WINDOWED);
    EXPECT_FALSE(w->IsFullscreen());

    w->Close();
    mgr->Shutdown();
}

TEST_F(NullWindow_CTest, MinimizeMaximizeRestoreNoop) {
    auto mgr = MakeInitializedNullManager();
    ASSERT_NE(mgr, nullptr);
    auto w = mgr->CreateWindow("mmr", 32, 32);
    ASSERT_NE(w, nullptr);
    w->Minimize();
    w->Maximize();
    w->Restore();
    w->Close();
    mgr->Shutdown();
}

TEST_F(NullWindow_CTest, DpiAndFramebufferDefaults) {
    auto mgr = MakeInitializedNullManager();
    ASSERT_NE(mgr, nullptr);
    auto w = mgr->CreateWindow("dpi", 100, 50);
    ASSERT_NE(w, nullptr);
    EXPECT_FLOAT_EQ(w->GetDPIScale(), 1.0F);
    EXPECT_EQ(w->GetWidth(), 100);
    EXPECT_EQ(w->GetHeight(), 50);
    EXPECT_EQ(w->GetFramebufferWidth(), 100U);
    EXPECT_EQ(w->GetFramebufferHeight(), 50U);

    w->Resize(200, 40);
    EXPECT_EQ(w->GetFramebufferWidth(), 200U);
    EXPECT_EQ(w->GetFramebufferHeight(), 40U);

    w->Close();
    mgr->Shutdown();
}

TEST_F(NullWindow_CTest, ClipboardRoundTripDefaultNoOp) {
    auto mgr = MakeInitializedNullManager();
    ASSERT_NE(mgr, nullptr);
    auto w = mgr->CreateWindow("clip", 16, 16);
    ASSERT_NE(w, nullptr);
    EXPECT_TRUE(w->GetClipboardText().empty());
    w->SetClipboardText("hello");
    EXPECT_TRUE(w->GetClipboardText().empty());
    w->Close();
    mgr->Shutdown();
}
