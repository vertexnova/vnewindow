#pragma once
/*
 * @brief `VNE_XWIN_API` — same contract as vneio `VNEIO_API` (shared vs static).
 * On Windows: @c VNE_XWIN_BUILDING_DLL export / @c VNE_XWIN_DLL import.
 * On Unix shared: @c VNE_XWIN_BUILDING_DLL sets default visibility; consumers need not define @c VNE_XWIN_DLL.
 */
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
#if defined(VNE_XWIN_BUILDING_DLL) && (defined(__GNUC__) || defined(__clang__))
#define VNE_XWIN_API __attribute__((visibility("default")))
#else
#define VNE_XWIN_API
#endif
#endif
