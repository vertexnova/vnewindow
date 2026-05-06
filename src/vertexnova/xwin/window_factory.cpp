/*
 * Linux: if WAYLAND_DISPLAY is set and the Wayland backend is compiled, it is preferred
 * over X11; otherwise DISPLAY selects X11. Headless CI uses WindowAPI::eNullWindow.
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
#include "platform/wasm/wasm_window_manager.h"
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

std::string WindowFactory::last_error_;

std::shared_ptr<IWindowManager> WindowFactory::CreateWindowManager(WindowAPI window_api) {
    ClearLastError();
    if (!IsWindowAPISupported(window_api)) {
        last_error_ = "Requested window API is not supported in this build.";
        return nullptr;
    }
    switch (window_api) {
        case WindowAPI::eNullWindow:
            return std::make_shared<NullWindowManager>();
#if VNE_XWIN_HAS_WIN32
        case WindowAPI::eWin32Window:
            return std::make_shared<Win32WindowManager_C>();
#endif
#if VNE_XWIN_HAS_COCOA
        case WindowAPI::eCocoaWindow:
            return std::make_shared<CocoaWindowManager>();
#endif
#if VNE_XWIN_HAS_X11
        case WindowAPI::eX11Window:
            return std::make_shared<X11WindowManager>();
#endif
#if VNE_XWIN_HAS_WAYLAND
        case WindowAPI::eWaylandWindow:
            return std::make_shared<WaylandWindowManager>();
#endif
#if VNE_XWIN_HAS_WASM
        case WindowAPI::eWasmWindow:
            return std::make_shared<WasmWindowManager>();
#endif
#if VNE_XWIN_HAS_UIKIT
        case WindowAPI::eIosUikitWindow:
            return std::make_shared<UIKitWindowManager>();
#endif
#if VNE_XWIN_HAS_ANDROID
        case WindowAPI::eAndroidSurfaceWindow:
            return std::make_shared<AndroidWindowManager>();
#endif
        default:
            last_error_ = "No factory mapping for this WindowAPI value.";
            return nullptr;
    }
}

std::shared_ptr<IWindowManager> WindowFactory::CreateWindowManager(WindowAPI window_api,
                                                                   const std::string& properties) {
    auto mgr = CreateWindowManager(window_api);
    if (mgr) {
        mgr->SetProperties(properties);
    }
    return mgr;
}

std::shared_ptr<IWindowManager> WindowFactory::CreateWindowManager() {
    const WindowAPI api = GetBestWindowAPIForPlatform();
    auto mgr = CreateWindowManager(api);
    if (mgr) {
        return mgr;
    }
    return std::make_shared<NullWindowManager>();
}

WindowAPI WindowFactory::GetBestWindowAPIForPlatform() {
#if VNE_XWIN_HAS_WASM
    return WindowAPI::eWasmWindow;
#elif VNE_XWIN_HAS_WIN32
    return WindowAPI::eWin32Window;
#elif VNE_XWIN_HAS_ANDROID
    return WindowAPI::eAndroidSurfaceWindow;
#elif VNE_XWIN_HAS_UIKIT
    return WindowAPI::eIosUikitWindow;
#elif VNE_XWIN_HAS_COCOA
    return WindowAPI::eCocoaWindow;
#elif defined(__linux__)
    const auto env_nonempty = [](const char* v) { return v != nullptr && v[0] != '\0'; };
#if VNE_XWIN_HAS_WAYLAND
    const bool prefer_wl = env_nonempty(std::getenv("WAYLAND_DISPLAY"));
#endif
    const bool prefer_x = env_nonempty(std::getenv("DISPLAY"));
#if VNE_XWIN_HAS_WAYLAND
    if (prefer_wl) {
        return WindowAPI::eWaylandWindow;
    }
#endif
#if VNE_XWIN_HAS_X11
    if (prefer_x) {
        return WindowAPI::eX11Window;
    }
#endif
#if VNE_XWIN_HAS_WAYLAND
    return WindowAPI::eWaylandWindow;
#elif VNE_XWIN_HAS_X11
    return WindowAPI::eX11Window;
#else
    return WindowAPI::eNullWindow;
#endif
#else
    return WindowAPI::eNullWindow;
#endif
}

bool WindowFactory::IsWindowAPISupported(WindowAPI window_api) {
    switch (window_api) {
        case WindowAPI::eNullWindow:
            return VNE_XWIN_HAS_NULL != 0;
#if VNE_XWIN_HAS_WIN32
        case WindowAPI::eWin32Window:
            return true;
#endif
#if VNE_XWIN_HAS_COCOA
        case WindowAPI::eCocoaWindow:
            return true;
#endif
#if VNE_XWIN_HAS_X11
        case WindowAPI::eX11Window:
            return true;
#endif
#if VNE_XWIN_HAS_WAYLAND
        case WindowAPI::eWaylandWindow:
            return true;
#endif
#if VNE_XWIN_HAS_WASM
        case WindowAPI::eWasmWindow:
            return true;
#endif
#if VNE_XWIN_HAS_UIKIT
        case WindowAPI::eIosUikitWindow:
            return true;
#endif
#if VNE_XWIN_HAS_ANDROID
        case WindowAPI::eAndroidSurfaceWindow:
            return true;
#endif
        default:
            return false;
    }
}

std::string WindowFactory::GetSupportedWindowAPIs() {
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
        add("eNullWindow");
    }
#if VNE_XWIN_HAS_WIN32
    add("eWin32Window");
#endif
#if VNE_XWIN_HAS_COCOA
    add("eCocoaWindow");
#endif
#if VNE_XWIN_HAS_X11
    add("eX11Window");
#endif
#if VNE_XWIN_HAS_WAYLAND
    add("eWaylandWindow");
#endif
#if VNE_XWIN_HAS_WASM
    add("eWasmWindow");
#endif
#if VNE_XWIN_HAS_UIKIT
    add("eIosUikitWindow");
#endif
#if VNE_XWIN_HAS_ANDROID
    add("eAndroidSurfaceWindow");
#endif
    return o.str();
}

std::string WindowFactory::GetWindowAPIInfo(WindowAPI window_api) {
    switch (window_api) {
        case WindowAPI::eNullWindow:
            return "Null backend for headless tests and CI.";
        case WindowAPI::eWin32Window:
            return "Microsoft Win32 HWND window.";
        case WindowAPI::eCocoaWindow:
            return "macOS AppKit NSWindow / NSView.";
        case WindowAPI::eX11Window:
            return "Linux X11 (Xlib) window.";
        case WindowAPI::eWaylandWindow:
            return "Linux Wayland wl_surface + xdg-shell toplevel.";
        case WindowAPI::eIosUikitWindow:
            return "iOS UIKit UIView surface host.";
        case WindowAPI::eAndroidSurfaceWindow:
            return "Android ANativeWindow via descriptor.platform_data.";
        case WindowAPI::eWasmWindow:
            return "Emscripten HTML5 canvas host.";
        default:
            return "Unknown or unimplemented window API.";
    }
}

std::string WindowFactory::GetWindowAPICapabilities(WindowAPI window_api) {
    return GetWindowAPIInfo(window_api);
}

std::string WindowFactory::GetVersion() {
    return get_version() ? std::string(get_version()) : std::string();
}

std::string WindowFactory::GetBuildInfo() {
    std::ostringstream o;
    o << PROJECT_NAME << " " << PROJECT_VERSION << " | APIs: " << GetSupportedWindowAPIs();
    return o.str();
}

bool WindowFactory::IsAvailable() {
    return true;
}

std::string WindowFactory::GetLastError() {
    return last_error_;
}

void WindowFactory::ClearLastError() {
    last_error_.clear();
}

}  // namespace vne::xwin
