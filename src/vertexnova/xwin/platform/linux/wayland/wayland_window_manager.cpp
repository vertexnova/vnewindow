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

#include "wayland_window_manager.h"

#include "wayland_window.h"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <thread>

#include <wayland-client.h>

extern "C" {
#include "xdg-shell-client-protocol.h"
}

namespace vne::xwin {

namespace {

void registry_global(void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version) {
    static_cast<WaylandWindowManager_C*>(data)->on_registry_global(registry, name, interface, version);
}

void registry_global_remove(void*, struct wl_registry*, uint32_t) {}

const wl_registry_listener kRegistryListener = {
    registry_global,
    registry_global_remove,
};

void xdg_wm_base_ping(void*, struct xdg_wm_base* xdg_wm_base, uint32_t serial) {
    xdg_wm_base_pong(xdg_wm_base, serial);
}

const xdg_wm_base_listener kXdgWmBaseListener = {
    xdg_wm_base_ping,
};

}  // namespace

void WaylandWindowManager_C::on_registry_global(struct wl_registry* registry,
                                                uint32_t name,
                                                const char* interface,
                                                uint32_t version) {
    if (std::strcmp(interface, wl_compositor_interface.name) == 0) {
        bind_compositor(registry, name, version);
    } else if (std::strcmp(interface, xdg_wm_base_interface.name) == 0) {
        bind_xdg_wm_base(registry, name, version);
    }
}

void WaylandWindowManager_C::bind_compositor(struct wl_registry* registry, uint32_t name, uint32_t version) {
    (void)version;
    _compositor = static_cast<wl_compositor*>(wl_registry_bind(registry, name, &wl_compositor_interface, 4));
}

void WaylandWindowManager_C::bind_xdg_wm_base(struct wl_registry* registry, uint32_t name, uint32_t version) {
    const uint32_t ver = version < 4 ? version : 4;
    _xdg_wm_base = static_cast<xdg_wm_base*>(wl_registry_bind(registry, name, &xdg_wm_base_interface, ver));
    if (_xdg_wm_base) {
        xdg_wm_base_add_listener(_xdg_wm_base, &kXdgWmBaseListener, nullptr);
    }
}

WaylandWindowManager_C::WaylandWindowManager_C() = default;

WaylandWindowManager_C::~WaylandWindowManager_C() {
    Shutdown();
}

void WaylandWindowManager_C::NotifyWindowEvent(Window_I* window, const WindowEventData_C& event) {
    if (_callback && window) {
        _callback(window, event);
    }
}

bool WaylandWindowManager_C::Initialize() {
    _display = wl_display_connect(nullptr);
    if (!_display) {
        return false;
    }
    _registry = wl_display_get_registry(_display);
    if (!_registry) {
        wl_display_disconnect(_display);
        _display = nullptr;
        return false;
    }
    wl_registry_add_listener(_registry, &kRegistryListener, this);
    if (wl_display_roundtrip(_display) < 0) {
        Shutdown();
        return false;
    }
    if (!_compositor || !_xdg_wm_base) {
        Shutdown();
        return false;
    }
    _initialized = true;
    return true;
}

void WaylandWindowManager_C::teardown_globals() {
    if (_xdg_wm_base) {
        xdg_wm_base_destroy(_xdg_wm_base);
        _xdg_wm_base = nullptr;
    }
    if (_compositor) {
        wl_compositor_destroy(_compositor);
        _compositor = nullptr;
    }
    if (_registry) {
        wl_registry_destroy(_registry);
        _registry = nullptr;
    }
    if (_display) {
        wl_display_disconnect(_display);
        _display = nullptr;
    }
}

void WaylandWindowManager_C::Shutdown() {
    DestroyAllWindows();
    teardown_globals();
    _initialized = false;
}

bool WaylandWindowManager_C::IsInitialized() const {
    return _initialized;
}

std::shared_ptr<Window_I> WaylandWindowManager_C::CreateWindow(const WindowDescriptor_C& descriptor) {
    if (!_initialized || !_display || !_compositor || !_xdg_wm_base) {
        return nullptr;
    }
    auto w = std::make_shared<WaylandWindow_C>();
    w->SetOwner(this);
    w->Initialize(descriptor);
    if (!w->IsOpen()) {
        return nullptr;
    }
    _windows.push_back(w);
    if (!_primary) {
        _primary = w;
    }
    _focused = w;
    return w;
}

std::shared_ptr<Window_I> WaylandWindowManager_C::CreateWindow(const std::string& title,
                                                               uint32_t width,
                                                               uint32_t height) {
    WindowDescriptor_C d(title, width, height);
    return CreateWindow(d);
}

void WaylandWindowManager_C::DestroyWindow(std::shared_ptr<Window_I> window) {
    if (!window) {
        return;
    }
    window->Close();
    auto it = std::find(_windows.begin(), _windows.end(), window);
    if (it != _windows.end()) {
        _windows.erase(it);
    }
    if (_primary == window) {
        _primary = _windows.empty() ? nullptr : _windows.front();
    }
    if (_focused == window) {
        _focused = _primary;
    }
}

void WaylandWindowManager_C::DestroyAllWindows() {
    for (auto& w : _windows) {
        if (w) {
            w->Close();
        }
    }
    _windows.clear();
    _primary.reset();
    _focused.reset();
}

size_t WaylandWindowManager_C::GetWindowCount() const {
    return _windows.size();
}

std::vector<std::shared_ptr<Window_I>> WaylandWindowManager_C::GetWindows() const {
    return _windows;
}

std::shared_ptr<Window_I> WaylandWindowManager_C::GetPrimaryWindow() const {
    return _primary;
}

std::shared_ptr<Window_I> WaylandWindowManager_C::GetFocusedWindow() const {
    return _focused;
}

void WaylandWindowManager_C::SetPrimaryWindow(std::shared_ptr<Window_I> window) {
    _primary = std::move(window);
}

void WaylandWindowManager_C::FocusWindow(std::shared_ptr<Window_I> window) {
    _focused = std::move(window);
}

void WaylandWindowManager_C::ProcessEvents() {
    if (_display) {
        wl_display_dispatch_pending(_display);
    }
}

void WaylandWindowManager_C::SetEventCallback(const WindowManagerEventCallback_T& callback) {
    _callback = callback;
}

void WaylandWindowManager_C::SetVneEventCallbacks(XWinVneEventCallbacks_C callbacks) {
    _vne_callbacks = std::move(callbacks);
}

bool WaylandWindowManager_C::ShouldClose() const {
    for (const auto& w : _windows) {
        if (w && !w->IsOpen()) {
            return true;
        }
    }
    return false;
}

bool WaylandWindowManager_C::ShouldCloseAll() const {
    if (_windows.empty()) {
        return false;
    }
    for (const auto& w : _windows) {
        if (w && w->IsOpen()) {
            return false;
        }
    }
    return true;
}

WindowAPI_TP WaylandWindowManager_C::GetWindowAPI() const {
    return WindowAPI_TP::WAYLAND_WINDOW;
}

std::string WaylandWindowManager_C::GetPlatformInfo() const {
    return "Linux / Wayland (xdg-shell)";
}

bool WaylandWindowManager_C::IsFeatureSupported(const std::string& feature) const {
    return feature == "resize" || feature == "close" || feature == "wayland";
}

std::string WaylandWindowManager_C::GetProperties() const {
    return _properties;
}

void WaylandWindowManager_C::SetProperties(const std::string& properties) {
    _properties = properties;
}

uint64_t WaylandWindowManager_C::GetCurrentTime() const {
    using namespace std::chrono;
    return static_cast<uint64_t>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}

void WaylandWindowManager_C::Sleep(uint32_t milliseconds) const {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

double WaylandWindowManager_C::GetPlatformTime() const {
    using namespace std::chrono;
    return duration<double>(steady_clock::now().time_since_epoch()).count();
}

}  // namespace vne::xwin
