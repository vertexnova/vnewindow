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

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#if defined(VNE_XWIN_BUILDING_DLL)
#define VNE_XWIN_API __declspec(dllexport)
#elif defined(VNE_XWIN_DLL)
#define VNE_XWIN_API __declspec(dllimport)
#else
#define VNE_XWIN_API
#endif
#else
#define VNE_XWIN_API
#endif
