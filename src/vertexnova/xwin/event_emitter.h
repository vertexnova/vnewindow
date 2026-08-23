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

/** @file event_emitter.h Internal bridge from platform windows to vne::events. */

#include "vertexnova/xwin/window_descriptor.h"
#include "vertexnova/xwin/xwin_export.h"
#include "vertexnova/xwin/xwin_types.h"

#include <vertexnova/events/types.h>

#include <cstdint>

namespace vne::xwin {

class IWindow;

/**
 * @brief Translates one native event into exactly one vne::events event, plus the Input mirror.
 *
 * Each backend window owns one bridge, constructed with itself and its live descriptor. That is
 * the whole subscription model: there are no per-window callbacks and no manager-level callback,
 * so a backend makes exactly one call per native event and every consumer subscribes through
 * vne::events::EventManager.
 *
 * Two descriptor flags gate the two sinks, and this is the only place they are consulted:
 *  - `enable_events` pushes a vne::events::Event onto the EventManager queue.
 *  - `enable_input`  mirrors the state into vne::events::Input for per-frame polling.
 *
 * Every emitted event is stamped with the owning window's id, so listeners can attribute it
 * without holding a raw IWindow pointer.
 *
 * @warning Not thread-safe. Call from the thread that owns the native event loop.
 */
class VNE_XWIN_API EventEmitter {
   public:
    /**
     * @param window Owning window; must outlive the bridge. Used only for its id.
     * @param descriptor The window's live descriptor. Held by reference because backends mutate
     *        their copy as the window resizes, and the gating flags must stay current.
     */
    EventEmitter(const IWindow* window, const WindowDescriptor& descriptor) noexcept
        : window_(window)
        , descriptor_(&descriptor) {}

    // -- Keyboard -----------------------------------------------------------
    void keyDown(vne::events::KeyCode key, std::uint8_t modifiers, bool repeat) const;
    void keyUp(vne::events::KeyCode key, std::uint8_t modifiers) const;
    /** @brief Committed text from the platform character/IME path. UTF-8; empty input is dropped. */
    void textInput(const char* utf8_text) const;

    // -- Pointer ------------------------------------------------------------
    void mouseButton(vne::events::MouseButton button, bool pressed, double x, double y, std::uint8_t modifiers) const;
    void mouseMove(double x, double y, std::uint8_t modifiers) const;
    void mouseScroll(float x_offset, float y_offset, double x, double y, std::uint8_t modifiers) const;

    // -- Touch --------------------------------------------------------------
    void touch(std::uint32_t touch_id, double x, double y, TouchPhase phase, std::uint8_t modifiers) const;

    /**
     * @brief Emits a process-scoped application lifecycle event.
     *
     * Static because lifecycle belongs to the process, not to any window: it carries no window id
     * and fires once per transition however many windows are open. Requiring an instance would
     * mean picking an arbitrary window just to announce that the application backgrounded.
     *
     * Unlike every other method here it consults no descriptor, so `enable_events` does not gate
     * it — that flag opts a *window* out of its own events, and this is not a window's event.
     * Window managers call this from IWindowManager::notifyApplicationLifecycle.
     */
    static void applicationLifecycle(ApplicationLifecycle transition);

    // -- Window state -------------------------------------------------------
    void windowResize(std::uint32_t width, std::uint32_t height) const;
    void windowClose() const;
    void windowFocus(bool focused) const;
    void windowMinimize() const;
    void windowRestore() const;
    void windowMove(std::int32_t x, std::int32_t y) const;
    void windowDpiChanged(float scale) const;
    void windowSafeAreaChanged(float top, float left, float bottom, float right) const;

   private:
    [[nodiscard]] vne::events::WindowId windowId() const noexcept;

    const IWindow* window_;
    const WindowDescriptor* descriptor_;
};
}  // namespace vne::xwin
