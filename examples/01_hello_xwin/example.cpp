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

/**
 * 01_hello_xwin — Minimal vne::xwin example.
 *
 * Opens a native window for the current platform (Cocoa on macOS, UIKit on
 * iOS, X11/Wayland on Linux, Win32 on Windows) and logs build / version info.
 * On CI (null backend) the runner performs a clean smoke cycle and exits.
 */

#include "common/example_base.h"

#include "vertexnova/xwin/window_factory.h"
#include "vertexnova/xwin/xwin_version.h"

#include <vertexnova/logging/logging.h>

class HelloXwinExample final : public vne::xwin::examples::ExampleBase {
   public:
    vne::xwin::examples::ExampleConfig configure() override {
        return {"Hello VneXWin", 800, 600};
    }

    void onInit(vne::xwin::IWindow& window,
                vne::xwin::IWindowManager& /*mgr*/) override {
        using vne::xwin::WindowFactory;
        VNE_LOG_INFO << "vne::xwin build: " << WindowFactory::getBuildInfo();
        VNE_LOG_INFO << "Version        : " << vne::xwin::libraryVersion();
        VNE_LOG_INFO << "Window size    : " << window.getWidth()
                     << "x" << window.getHeight();
        VNE_LOG_INFO << "DPI scale      : " << window.getDpiScale();
        VNE_LOG_INFO << "Press ESC or close the window to exit.";
    }

    bool onFrame(float /*dt*/) override {
        return true; // keep running until ESC / close
    }
};

std::unique_ptr<vne::xwin::examples::ExampleBase> createExample() {
    return std::make_unique<HelloXwinExample>();
}
