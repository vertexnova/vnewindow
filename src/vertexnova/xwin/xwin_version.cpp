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

#include "vertexnova/xwin/xwin_version.h"

#include "config.h"

namespace vne::xwin {

const char* libraryVersion() noexcept {
    return PROJECT_VERSION;
}

}  // namespace vne::xwin
