/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * iOS entry point for vnewindow examples.
 *
 * This file is injected automatically by vne_add_example() when building for
 * iOS. UIApplicationMain drives the event loop; each frame is ticked via a
 * CADisplayLink set up in VneExampleAppDelegate.
 * ----------------------------------------------------------------------
 */

#import <UIKit/UIKit.h>

#import "app_delegate.h"

int main(int argc, char* argv[]) {
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([VneExampleAppDelegate class]));
    }
}
