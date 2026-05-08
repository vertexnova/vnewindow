#pragma once
/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   May 2026
 *
 * iOS UIApplicationDelegate for vnewindow examples.
 * Bridges the ExampleRunner lifecycle to UIKit conventions and drives each
 * frame with a CADisplayLink.
 * ----------------------------------------------------------------------
 */

#import <UIKit/UIKit.h>

@interface ExampleAppDelegate : UIResponder <UIApplicationDelegate>

@property(strong, nonatomic) UIWindow* window;

@end
