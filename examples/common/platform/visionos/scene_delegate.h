#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   May 2026
 *
 * UIWindowSceneDelegate for vnewindow examples (visionOS).
 * Owns the ExampleRunner and CADisplayLink; creates the UIWindow inside
 * scene:willConnectToSession: as required by the UIScene lifecycle.
 * ----------------------------------------------------------------------
 */

#if defined(VNE_PLATFORM_VISIONOS)
#import <UIKit/UIKit.h>
#else
// Lint/editor fallback for environments without Apple SDK headers.
@class UIWindow;
@protocol UIWindowSceneDelegate
@end
@interface UIResponder
@end
#endif

@interface ExampleSceneDelegate : UIResponder <UIWindowSceneDelegate>

@property(strong, nonatomic) UIWindow* window;

@end
