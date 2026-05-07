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

#import <Foundation/Foundation.h>
#include <dispatch/dispatch.h>

namespace vne::xwin {

/** @brief Run work on the app main thread; if already there, runs inline (avoids dispatch_sync deadlock). */
inline void uikitRunOnMainSync(void (^work)(void)) {
    if ([NSThread isMainThread]) {
        work();
    } else {
        dispatch_sync(dispatch_get_main_queue(), work);
    }
}

}  // namespace vne::xwin
