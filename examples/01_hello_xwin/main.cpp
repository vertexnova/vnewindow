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
#include "vertexnova/xwin/window_factory.h"
#include "vertexnova/xwin/xwin_version.h"

int main() {
    vne::xwin::examples::LoggingGuard logging_guard;

    using vne::xwin::WindowFactory;

    VNE_LOG_INFO << "vne::xwin " << WindowFactory::GetBuildInfo();
    VNE_LOG_INFO << "Version: " << vne::xwin::get_version();

    auto mgr = WindowFactory::CreateWindowManager(vne::xwin::WindowAPI_TP::NULL_WINDOW);
    if (mgr && mgr->Initialize()) {
        auto w = mgr->CreateWindow("hello_xwin", 320, 240);
        if (w) {
            VNE_LOG_INFO << "Null window size: " << w->GetWidth() << "x" << w->GetHeight();
        }
        mgr->Shutdown();
    }

    return 0;
}
