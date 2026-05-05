#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   May 2026
 *
 * Autodoc:   yes
 * ----------------------------------------------------------------------
 */

/** @file monitor_info.h Per-monitor geometry and DPI metadata. */

#include "vertexnova/xwin/xwin_types.h"

#include <cstdint>
#include <string>

namespace vne::xwin {

struct MonitorInfo {
    std::string name;
    WindowBounds bounds;
    WindowBounds work_area;
    float dpi_scale = 1.0F;
    bool is_primary = false;
    uint32_t index = 0;
    uint32_t refresh_rate_hz = 0;
};

}  // namespace vne::xwin
