#pragma once
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

/** @file input_mapping.h Native input tokens ↔ vne::events types, with optional per-window overrides.
 *
 * Built-in tables cover Win32, Cocoa, X11, and Wayland. UIKit uses the same Cocoa-style key / button /
 * modifier mapping for pointer and hardware keyboard (e.g. Simulator mouse, iPad trackpad); finger
 * input is still delivered as touch. Android remains touch-first with no default key/mouse tables.
 */

#include "vertexnova/xwin/xwin_export.h"
#include "vertexnova/xwin/xwin_types.h"

#include <vertexnova/events/types.h>

#include <cstdint>
#include <functional>

namespace vne::xwin {

/**
 * Optional per-window translation hooks. If a slot is empty, or returns eUnknown / zero where
 * documented, the library falls back to built-in platform tables (Win32 VK, Cocoa CGKeyCode,
 * X11/Wayland keysym, etc.). Android has no default tables; UIKit uses these hooks when routing
 * pointer/keyboard through the shared Cocoa-compatible mapping.
 *
 * Native payload packing is API-specific (see packWin32NativeKey, packCocoaNativeKey, …).
 */
struct WindowInputMapping {
    std::function<vne::events::KeyCode(WindowAPI api, uint64_t native_key_packed)> native_key_to_events;
    std::function<uint64_t(WindowAPI api, vne::events::KeyCode key)> events_key_to_native_packed;

    std::function<vne::events::MouseButton(WindowAPI api, uint64_t native_mouse_packed)> native_mouse_to_events;
    std::function<uint64_t(WindowAPI api, vne::events::MouseButton button)> events_mouse_to_native_packed;

    /** Platform-native modifier representation (e.g. NSEvent.modifierFlags on Cocoa, uint8_t flags on Win32). */
    std::function<uint8_t(WindowAPI api, uint64_t native_modifiers_packed)> native_modifiers_to_events;
    std::function<uint64_t(WindowAPI api, uint8_t events_modifiers)> events_modifiers_to_native_packed;
};

// -----------------------------------------------------------------------------
// Native token packing (inline; documented per backend)
// -----------------------------------------------------------------------------

/** @brief Win32 WM_KEY* token: low 16 = VK; bits 16–23 = scan; bit 24 = extended (from lParam). */
inline uint64_t packWin32NativeKey(std::uintptr_t vk, std::uintptr_t l_param) {
    const std::uint32_t scan = (static_cast<std::uint32_t>(l_param) >> 16U) & 0xFFU;
    const std::uint32_t ext = (static_cast<std::uint32_t>(l_param) & (1U << 24)) != 0 ? 1U : 0U;
    uint64_t p = static_cast<uint64_t>(vk) & 0xFFFFULL;
    p |= (static_cast<uint64_t>(scan) << 16);
    p |= (static_cast<uint64_t>(ext) << 24);
    return p;
}

inline void unpackWin32NativeKey(uint64_t packed, std::uintptr_t* vk_out, std::uintptr_t* l_param_out) {
    *vk_out = static_cast<std::uintptr_t>(packed & 0xFFFFULL);
    const std::uint32_t scan = static_cast<std::uint32_t>((packed >> 16) & 0xFFULL);
    const std::uint32_t ext = static_cast<std::uint32_t>((packed >> 24) & 1ULL);
    std::uintptr_t lp = static_cast<std::uintptr_t>(scan) << 16U;
    if (ext != 0U) {
        lp |= static_cast<std::uintptr_t>(1U) << 24U;
    }
    *l_param_out = lp;
}

/** @brief Win32 mouse: low 32 = UINT msg, high 32 = WPARAM for XBUTTON. */
inline uint64_t packWin32Mouse(unsigned int msg, std::uintptr_t w_param) {
    uint64_t p = static_cast<uint64_t>(msg) & 0xFFFFFFFFULL;
    p |= (static_cast<uint64_t>(w_param) << 32);
    return p;
}

inline void unpackWin32Mouse(uint64_t packed, unsigned int* msg_out, std::uintptr_t* w_param_out) {
    *msg_out = static_cast<unsigned int>(packed & 0xFFFFFFFFULL);
    *w_param_out = static_cast<std::uintptr_t>(packed >> 32);
}

/** @brief Cocoa / UIKit NSEvent.keyCode (CGKeyCode). */
inline uint64_t packCocoaNativeKey(std::uint16_t key_code) {
    return static_cast<uint64_t>(key_code);
}

/** @brief X11 KeySym or Wayland xkb_keysym_t (fits 32 bits). */
inline uint64_t packXkbNativeKey(std::uint32_t keysym) {
    return static_cast<uint64_t>(keysym);
}

/** @brief Cocoa/UIKit NSEvent.buttonNumber (0–7 align with vne::events::MouseButton indices). */
inline uint64_t packCocoaNativeMouse(std::uint16_t button_number) {
    return static_cast<uint64_t>(button_number);
}

/** @brief X11 ButtonPress/ButtonRelease button field (1-based button index). */
inline uint64_t packX11NativeMouse(std::uint32_t x11_button) {
    return static_cast<uint64_t>(x11_button);
}

/** @brief Linux evdev button code (e.g. BTN_LEFT) as used by Wayland pointer protocol. */
inline uint64_t packWaylandNativeMouse(std::uint32_t linux_input_button) {
    return static_cast<uint64_t>(linux_input_button);
}

// -----------------------------------------------------------------------------
// Mapping API (built-in tables + optional WindowInputMapping from WindowDescriptor)
// -----------------------------------------------------------------------------

/** Uses mapping from WindowDescriptor when non-null; otherwise built-ins only. */
VNE_XWIN_API vne::events::KeyCode mapNativeKeyToEvents(WindowAPI api,
                                                       uint64_t native_key_packed,
                                                       const WindowInputMapping* mapping = nullptr);

VNE_XWIN_API uint64_t mapEventsKeyToNativePacked(WindowAPI api,
                                                 vne::events::KeyCode key,
                                                 const WindowInputMapping* mapping = nullptr);

VNE_XWIN_API vne::events::MouseButton mapNativeMouseToEvents(WindowAPI api,
                                                             uint64_t native_mouse_packed,
                                                             const WindowInputMapping* mapping = nullptr);

VNE_XWIN_API uint64_t mapEventsMouseToNativePacked(WindowAPI api,
                                                   vne::events::MouseButton button,
                                                   const WindowInputMapping* mapping = nullptr);

VNE_XWIN_API std::uint8_t mapNativeModifiersToEvents(WindowAPI api,
                                                     std::uint64_t native_modifiers_packed,
                                                     const WindowInputMapping* mapping = nullptr);

VNE_XWIN_API std::uint64_t mapEventsModifiersToNativePacked(WindowAPI api,
                                                            std::uint8_t events_modifiers,
                                                            const WindowInputMapping* mapping = nullptr);

/** Built-in tables only (ignores WindowInputMapping). */
VNE_XWIN_API vne::events::KeyCode mapNativeKeyToEventsDefault(WindowAPI api, uint64_t native_key_packed);

VNE_XWIN_API uint64_t mapEventsKeyToNativePackedDefault(WindowAPI api, vne::events::KeyCode key);

}  // namespace vne::xwin
