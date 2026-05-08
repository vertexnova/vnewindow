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

#import "app_delegate.h"

#if defined(VNE_PLATFORM_VISIONOS)

@implementation ExampleAppDelegate

// Window creation and frame loop are handled entirely by ExampleSceneDelegate.
// The scene manifest in Info.plist.in routes the default session to it.

- (BOOL)application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launch_options {
    (void)application;
    (void)launch_options;
    return YES;
}

@end

#else

@implementation ExampleAppDelegate
@end

#endif
