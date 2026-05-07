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

#import "scene_delegate.h"

#import <QuartzCore/QuartzCore.h>

#include "common/example_runner.h"
#include "vertexnova/xwin/native_window_handle.h"

#include <vertexnova/logging/logging.h>
#include <memory>

// Factory declared in each example's example.cpp
std::unique_ptr<vne::xwin::examples::ExampleBase> createExample();

@implementation ExampleSceneDelegate {
    std::unique_ptr<vne::xwin::examples::ExampleRunner> _runner;
    CADisplayLink* _displayLink;
    BOOL _isShutdown;
}

- (void)scene:(UIScene*)scene
    willConnectToSession:(UISceneSession*)session
                 options:(UISceneConnectionOptions*)connectionOptions {

    _isShutdown = NO;
    UIWindowScene* windowScene = (UIWindowScene*)scene;

    _runner = std::make_unique<vne::xwin::examples::ExampleRunner>(createExample());

    if (!_runner->initialize()) {
        VNE_LOG_ERROR << "[iOS SceneDelegate] ExampleRunner::initialize() failed.";
        return;
    }

    // ------------------------------------------------------------------
    // Bridge: embed the UIView created by UIKitWindow into the scene's
    // UIWindow. Window is created with the UIWindowScene to satisfy the
    // modern scene lifecycle (suppresses the "UIScene lifecycle" assert).
    // ------------------------------------------------------------------
    vne::xwin::IWindow* xwin = _runner->window();
    if (xwin) {
        vne::xwin::NativeWindowHandle handle = xwin->getNativeHandle();
        UIView* xwinView = (__bridge UIView*)handle.ui_view;

        self.window = [[UIWindow alloc] initWithWindowScene:windowScene];

        UIViewController* vc = [[UIViewController alloc] init];
        vc.view.backgroundColor = UIColor.blackColor;

        if (xwinView) {
            xwinView.frame = vc.view.bounds;
            xwinView.autoresizingMask =
                UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
            [vc.view addSubview:xwinView];
        }

        self.window.rootViewController = vc;
        [self.window makeKeyAndVisible];
    }

    // CADisplayLink drives the per-frame tick (replaces the desktop while loop)
    _displayLink = [CADisplayLink displayLinkWithTarget:self
                                               selector:@selector(_onFrame)];
    [_displayLink addToRunLoop:NSRunLoop.mainRunLoop
                       forMode:NSRunLoopCommonModes];
}

- (void)_onFrame {
    if (_isShutdown || !_runner) {
        return;
    }
    @autoreleasepool {
        if (!_runner->tick()) {
            [self _doShutdown];
        }
    }
}

- (void)sceneDidDisconnect:(UIScene*)scene {
    [self _doShutdown];
}

- (void)sceneWillResignActive:(UIScene*)scene {
    _displayLink.paused = YES;
}

- (void)sceneDidBecomeActive:(UIScene*)scene {
    _displayLink.paused = NO;
}

- (void)_doShutdown {
    if (_isShutdown) {
        return;
    }
    _isShutdown = YES;
    [_displayLink invalidate];
    _displayLink = nil;
    if (_runner) {
        _runner->shutdown();
        _runner.reset();
    }
}

@end
