/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * visionOS entry point for vnewindow examples.
 *
 * This file is injected automatically by vne_add_example() when building for
 * visionOS. UIApplicationMain drives the event loop; each frame is ticked via a
 * CADisplayLink set up in ExampleSceneDelegate.
 * ----------------------------------------------------------------------
 */

#if defined(VNE_PLATFORM_VISIONOS)
#import <UIKit/UIKit.h>
#import "app_delegate.h"

int main(int argc, char* argv[]) {
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([ExampleAppDelegate class]));
    }
}
#else
// SDK-less lint/editor fallback: make the translation unit compile even
// when UIKit/visionOS headers aren't available in the current environment.
int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    return 0;
}
#endif
