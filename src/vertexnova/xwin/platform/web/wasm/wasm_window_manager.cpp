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

#include "wasm_window_manager.h"

#include "wasm_window.h"

#include <algorithm>
#include <chrono>
#include <thread>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

namespace vne::xwin {

WasmWindowManager_C::WasmWindowManager_C() = default;

WasmWindowManager_C::~WasmWindowManager_C() {
    Shutdown();
}

void WasmWindowManager_C::NotifyWindowEvent(Window_I* window, const WindowEventData_C& event) {
    if (_callback && window) {
        _callback(window, event);
    }
}

bool WasmWindowManager_C::Initialize() {
    _initialized = true;
    return true;
}

void WasmWindowManager_C::Shutdown() {
    DestroyAllWindows();
    _initialized = false;
}

bool WasmWindowManager_C::IsInitialized() const {
    return _initialized;
}

std::shared_ptr<Window_I> WasmWindowManager_C::CreateWindow(const WindowDescriptor_C& descriptor) {
    if (!_initialized) {
        return nullptr;
    }
    auto w = std::make_shared<WasmWindow_C>();
    w->SetEventOwner(this);
    w->Initialize(descriptor);
    _windows.push_back(w);
    if (!_primary) {
        _primary = w;
    }
    _focused = w;
    return w;
}

std::shared_ptr<Window_I> WasmWindowManager_C::CreateWindow(const std::string& title, uint32_t width, uint32_t height) {
    WindowDescriptor_C d(title, width, height);
    return CreateWindow(d);
}

void WasmWindowManager_C::DestroyWindow(std::shared_ptr<Window_I> window) {
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

void WasmWindowManager_C::DestroyAllWindows() {
    for (auto& w : _windows) {
        if (w) {
            w->Close();
        }
    }
    _windows.clear();
    _primary.reset();
    _focused.reset();
}

size_t WasmWindowManager_C::GetWindowCount() const {
    return _windows.size();
}

std::vector<std::shared_ptr<Window_I>> WasmWindowManager_C::GetWindows() const {
    return _windows;
}

std::shared_ptr<Window_I> WasmWindowManager_C::GetPrimaryWindow() const {
    return _primary;
}

std::shared_ptr<Window_I> WasmWindowManager_C::GetFocusedWindow() const {
    return _focused;
}

void WasmWindowManager_C::SetPrimaryWindow(std::shared_ptr<Window_I> window) {
    _primary = std::move(window);
}

void WasmWindowManager_C::FocusWindow(std::shared_ptr<Window_I> window) {
    _focused = std::move(window);
}

void WasmWindowManager_C::ProcessEvents() {}

void WasmWindowManager_C::SetEventCallback(const WindowManagerEventCallback_T& callback) {
    _callback = callback;
}

bool WasmWindowManager_C::ShouldClose() const {
    for (const auto& w : _windows) {
        if (w && !w->IsOpen()) {
            return true;
        }
    }
    return false;
}

bool WasmWindowManager_C::ShouldCloseAll() const {
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

WindowAPI_TP WasmWindowManager_C::GetWindowAPI() const {
    return WindowAPI_TP::WASM_WINDOW;
}

std::string WasmWindowManager_C::GetPlatformInfo() const {
    return "WebAssembly / Emscripten";
}

bool WasmWindowManager_C::IsFeatureSupported(const std::string& feature) const {
    return feature == "resize" || feature == "canvas" || feature == "fullscreen";
}

std::string WasmWindowManager_C::GetProperties() const {
    return _properties;
}

void WasmWindowManager_C::SetProperties(const std::string& properties) {
    _properties = properties;
}

uint64_t WasmWindowManager_C::GetCurrentTime() const {
#ifdef __EMSCRIPTEN__
    return static_cast<uint64_t>(emscripten_get_now());
#else
    using namespace std::chrono;
    return static_cast<uint64_t>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
#endif
}

void WasmWindowManager_C::Sleep(uint32_t milliseconds) const {
#ifdef __EMSCRIPTEN__
    emscripten_sleep(milliseconds);
#else
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
#endif
}

double WasmWindowManager_C::GetPlatformTime() const {
#ifdef __EMSCRIPTEN__
    return emscripten_get_now();
#else
    using namespace std::chrono;
    return duration<double>(steady_clock::now().time_since_epoch()).count();
#endif
}

}  // namespace vne::xwin
