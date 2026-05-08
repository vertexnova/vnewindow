#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   May 2026
 *
 * visionOS UIApplicationDelegate for vnewindow examples.
 * Bridges the ExampleRunner lifecycle to UIKit conventions and drives each
 * frame with a CADisplayLink set up in ExampleSceneDelegate.
 * ----------------------------------------------------------------------
 */

#if defined(VNE_PLATFORM_VISIONOS)
#import <UIKit/UIKit.h>
#else
// Lint/editor fallback for environments without Apple SDK headers.
@class UIWindow;
@protocol UIApplicationDelegate
@end
@interface UIResponder
@end
#endif

@interface ExampleAppDelegate : UIResponder <UIApplicationDelegate>

@property(strong, nonatomic) UIWindow* window;

@end
