/*
 * Linux: if WAYLAND_DISPLAY is set and the Wayland backend is compiled, it is preferred
 * over X11; otherwise DISPLAY selects X11. Headless CI uses NULL_WINDOW.
 */
/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   April 2026
 *
 * Autodoc:   yes
 * ----------------------------------------------------------------------
 */

#include "vertexnova/xwin/window_factory.h"

#include "vertexnova/xwin/xwin_version.h"

#include "platform/null/null_window_manager.h"

#if VNE_XWIN_HAS_WIN32
#include "platform/win32/win32_window_manager.h"
#endif
#if VNE_XWIN_HAS_COCOA
#include "platform/cocoa/cocoa_window_manager.h"
#endif
#if VNE_XWIN_HAS_X11
#include "platform/linux/x11/x11_window_manager.h"
#endif
#if VNE_XWIN_HAS_WAYLAND
#include "platform/linux/wayland/wayland_window_manager.h"
#endif
#if VNE_XWIN_HAS_WASM
#include "platform/web/wasm/wasm_window_manager.h"
#endif
#if VNE_XWIN_HAS_UIKIT
#include "platform/uikit/uikit_window_manager.h"
#endif
#if VNE_XWIN_HAS_ANDROID
#include "platform/android/android_window_manager.h"
#endif

#include "config.h"

#include <cstdlib>
#include <sstream>

namespace vne::xwin {

std::string WindowFactory_C::_last_error;

std::shared_ptr<WindowManager_I> WindowFactory_C::CreateWindowManager(WindowAPI_TP window_api) {
    ClearLastError();
    if (!IsWindowAPISupported(window_api)) {
        _last_error = "Requested window API is not supported in this build.";
        return nullptr;
    }
    switch (window_api) {
        case WindowAPI_TP::NULL_WINDOW:
            return std::make_shared<NullWindowManager_C>();
#if VNE_XWIN_HAS_WIN32
        case WindowAPI_TP::WIN32_WINDOW:
            return std::make_shared<Win32WindowManager_C>();
#endif
#if VNE_XWIN_HAS_COCOA
        case WindowAPI_TP::COCOA_WINDOW:
            return std::make_shared<CocoaWindowManager_C>();
#endif
#if VNE_XWIN_HAS_X11
        case WindowAPI_TP::X11_WINDOW:
            return std::make_shared<X11WindowManager_C>();
#endif
#if VNE_XWIN_HAS_WAYLAND
        case WindowAPI_TP::WAYLAND_WINDOW:
            return std::make_shared<WaylandWindowManager_C>();
#endif
#if VNE_XWIN_HAS_WASM
        case WindowAPI_TP::WASM_WINDOW:
            return std::make_shared<WasmWindowManager_C>();
#endif
#if VNE_XWIN_HAS_UIKIT
        case WindowAPI_TP::IOS_UIKIT_WINDOW:
            return std::make_shared<UIKitWindowManager_C>();
#endif
#if VNE_XWIN_HAS_ANDROID
        case WindowAPI_TP::ANDROID_SURFACE_WINDOW:
            return std::make_shared<AndroidWindowManager_C>();
#endif
        default:
            _last_error = "No factory mapping for this WindowAPI_TP value.";
            return nullptr;
    }
}

std::shared_ptr<WindowManager_I> WindowFactory_C::CreateWindowManager(WindowAPI_TP window_api,
                                                                      const std::string& properties) {
    auto mgr = CreateWindowManager(window_api);
    if (mgr) {
        mgr->SetProperties(properties);
    }
    return mgr;
}

std::shared_ptr<WindowManager_I> WindowFactory_C::CreateWindowManager() {
    const WindowAPI_TP api = GetBestWindowAPIForPlatform();
    auto mgr = CreateWindowManager(api);
    if (mgr) {
        return mgr;
    }
    return std::make_shared<NullWindowManager_C>();
}

WindowAPI_TP WindowFactory_C::GetBestWindowAPIForPlatform() {
#if VNE_XWIN_HAS_WASM
    return WindowAPI_TP::WASM_WINDOW;
#elif VNE_XWIN_HAS_WIN32
    return WindowAPI_TP::WIN32_WINDOW;
#elif VNE_XWIN_HAS_ANDROID
    return WindowAPI_TP::ANDROID_SURFACE_WINDOW;
#elif VNE_XWIN_HAS_UIKIT
    return WindowAPI_TP::IOS_UIKIT_WINDOW;
#elif VNE_XWIN_HAS_COCOA
    return WindowAPI_TP::COCOA_WINDOW;
#elif defined(__linux__)
    const auto env_nonempty = [](const char* v) { return v != nullptr && v[0] != '\0'; };
    const bool prefer_wl = env_nonempty(std::getenv("WAYLAND_DISPLAY"));
    const bool prefer_x = env_nonempty(std::getenv("DISPLAY"));
#if VNE_XWIN_HAS_WAYLAND
    if (prefer_wl) {
        return WindowAPI_TP::WAYLAND_WINDOW;
    }
#endif
#if VNE_XWIN_HAS_X11
    if (prefer_x) {
        return WindowAPI_TP::X11_WINDOW;
    }
#endif
#if VNE_XWIN_HAS_WAYLAND
    return WindowAPI_TP::WAYLAND_WINDOW;
#elif VNE_XWIN_HAS_X11
    return WindowAPI_TP::X11_WINDOW;
#else
    return WindowAPI_TP::NULL_WINDOW;
#endif
#else
    return WindowAPI_TP::NULL_WINDOW;
#endif
}

bool WindowFactory_C::IsWindowAPISupported(WindowAPI_TP window_api) {
    switch (window_api) {
        case WindowAPI_TP::NULL_WINDOW:
            return VNE_XWIN_HAS_NULL != 0;
#if VNE_XWIN_HAS_WIN32
        case WindowAPI_TP::WIN32_WINDOW:
            return true;
#endif
#if VNE_XWIN_HAS_COCOA
        case WindowAPI_TP::COCOA_WINDOW:
            return true;
#endif
#if VNE_XWIN_HAS_X11
        case WindowAPI_TP::X11_WINDOW:
            return true;
#endif
#if VNE_XWIN_HAS_WAYLAND
        case WindowAPI_TP::WAYLAND_WINDOW:
            return true;
#endif
#if VNE_XWIN_HAS_WASM
        case WindowAPI_TP::WASM_WINDOW:
            return true;
#endif
#if VNE_XWIN_HAS_UIKIT
        case WindowAPI_TP::IOS_UIKIT_WINDOW:
            return true;
#endif
#if VNE_XWIN_HAS_ANDROID
        case WindowAPI_TP::ANDROID_SURFACE_WINDOW:
            return true;
#endif
        default:
            return false;
    }
}

std::string WindowFactory_C::GetSupportedWindowAPIs() {
    std::ostringstream o;
    bool first = true;
    auto add = [&](const char* name) {
        if (!first) {
            o << ", ";
        }
        first = false;
        o << name;
    };
    if (VNE_XWIN_HAS_NULL) {
        add("NULL_WINDOW");
    }
#if VNE_XWIN_HAS_WIN32
    add("WIN32_WINDOW");
#endif
#if VNE_XWIN_HAS_COCOA
    add("COCOA_WINDOW");
#endif
#if VNE_XWIN_HAS_X11
    add("X11_WINDOW");
#endif
#if VNE_XWIN_HAS_WAYLAND
    add("WAYLAND_WINDOW");
#endif
#if VNE_XWIN_HAS_WASM
    add("WASM_WINDOW");
#endif
#if VNE_XWIN_HAS_UIKIT
    add("IOS_UIKIT_WINDOW");
#endif
#if VNE_XWIN_HAS_ANDROID
    add("ANDROID_SURFACE_WINDOW");
#endif
    return o.str();
}

std::string WindowFactory_C::GetWindowAPIInfo(WindowAPI_TP window_api) {
    switch (window_api) {
        case WindowAPI_TP::NULL_WINDOW:
            return "Null backend for headless tests and CI.";
        case WindowAPI_TP::WIN32_WINDOW:
            return "Microsoft Win32 HWND window.";
        case WindowAPI_TP::COCOA_WINDOW:
            return "macOS AppKit NSWindow / NSView.";
        case WindowAPI_TP::X11_WINDOW:
            return "Linux X11 (Xlib) window.";
        case WindowAPI_TP::WAYLAND_WINDOW:
            return "Linux Wayland wl_surface + xdg-shell toplevel.";
        case WindowAPI_TP::IOS_UIKIT_WINDOW:
            return "iOS UIKit UIView surface host.";
        case WindowAPI_TP::ANDROID_SURFACE_WINDOW:
            return "Android ANativeWindow via descriptor.platform_data.";
        case WindowAPI_TP::WASM_WINDOW:
            return "Emscripten HTML5 canvas host.";
        default:
            return "Unknown or unimplemented window API.";
    }
}

std::string WindowFactory_C::GetWindowAPICapabilities(WindowAPI_TP window_api) {
    return GetWindowAPIInfo(window_api);
}

std::string WindowFactory_C::GetVersion() {
    return get_version() ? std::string(get_version()) : std::string();
}

std::string WindowFactory_C::GetBuildInfo() {
    std::ostringstream o;
    o << PROJECT_NAME << " " << PROJECT_VERSION << " | APIs: " << GetSupportedWindowAPIs();
    return o.str();
}

bool WindowFactory_C::IsAvailable() {
    return true;
}

std::string WindowFactory_C::GetLastError() {
    return _last_error;
}

void WindowFactory_C::ClearLastError() {
    _last_error.clear();
}

}  // namespace vne::xwin
