/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   May 2026
 *
 * visionOS scene delegate for vnewindow examples.
 * ----------------------------------------------------------------------
 */

#import "scene_delegate.h"

#if defined(VNE_PLATFORM_VISIONOS)

#import <QuartzCore/QuartzCore.h>

#include "common/example_runner.h"
#include "vertexnova/xwin/native_window_handle.h"
#include "vertexnova/logging/logging.h"

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
                 options:(UISceneConnectionOptions*)connection_options {
    (void)session;
    (void)connection_options;
    _isShutdown = NO;

    _runner = std::make_unique<vne::xwin::examples::ExampleRunner>(createExample());
    if ([scene isKindOfClass:[UIWindowScene class]]) {
        _runner->setPlatformData((__bridge void*)static_cast<UIWindowScene*>(scene));
    }

    if (!_runner->initialize()) {
        VNE_LOG_ERROR << "[visionOS SceneDelegate] ExampleRunner::initialize() failed.";
        return;
    }

    // Adopt the UIWindow created by UIKitWindow (already bound to a UIWindowScene).
    vne::xwin::IWindow* xwin_window = _runner->window();
    if (xwin_window) {
        vne::xwin::NativeWindowHandle handle = xwin_window->getNativeHandle();
        UIWindow* window = (__bridge UIWindow*)handle.ui_window;
        if (window) {
            self.window = window;
            [self.window makeKeyAndVisible];
        }
    }

    // CADisplayLink drives the per-frame tick (replaces the desktop while loop)
    _displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(_onFrame)];
    [_displayLink addToRunLoop:NSRunLoop.mainRunLoop forMode:NSRunLoopCommonModes];
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
    (void)scene;
    [self _doShutdown];
}

- (void)sceneWillResignActive:(UIScene*)scene {
    (void)scene;
    _displayLink.paused = YES;
}

- (void)sceneDidBecomeActive:(UIScene*)scene {
    (void)scene;
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

#else

// SDK-less lint/editor fallback: provide an empty implementation.
@implementation ExampleSceneDelegate
@end

#endif
