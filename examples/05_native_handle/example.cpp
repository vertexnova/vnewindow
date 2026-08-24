/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   August 2026
 *
 * Autodoc:   yes
 * ----------------------------------------------------------------------
 */

/**
 * 05_native_handle - Inspect what vne::xwin hands to a renderer.
 *
 * A graphics backend consumes NativeWindowHandle plus the DPI/framebuffer geometry, and nothing
 * else. This example prints exactly that, so the handoff can be verified on a device without
 * building a renderer: if the pointer a platform needs is null here, RHI surface creation was
 * always going to fail, and this says so in one line rather than in a swapchain error.
 *
 * It also reports the manager's capability answers, since those decide what an app may attempt.
 */

#include "common/example_base.h"

#include "vertexnova/xwin/native_window_handle.h"
#include "vertexnova/xwin/window.h"
#include "vertexnova/xwin/window_manager.h"
#include "vertexnova/xwin/window_factory.h"
#include "vertexnova/xwin/xwin_types.h"

#include <vertexnova/events/events.h>
#include <vertexnova/logging/logging.h>

#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

namespace {

const char* toString(vne::xwin::WindowAPI api) {
    using vne::xwin::WindowAPI;
    switch (api) {
        case WindowAPI::eNullWindow:
            return "eNullWindow";
        case WindowAPI::eWin32Window:
            return "eWin32Window";
        case WindowAPI::eCocoaWindow:
            return "eCocoaWindow";
        case WindowAPI::eX11Window:
            return "eX11Window";
        case WindowAPI::eWaylandWindow:
            return "eWaylandWindow";
        case WindowAPI::eIosUikitWindow:
            return "eIosUikitWindow";
        case WindowAPI::eAndroidSurfaceWindow:
            return "eAndroidSurfaceWindow";
        case WindowAPI::eWasmWindow:
            return "eWasmWindow";
    }
    return "Unknown";
}

std::string ptr(const void* p) {
    if (p == nullptr) {
        return "(null)";
    }
    std::ostringstream ss;
    ss << p;
    return ss.str();
}

/** Logs a field only when populated, so each platform's output is just its own handle. */
void field(const char* name, const void* value) {
    if (value != nullptr) {
        VNE_LOG_INFO << "  " << name << " = " << ptr(value);
    }
}

void field(const char* name, uint32_t value) {
    if (value != 0U) {
        VNE_LOG_INFO << "  " << name << " = " << value;
    }
}

}  // namespace

class NativeHandleExample final : public vne::xwin::examples::ExampleBase {
   public:
    vne::xwin::examples::ExampleConfig configure() override { return {"05 Native Handle Inspector", 800, 600}; }

    void onInit(vne::xwin::IWindow& window, vne::xwin::IWindowManager& mgr) override {
        window_ = &window;
        mgr_ = &mgr;

        dumpPlatform();
        dumpHandle();
        dumpGeometry();
        dumpCapabilities();
        VNE_LOG_INFO << "Press [H] to re-dump (values change on resize, move or display switch), [ESC] to quit.";
    }

    bool onFrame(float /*dt*/) override { return true; }

    void onEvent(const vne::events::Event& event) override {
        using vne::events::EventType;

        if (event.type() == EventType::eKeyPressed) {
            const auto& key = static_cast<const vne::events::KeyPressedEvent&>(event);
            if (key.keyCode() == vne::events::KeyCode::eH) {
                dumpHandle();
                dumpGeometry();
            }
            return;
        }

        // Geometry the renderer depends on just changed; re-report it rather than make the
        // reader guess what the new drawable size is.
        if (event.type() == EventType::eWindowResize || event.type() == EventType::eWindowDpiChanged) {
            dumpGeometry();
        }
    }

   private:
    void dumpPlatform() const {
        VNE_LOG_INFO << "--- platform ---";
        VNE_LOG_INFO << "  api          = " << toString(window_->getWindowAPI());
        VNE_LOG_INFO << "  platform     = " << mgr_->getPlatformInfo();
        VNE_LOG_INFO << "  build        = " << vne::xwin::WindowFactory::getBuildInfo();
    }

    void dumpHandle() const {
        const vne::xwin::NativeWindowHandle h = window_->getNativeHandle();
        VNE_LOG_INFO << "--- native handle (populated fields only) ---";
        VNE_LOG_INFO << "  api          = " << toString(h.api);
        field("hwnd", h.hwnd);
        field("ns_view", h.ns_view);
        field("ns_window", h.ns_window);
        field("ca_layer", h.ca_layer);
        field("ui_view", h.ui_view);
        field("ui_window", h.ui_window);
        field("x11_window_id", h.x11_window_id);
        field("x11_display", h.x11_display);
        field("xcb_connection", h.xcb_connection);
        field("xcb_window_id", h.xcb_window_id);
        field("wl_display", h.wl_display);
        field("wl_surface", h.wl_surface);
        field("a_native_window", h.a_native_window);
        if (h.canvas_id != nullptr) {
            VNE_LOG_INFO << "  canvas_id    = " << h.canvas_id;
        }
    }

    void dumpGeometry() const {
        const auto pos = window_->getPosition();
        VNE_LOG_INFO << "--- geometry (what a swapchain is sized from) ---";
        VNE_LOG_INFO << "  window id    = " << static_cast<std::uint32_t>(window_->getId());
        VNE_LOG_INFO << "  logical      = " << window_->getWidth() << " x " << window_->getHeight();
        VNE_LOG_INFO << "  framebuffer  = " << window_->getFramebufferWidth() << " x "
                     << window_->getFramebufferHeight();
        VNE_LOG_INFO << "  dpi_scale    = " << window_->getDpiScale();
        VNE_LOG_INFO << "  position     = (" << pos.x << ", " << pos.y << ")";
    }

    void dumpCapabilities() const {
        VNE_LOG_INFO << "--- capabilities ---";
        VNE_LOG_INFO << "  multi-window = " << (mgr_->supportsMultipleWindows() ? "yes" : "no");

        // getMonitorCount() has no backend implementation yet; it reports the interface default.
        VNE_LOG_INFO << "  monitors     = " << mgr_->getMonitorCount()
                     << " (0 means no backend implements monitor enumeration yet)";

        std::string supported;
        for (const char* feature : {"resize", "dpi", "fullscreen", "transparency", "clipboard", "multi_window"}) {
            if (mgr_->isFeatureSupported(feature)) {
                supported += supported.empty() ? feature : (std::string(", ") + feature);
            }
        }
        VNE_LOG_INFO << "  features     = " << (supported.empty() ? "(none reported)" : supported);
    }

    vne::xwin::IWindow* window_ = nullptr;
    vne::xwin::IWindowManager* mgr_ = nullptr;
};

std::unique_ptr<vne::xwin::examples::ExampleBase> createExample() {
    return std::make_unique<NativeHandleExample>();
}
