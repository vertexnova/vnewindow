/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   April 2026
 *
 * Autodoc:   yes
 * Tests for src/vertexnova/xwin/platform/null/null_window_manager.cpp (via eNullWindow factory).
 * ----------------------------------------------------------------------
 */

#include "vertexnova/xwin/window_factory.h"
#include "vertexnova/xwin/xwin_types.h"

#include <vertexnova/events/events.h>

#include <memory>
#include <set>

#include <gtest/gtest.h>

using vne::xwin::WindowAPI;
using vne::xwin::WindowFactory;

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

class NullWindowManagerTest : public ::testing::Test {};

TEST_F(NullWindowManagerTest, MonitorQueriesUseBaseDefaults) {
    auto mgr = MakeInitializedNullManager();
    ASSERT_NE(mgr, nullptr);
    EXPECT_EQ(mgr->getMonitorCount(), 0U);
    EXPECT_EQ(mgr->getPrimaryMonitorIndex(), 0U);
    const auto info = mgr->getMonitorInfo(0);
    EXPECT_TRUE(info.name.empty());
    EXPECT_FLOAT_EQ(info.dpi_scale, 1.0F);
    mgr->shutdown();
}

TEST_F(NullWindowManagerTest, ProcessEventsOnNullBackendEmitsNothing) {
    auto mgr = MakeInitializedNullManager();
    ASSERT_NE(mgr, nullptr);

    auto& events = vne::events::EventManager::instance();
    events.clearPendingEvents();

    auto w = mgr->openWindow("eb", 16, 16);
    ASSERT_NE(w, nullptr);
    mgr->processEvents();

    // The null backend has no native event source; it must stay silent rather than synthesizing.
    EXPECT_EQ(events.pendingEventCount(), 0U);

    w->close();
    mgr->shutdown();
}

TEST_F(NullWindowManagerTest, WindowIdsAreNonZeroAndUnique) {
    auto mgr = MakeInitializedNullManager();
    ASSERT_NE(mgr, nullptr);

    auto a = mgr->openWindow("a", 16, 16);
    auto b = mgr->openWindow("b", 16, 16);
    ASSERT_NE(a, nullptr);
    ASSERT_NE(b, nullptr);

    // Zero is kInvalidWindowId, which must never be handed out as a real id.
    EXPECT_NE(a->getId(), vne::events::kInvalidWindowId);
    EXPECT_NE(b->getId(), vne::events::kInvalidWindowId);
    EXPECT_NE(a->getId(), b->getId());

    mgr->shutdown();
}

TEST_F(NullWindowManagerTest, FindWindowResolvesById) {
    auto mgr = MakeInitializedNullManager();
    ASSERT_NE(mgr, nullptr);

    auto w = mgr->openWindow("findme", 16, 16);
    ASSERT_NE(w, nullptr);

    // This is how a listener gets from an event back to the window that produced it.
    EXPECT_EQ(mgr->findWindow(w->getId()), w);
    EXPECT_EQ(mgr->findWindow(vne::events::kInvalidWindowId), nullptr);
    EXPECT_EQ(mgr->findWindow(w->getId() + 100000U), nullptr);

    mgr->shutdown();
}

TEST_F(NullWindowManagerTest, ApplicationLifecycleIsEmittedWithoutAnyWindow) {
    auto mgr = MakeInitializedNullManager();
    ASSERT_NE(mgr, nullptr);

    auto& events = vne::events::EventManager::instance();
    events.clearPendingEvents();

    // Lifecycle is process-scoped: it must fire even on a backend that owns no window.
    mgr->notifyApplicationLifecycle(vne::xwin::ApplicationLifecycle::ePause);
    mgr->notifyApplicationLifecycle(vne::xwin::ApplicationLifecycle::eResume);
    mgr->notifyApplicationLifecycle(vne::xwin::ApplicationLifecycle::eLowMemory);

    EXPECT_EQ(events.pendingEventCount(), 3U);
    events.clearPendingEvents();
    mgr->shutdown();
}
