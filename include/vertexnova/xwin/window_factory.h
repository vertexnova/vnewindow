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

#include "vertexnova/xwin/window_manager.h"
#include "vertexnova/xwin/xwin_export.h"
#include "vertexnova/xwin/xwin_types.h"

#include <memory>
#include <string>

namespace vne::xwin {

class VNE_XWIN_API WindowFactory_C {
   public:
    static std::shared_ptr<WindowManager_I> CreateWindowManager(WindowAPI_TP window_api);
    static std::shared_ptr<WindowManager_I> CreateWindowManager(WindowAPI_TP window_api, const std::string& properties);
    static std::shared_ptr<WindowManager_I> CreateWindowManager();

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
    static std::string _last_error;
};

}  // namespace vne::xwin
