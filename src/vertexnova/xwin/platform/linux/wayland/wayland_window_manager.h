#pragma once
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

#include "vertexnova/xwin/window_manager.h"

#include <chrono>
#include <memory>
#include <string>
#include <vector>

struct wl_display;
struct wl_registry;
struct wl_compositor;
struct wl_seat;
struct wl_surface;
struct wl_keyboard;
struct wl_pointer;
struct wl_touch;
struct wl_output;
struct xdg_wm_base;

namespace vne::xwin {

class WaylandWindow_C;

class WaylandWindowManager_C final : public WindowManager_I {
   public:
    void NotifyWindowEvent(Window_I* window, const WindowEventData_C& event);
    const EventBridgeCallbacks& eventBridgeCallbacks() const { return _event_bridge_callbacks; }

    /** @brief Bound from wl_registry global callback (xdg-shell + compositor + seat). */
    void on_registry_global(struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version);

    // Seat capability callbacks — called from wl_seat_listener
    void on_seat_capabilities(struct wl_seat* seat, uint32_t capabilities);

    // Input event callbacks — called from wl_keyboard/pointer/touch listeners
    void on_key(uint32_t key, uint32_t state, uint32_t time);
    void on_modifiers(uint32_t depressed, uint32_t latched, uint32_t locked);
    void on_pointer_button(uint32_t button, uint32_t state, double x, double y);
    void on_pointer_motion(double x, double y);
    void on_pointer_axis(double x_offset, double y_offset);
    void on_output_scale(int32_t factor);
    void on_touch_down(uint32_t id, double x, double y);
    void on_touch_up(uint32_t id, double x, double y);
    void on_touch_motion(uint32_t id, double x, double y);

    wl_display* NativeDisplay() const { return _display; }
    wl_compositor* NativeCompositor() const { return _compositor; }
    xdg_wm_base* NativeXdgWmBase() const { return _xdg_wm_base; }
    float OutputScale() const { return _output_scale; }

    WaylandWindowManager_C();
    ~WaylandWindowManager_C() override;

    bool Initialize() override;
    void Shutdown() override;
    bool IsInitialized() const override;

    std::shared_ptr<Window_I> CreateWindow(const WindowDescriptor_C& descriptor) override;
    std::shared_ptr<Window_I> CreateWindow(const std::string& title, uint32_t width, uint32_t height) override;
    void DestroyWindow(std::shared_ptr<Window_I> window) override;
    void DestroyAllWindows() override;

    size_t GetWindowCount() const override;
    std::vector<std::shared_ptr<Window_I>> GetWindows() const override;
    std::shared_ptr<Window_I> GetPrimaryWindow() const override;
    std::shared_ptr<Window_I> GetFocusedWindow() const override;
    void SetPrimaryWindow(std::shared_ptr<Window_I> window) override;
    void FocusWindow(std::shared_ptr<Window_I> window) override;

    void ProcessEvents() override;
    void SetEventCallback(const WindowManagerEventCallback_T& callback) override;
    void setEventBridgeCallbacks(EventBridgeCallbacks callbacks) override;
    bool ShouldClose() const override;
    bool ShouldCloseAll() const override;

    WindowAPI_TP GetWindowAPI() const override;
    std::string GetPlatformInfo() const override;
    bool IsFeatureSupported(const std::string& feature) const override;
    std::string GetProperties() const override;
    void SetProperties(const std::string& properties) override;

    uint64_t GetCurrentTime() const override;
    void Sleep(uint32_t milliseconds) const override;
    double GetPlatformTime() const override;

   private:
    void bind_compositor(struct wl_registry* registry, uint32_t name, uint32_t version);
    void bind_xdg_wm_base(struct wl_registry* registry, uint32_t name, uint32_t version);
    void bind_seat(struct wl_registry* registry, uint32_t name, uint32_t version);
    void teardown_globals();

    /** @brief Return the focused window (or primary fallback) for input routing. */
    WaylandWindow_C* focused_window() const;

    WaylandWindow_C* window_for_surface(struct wl_surface* surface) const;
    void on_keyboard_enter(struct wl_surface* surface);
    void on_keyboard_leave(struct wl_surface* surface);
    void notify_window_focus(WaylandWindow_C* win, bool focused);

    wl_surface* _kbd_focus_surface = nullptr;

    wl_display* _display = nullptr;
    wl_registry* _registry = nullptr;
    wl_compositor* _compositor = nullptr;
    xdg_wm_base* _xdg_wm_base = nullptr;
    wl_seat* _seat = nullptr;
    wl_keyboard* _keyboard = nullptr;
    wl_pointer* _pointer = nullptr;
    wl_touch* _wl_touch = nullptr;
    wl_output* _output = nullptr;
    int32_t _output_scale = 1;

    // Modifier state accumulated from wl_keyboard::modifiers event
    uint32_t _mod_depressed = 0;
    uint32_t _mod_latched = 0;
    uint32_t _mod_locked = 0;

    // Last known pointer position (needed for button events that don't re-send coords)
    double _ptr_x = 0.0;
    double _ptr_y = 0.0;

    std::vector<std::shared_ptr<Window_I>> _windows;
    std::shared_ptr<Window_I> _primary;
    std::shared_ptr<Window_I> _focused;
    WindowManagerEventCallback_T _callback{};
    EventBridgeCallbacks _event_bridge_callbacks{};
    bool _initialized = false;
    std::string _properties;
};

}  // namespace vne::xwin
