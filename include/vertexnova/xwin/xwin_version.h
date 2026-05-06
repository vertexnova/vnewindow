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

/** @file xwin_version.h Library version string accessor. */

#include "vertexnova/xwin/xwin_export.h"

namespace vne::xwin {

/** @brief Library version string (from CMake PROJECT_VERSION). */
/** @brief Nul-terminated PROJECT_VERSION string (CMake); prefer WindowFactory::getVersion() for std::string. */
VNE_XWIN_API const char* libraryVersion() noexcept;

}  // namespace vne::xwin
