/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   April 2026
 *
 * Autodoc:   yes
 * Tests for src/vertexnova/xwin/platform/null/null_window_manager.cpp (via NULL_WINDOW factory).
 * ----------------------------------------------------------------------
 */

#include <memory>

#include <gtest/gtest.h>

#include "vertexnova/xwin/event_bridge_callbacks.h"
#include "vertexnova/xwin/window_factory.h"
#include "vertexnova/xwin/xwin_types.h"

using vne::xwin::EventBridgeCallbacks;
using vne::xwin::WindowAPI_TP;
using vne::xwin::WindowFactory;

namespace {

std::shared_ptr<vne::xwin::IWindowManager> MakeInitializedNullManager() {
    auto mgr = WindowFactory::CreateWindowManager(WindowAPI_TP::NULL_WINDOW);
    EXPECT_NE(mgr, nullptr);
    if (!mgr) {
        return nullptr;
    }
    EXPECT_TRUE(mgr->Initialize());
    return mgr;
}

}  // namespace

class NullWindowManager_CTest : public ::testing::Test {};

TEST_F(NullWindowManager_CTest, MonitorQueriesUseBaseDefaults) {
    auto mgr = MakeInitializedNullManager();
    ASSERT_NE(mgr, nullptr);
    EXPECT_EQ(mgr->GetMonitorCount(), 0U);
    EXPECT_EQ(mgr->GetPrimaryMonitorIndex(), 0U);
    const auto info = mgr->GetMonitorInfo(0);
    EXPECT_TRUE(info.name.empty());
    EXPECT_FLOAT_EQ(info.dpi_scale, 1.0F);
    mgr->Shutdown();
}

TEST_F(NullWindowManager_CTest, EventBridgeCallbacksDoNotCrashOnProcessEvents) {
    auto mgr = MakeInitializedNullManager();
    ASSERT_NE(mgr, nullptr);

    bool called = false;
    EventBridgeCallbacks cb{};
    cb.onWindowFocus = [&called](vne::xwin::IWindow*, bool) { called = true; };

    mgr->setEventBridgeCallbacks(std::move(cb));
    mgr->ProcessEvents();

    auto w = mgr->CreateWindow("eb", 16, 16);
    ASSERT_NE(w, nullptr);
    mgr->ProcessEvents();
    EXPECT_FALSE(called);

    w->Close();
    mgr->Shutdown();
}
