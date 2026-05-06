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

class WaylandWindow;

class WaylandWindowManager final : public IWindowManager {
   public:
    void notifyWindowEvent(IWindow* window, const WindowEventData& event);
    [[nodiscard]] const EventBridgeCallbacks& eventBridgeCallbacks() const noexcept { return event_bridge_callbacks_; }

    /** @brief Bound from wl_registry global callback (xdg-shell + compositor + seat). */
    void onRegistryGlobal(struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version);

    // Seat capability callbacks — called from wl_seat_listener
    void onSeatCapabilities(struct wl_seat* seat, uint32_t capabilities);

    // Input event callbacks — called from wl_keyboard/pointer/touch listeners
    void onKey(uint32_t key, uint32_t state, uint32_t time);
    void onModifiers(uint32_t depressed, uint32_t latched, uint32_t locked);
    void onPointerButton(uint32_t button, uint32_t state, double x, double y);
    void onPointerMotion(double x, double y);
    void onPointerAxis(double x_offset, double y_offset);
    void onOutputScale(int32_t factor);
    void onTouchDown(uint32_t id, double x, double y);
    void onTouchUp(uint32_t id, double x, double y);
    void onTouchMotion(uint32_t id, double x, double y);

    /** Called from wl_keyboard_listener thunks. */
    void onKeyboardEnter(struct wl_surface* surface);
    void onKeyboardLeave(struct wl_surface* surface);

    [[nodiscard]] wl_display* nativeDisplay() const noexcept { return display_; }
    [[nodiscard]] wl_compositor* nativeCompositor() const noexcept { return compositor_; }
    [[nodiscard]] xdg_wm_base* nativeXdgWmBase() const noexcept { return xdg_wm_base_; }
    [[nodiscard]] float outputScale() const noexcept { return static_cast<float>(output_scale_); }

    WaylandWindowManager();
    ~WaylandWindowManager() override;

    bool Initialize() override;
    void Shutdown() override;
    [[nodiscard]] bool IsInitialized() const noexcept override;

    std::shared_ptr<IWindow> OpenWindow(const WindowDescriptor& descriptor) override;
    std::shared_ptr<IWindow> OpenWindow(const std::string& title, uint32_t width, uint32_t height) override;
    void RemoveWindow(std::shared_ptr<IWindow> window) override;
    void DestroyAllWindows() override;

    [[nodiscard]] size_t GetWindowCount() const noexcept override;
    [[nodiscard]] std::vector<std::shared_ptr<IWindow>> GetWindows() const override;
    [[nodiscard]] std::shared_ptr<IWindow> GetPrimaryWindow() const noexcept override;
    [[nodiscard]] std::shared_ptr<IWindow> GetFocusedWindow() const noexcept override;
    void SetPrimaryWindow(std::shared_ptr<IWindow> window) override;
    void FocusWindow(std::shared_ptr<IWindow> window) override;

    void ProcessEvents() override;
    void SetEventCallback(const WindowManagerEventCallback_T& callback) override;
    void setEventBridgeCallbacks(EventBridgeCallbacks callbacks) override;
    [[nodiscard]] bool ShouldClose() const noexcept override;
    [[nodiscard]] bool ShouldCloseAll() const noexcept override;

    [[nodiscard]] WindowAPI GetWindowAPI() const noexcept override;
    std::string GetPlatformInfo() const override;
    [[nodiscard]] bool IsFeatureSupported(const std::string& feature) const override;
    [[nodiscard]] std::string GetProperties() const override;
    void SetProperties(const std::string& properties) override;

    [[nodiscard]] uint64_t GetCurrentTime() const noexcept override;
    void Sleep(uint32_t milliseconds) const noexcept override;
    [[nodiscard]] double GetPlatformTime() const noexcept override;

   private:
    void bindCompositor(struct wl_registry* registry, uint32_t name, uint32_t version);
    void bindXdgWmBase(struct wl_registry* registry, uint32_t name, uint32_t version);
    void bindSeat(struct wl_registry* registry, uint32_t name, uint32_t version);
    void teardownGlobals();

    /** @brief Return the focused window (or primary fallback) for input routing. */
    [[nodiscard]] WaylandWindow* focusedWindow() const;

    [[nodiscard]] WaylandWindow* windowForSurface(struct wl_surface* surface) const;
    void notifyWindowFocus(WaylandWindow* win, bool focused);

    wl_surface* kbd_focus_surface_ = nullptr;

    wl_display* display_ = nullptr;
    wl_registry* registry_ = nullptr;
    wl_compositor* compositor_ = nullptr;
    xdg_wm_base* xdg_wm_base_ = nullptr;
    wl_seat* seat_ = nullptr;
    wl_keyboard* keyboard_ = nullptr;
    wl_pointer* pointer_ = nullptr;
    wl_touch* wl_touch_ = nullptr;
    wl_output* output_ = nullptr;
    int32_t output_scale_ = 1;

    // Modifier state accumulated from wl_keyboard::modifiers event
    uint32_t mod_depressed_ = 0;
    uint32_t mod_latched_ = 0;
    uint32_t mod_locked_ = 0;

    // Last known pointer position (needed for button events that don't re-send coords)
    double ptr_x_ = 0.0;
    double ptr_y_ = 0.0;

    std::vector<std::shared_ptr<IWindow>> windows_;
    std::shared_ptr<IWindow> primary_;
    std::shared_ptr<IWindow> focused_;
    WindowManagerEventCallback_T callback_{};
    EventBridgeCallbacks event_bridge_callbacks_{};
    bool initialized_ = false;
    std::string properties_;
};

}  // namespace vne::xwin
