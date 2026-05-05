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
 * @brief Selects and constructs an IWindowManager for the requested or default WindowAPI_TP.
 */
class VNE_XWIN_API WindowFactory {
   public:
    static std::shared_ptr<IWindowManager> CreateWindowManager(WindowAPI_TP window_api);
    static std::shared_ptr<IWindowManager> CreateWindowManager(WindowAPI_TP window_api, const std::string& properties);
    static std::shared_ptr<IWindowManager> CreateWindowManager();

   private:
    static WindowAPI_TP GetBestWindowAPIForPlatform();
    static bool IsWindowAPISupported(WindowAPI_TP window_api);
    static std::string GetSupportedWindowAPIs();
    static std::string GetWindowAPIInfo(WindowAPI_TP window_api);
    static std::string GetWindowAPICapabilities(WindowAPI_TP window_api);

   public:
    static std::string GetVersion();
    static std::string GetBuildInfo();
    static bool IsAvailable();
    static std::string GetLastError();
    static void ClearLastError();

   private:
    static std::string last_error_;
};

}  // namespace vne::xwin
