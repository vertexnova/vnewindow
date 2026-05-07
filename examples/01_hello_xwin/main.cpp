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

    VNE_LOG_INFO << "vne::xwin " << WindowFactory::getBuildInfo();
    VNE_LOG_INFO << "Version: " << vne::xwin::libraryVersion();

    auto mgr = WindowFactory::createWindowManager(vne::xwin::WindowAPI::eNullWindow);
    if (!mgr) {
        return 1;
    }
    if (!mgr->initialize()) {
        mgr->shutdown();
        return 1;
    }
    auto w = mgr->openWindow("hello_xwin", 320, 240);
    if (!w) {
        mgr->shutdown();
        return 1;
    }
    VNE_LOG_INFO << "Null window size: " << w->getWidth() << "x" << w->getHeight();
    mgr->shutdown();
    return 0;
}
