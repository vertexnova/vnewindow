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

#include <gtest/gtest.h>

#include "vertexnova/xwin/window_factory.h"
#include "vertexnova/xwin/xwin_version.h"

TEST(VneXWin, GetVersion) {
    const char* ver = vne::xwin::get_version();
    ASSERT_NE(ver, nullptr);
    EXPECT_STRNE(ver, "");
}

TEST(VneXWin, FactoryNullBackend) {
    auto mgr = vne::xwin::WindowFactory_C::CreateWindowManager(vne::xwin::WindowAPI_TP::NULL_WINDOW);
    ASSERT_NE(mgr, nullptr);
    EXPECT_TRUE(mgr->Initialize());
    auto w = mgr->CreateWindow("test", 64, 48);
    ASSERT_NE(w, nullptr);
    EXPECT_TRUE(w->IsOpen());
    EXPECT_EQ(w->GetWindowAPI(), vne::xwin::WindowAPI_TP::NULL_WINDOW);
    mgr->Shutdown();
}

TEST(VneXWin, FactoryGetBuildInfo) {
    const std::string info = vne::xwin::WindowFactory_C::GetBuildInfo();
    EXPECT_FALSE(info.empty());
}
