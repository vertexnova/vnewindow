#pragma once
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

/** @file window_factory.h Entry points to construct IWindowManager for a build and platform. */

#include "vertexnova/xwin/window_manager.h"
#include "vertexnova/xwin/xwin_export.h"
#include "vertexnova/xwin/xwin_types.h"

#include <memory>
#include <string>

namespace vne::xwin {

/**
 * @brief Selects and constructs an IWindowManager for the requested or default WindowAPI.
 */
class VNE_XWIN_API WindowFactory {
   public:
    [[nodiscard]] static std::shared_ptr<IWindowManager> createWindowManager(WindowAPI window_api);
    [[nodiscard]] static std::shared_ptr<IWindowManager> createWindowManager(WindowAPI window_api,
                                                                             const std::string& properties);
    [[nodiscard]] static std::shared_ptr<IWindowManager> createWindowManager();

   private:
    [[nodiscard]] static WindowAPI getBestWindowAPIForPlatform();
    [[nodiscard]] static bool isWindowAPISupported(WindowAPI window_api);
    [[nodiscard]] static std::string getSupportedWindowAPIs();
    [[nodiscard]] static std::string getWindowAPIInfo(WindowAPI window_api);
    [[nodiscard]] static std::string getWindowAPICapabilities(WindowAPI window_api);

   public:
    [[nodiscard]] static std::string getVersion();
    [[nodiscard]] static std::string getBuildInfo();
    [[nodiscard]] static bool isAvailable();
    [[nodiscard]] static std::string getLastError();
    static void clearLastError();

   private:
    static std::string last_error;
};

}  // namespace vne::xwin
