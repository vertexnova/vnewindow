#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   May 2026
 *
 * UIWindowSceneDelegate for vnewindow examples (iOS 13+).
 * Owns the ExampleRunner and CADisplayLink; creates the UIWindow inside
 * scene:willConnectToSession: as required by the UIScene lifecycle.
 * ----------------------------------------------------------------------
 */

#import <UIKit/UIKit.h>

@interface ExampleSceneDelegate : UIResponder <UIWindowSceneDelegate>

@property(strong, nonatomic) UIWindow* window;

@end
