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

#include "vertexnova/xwin/input_mapping.h"

#if VNE_XWIN_HAS_WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "platform/win32/win32_map_key.h"
#endif
#if VNE_XWIN_HAS_COCOA || VNE_XWIN_HAS_UIKIT
#include "platform/cocoa/cocoa_map_key.h"
#endif
#if VNE_XWIN_HAS_X11
#include <X11/Xlib.h>
#include "platform/linux/x11/x11_map_key.h"
#endif
#if VNE_XWIN_HAS_WAYLAND
#include <linux/input-event-codes.h>
#include "platform/linux/wayland/wayland_map_key.h"
#endif

namespace vne::xwin {

using vne::events::KeyCode;
using vne::events::MouseButton;

namespace {
[[maybe_unused]] constexpr std::uint64_t kLowByteMask = 0xFFu;
[[maybe_unused]] constexpr std::uint64_t kLow16Mask = 0xFFFFu;
[[maybe_unused]] constexpr std::uint64_t kLow32Mask = 0xFFFFFFFFu;
[[maybe_unused]] constexpr std::uint8_t kMouseButtonMask = 7U;

/* Native -> MouseButton for X11/Wayland: unmapped values use vne::events::MouseButton::eUnknown (input_mapping.h;
 * TODO(vneevents): replace with MouseButton::eUnknown). */

#if VNE_XWIN_HAS_X11
MouseButton mapX11NativeButtonToMouse(unsigned int b) {
    switch (b) {
        case 1U:
            return MouseButton::eLeft;
        case 2U:
            return MouseButton::eMiddle;
        case 3U:
            return MouseButton::eRight;
        default:
            return vne::events::MouseButton::eUnknown;
    }
}

unsigned int mapMouseButtonToX11Button(MouseButton button) {
    if (button == vne::events::MouseButton::eUnknown) {
        return 0U;
    }
    switch (button) {
        case MouseButton::eLeft:
            return 1U;
        case MouseButton::eMiddle:
            return 2U;
        case MouseButton::eRight:
            return 3U;
        default:
            return 0U;
    }
}
#endif

#if VNE_XWIN_HAS_WAYLAND
MouseButton mapLinuxEvdevButtonToMouse(std::uint32_t btn) {
    switch (btn) {
        case BTN_LEFT:
            return MouseButton::eLeft;
        case BTN_RIGHT:
            return MouseButton::eRight;
        case BTN_MIDDLE:
            return MouseButton::eMiddle;
        default:
            return vne::events::MouseButton::eUnknown;
    }
}

std::uint32_t mapMouseButtonToLinuxEvdev(MouseButton button) {
    if (button == vne::events::MouseButton::eUnknown) {
        return 0U;
    }
    switch (button) {
        case MouseButton::eLeft:
            return BTN_LEFT;
        case MouseButton::eRight:
            return BTN_RIGHT;
        case MouseButton::eMiddle:
            return BTN_MIDDLE;
        default:
            return 0U;
    }
}
#endif

}  // namespace

KeyCode mapNativeKeyToEventsDefault(WindowAPI api, uint64_t native_key_packed) noexcept {
    (void)native_key_packed;
    switch (api) {
        case WindowAPI::eNullWindow:
            return KeyCode::eUnknown;
#if VNE_XWIN_HAS_WIN32
        case WindowAPI::eWin32Window: {
            std::uintptr_t vk = 0;
            std::uintptr_t lp = 0;
            unpackWin32NativeKey(native_key_packed, &vk, &lp);
            return mapWin32Key(static_cast<WPARAM>(vk), static_cast<LPARAM>(lp));
        }
#endif
#if VNE_XWIN_HAS_COCOA
        case WindowAPI::eCocoaWindow:
            return mapCocoaKeyCode(static_cast<std::uint16_t>(native_key_packed & kLow16Mask));
#endif
#if VNE_XWIN_HAS_UIKIT
        case WindowAPI::eIosUikitWindow:
            return mapCocoaKeyCode(static_cast<std::uint16_t>(native_key_packed & kLow16Mask));
#endif
#if VNE_XWIN_HAS_X11
        case WindowAPI::eX11Window:
            return mapX11Keysym(static_cast<KeySym>(native_key_packed));
#endif
#if VNE_XWIN_HAS_WAYLAND
        case WindowAPI::eWaylandWindow:
            return mapWaylandKeysym(static_cast<std::uint32_t>(native_key_packed & kLow32Mask));
#endif
        default:
            return KeyCode::eUnknown;
    }
}

uint64_t mapEventsKeyToNativePackedDefault(WindowAPI api, KeyCode key) noexcept {
#if VNE_XWIN_HAS_WIN32
    if (api == WindowAPI::eWin32Window) {
        return mapEventsKeyCodeToWin32Packed(key);
    }
#endif
#if VNE_XWIN_HAS_COCOA
    if (api == WindowAPI::eCocoaWindow) {
        return mapEventsKeyCodeToCocoaPacked(key);
    }
#endif
#if VNE_XWIN_HAS_UIKIT
    if (api == WindowAPI::eIosUikitWindow) {
        return mapEventsKeyCodeToCocoaPacked(key);
    }
#endif
#if VNE_XWIN_HAS_X11
    if (api == WindowAPI::eX11Window) {
        return mapEventsKeyCodeToX11Keysym(key);
    }
#endif
#if VNE_XWIN_HAS_WAYLAND
    if (api == WindowAPI::eWaylandWindow) {
        return mapEventsKeyCodeToWaylandKeysym(key);
    }
#endif
    (void)api;
    (void)key;
    return 0;
}

MouseButton mapNativeMouseToEventsDefault(WindowAPI api, uint64_t native_mouse_packed) noexcept {
#if VNE_XWIN_HAS_WIN32
    if (api == WindowAPI::eWin32Window) {
        unsigned int msg = 0;
        std::uintptr_t wp = 0;
        unpackWin32Mouse(native_mouse_packed, &msg, &wp);
        return mapWin32MouseButtonFromMessage(static_cast<UINT>(msg), static_cast<WPARAM>(wp));
    }
#endif
#if VNE_XWIN_HAS_COCOA
    if (api == WindowAPI::eCocoaWindow) {
        const auto bn = static_cast<std::uint32_t>(native_mouse_packed & kLowByteMask);
        if (bn <= kMouseButtonMask) {
            return static_cast<MouseButton>(bn);
        }
        return MouseButton::eLeft;
    }
#endif
#if VNE_XWIN_HAS_UIKIT
    if (api == WindowAPI::eIosUikitWindow) {
        const auto bn = static_cast<std::uint32_t>(native_mouse_packed & kLowByteMask);
        if (bn <= kMouseButtonMask) {
            return static_cast<MouseButton>(bn);
        }
        return MouseButton::eLeft;
    }
#endif
#if VNE_XWIN_HAS_X11
    if (api == WindowAPI::eX11Window) {
        return mapX11NativeButtonToMouse(static_cast<unsigned int>(native_mouse_packed & kLow32Mask));
    }
#endif
#if VNE_XWIN_HAS_WAYLAND
    if (api == WindowAPI::eWaylandWindow) {
        return mapLinuxEvdevButtonToMouse(static_cast<std::uint32_t>(native_mouse_packed & kLow32Mask));
    }
#endif
    (void)native_mouse_packed;
    (void)api;
    return vne::events::MouseButton::eUnknown;
}

uint64_t mapEventsMouseToNativePackedDefault(WindowAPI api, MouseButton button) noexcept {
#if VNE_XWIN_HAS_WIN32
    if (api == WindowAPI::eWin32Window) {
        return mapEventsMouseButtonToWin32Packed(button);
    }
#endif
#if VNE_XWIN_HAS_COCOA
    if (api == WindowAPI::eCocoaWindow) {
        if (button == vne::events::MouseButton::eUnknown) {
            return 0;
        }
        return static_cast<uint64_t>(static_cast<std::uint8_t>(button) & kMouseButtonMask);
    }
#endif
#if VNE_XWIN_HAS_UIKIT
    if (api == WindowAPI::eIosUikitWindow) {
        if (button == vne::events::MouseButton::eUnknown) {
            return 0;
        }
        return static_cast<uint64_t>(static_cast<std::uint8_t>(button) & kMouseButtonMask);
    }
#endif
#if VNE_XWIN_HAS_X11
    if (api == WindowAPI::eX11Window) {
        return static_cast<uint64_t>(mapMouseButtonToX11Button(button));
    }
#endif
#if VNE_XWIN_HAS_WAYLAND
    if (api == WindowAPI::eWaylandWindow) {
        return static_cast<uint64_t>(mapMouseButtonToLinuxEvdev(button));
    }
#endif
    (void)button;
    (void)api;
    return 0;
}

std::uint8_t mapNativeModifiersToEventsDefault(WindowAPI api, std::uint64_t native_modifiers_packed) noexcept {
#if VNE_XWIN_HAS_WIN32
    if (api == WindowAPI::eWin32Window) {
        return static_cast<std::uint8_t>(native_modifiers_packed & kLowByteMask);
    }
#endif
#if VNE_XWIN_HAS_COCOA
    if (api == WindowAPI::eCocoaWindow) {
        return mapCocoaModifiers(static_cast<unsigned long>(native_modifiers_packed));
    }
#endif
#if VNE_XWIN_HAS_UIKIT
    if (api == WindowAPI::eIosUikitWindow) {
        return mapCocoaModifiers(static_cast<unsigned long>(native_modifiers_packed));
    }
#endif
#if VNE_XWIN_HAS_X11
    if (api == WindowAPI::eX11Window) {
        return mapX11Modifiers(static_cast<unsigned int>(native_modifiers_packed));
    }
#endif
#if VNE_XWIN_HAS_WAYLAND
    if (api == WindowAPI::eWaylandWindow) {
        /* Wayland callers typically pass the byte from mapWaylandModifiers(...); custom hooks may pack differently. */
        return static_cast<std::uint8_t>(native_modifiers_packed & kLowByteMask);
    }
#endif
    (void)api;
    (void)native_modifiers_packed;
    return 0;
}

std::uint64_t mapEventsModifiersToNativePackedDefault(WindowAPI api, std::uint8_t events_modifiers) noexcept {
#if VNE_XWIN_HAS_WIN32
    if (api == WindowAPI::eWin32Window) {
        return static_cast<std::uint64_t>(events_modifiers);
    }
#endif
#if VNE_XWIN_HAS_COCOA
    if (api == WindowAPI::eCocoaWindow) {
        return static_cast<std::uint64_t>(mapEventsModifiersToCocoaFlags(events_modifiers));
    }
#endif
#if VNE_XWIN_HAS_UIKIT
    if (api == WindowAPI::eIosUikitWindow) {
        return static_cast<std::uint64_t>(mapEventsModifiersToCocoaFlags(events_modifiers));
    }
#endif
#if VNE_XWIN_HAS_X11
    if (api == WindowAPI::eX11Window) {
        unsigned int s = 0;
        if ((events_modifiers & static_cast<std::uint8_t>(vne::events::ModifierKey::eModShift)) != 0) {
            s |= ShiftMask;
        }
        if ((events_modifiers & static_cast<std::uint8_t>(vne::events::ModifierKey::eModCtrl)) != 0) {
            s |= ControlMask;
        }
        if ((events_modifiers & static_cast<std::uint8_t>(vne::events::ModifierKey::eModAlt)) != 0) {
            s |= Mod1Mask;
        }
        if ((events_modifiers & static_cast<std::uint8_t>(vne::events::ModifierKey::eModSuper)) != 0) {
            s |= Mod4Mask;
        }
        return static_cast<std::uint64_t>(s);
    }
#endif
#if VNE_XWIN_HAS_WAYLAND
    if (api == WindowAPI::eWaylandWindow) {
        return static_cast<std::uint64_t>(events_modifiers);
    }
#endif
    (void)api;
    (void)events_modifiers;
    return 0;
}

KeyCode mapNativeKeyToEvents(WindowAPI api, uint64_t native_key_packed, const WindowInputMapping* mapping) {
    if (mapping && mapping->native_key_to_events) {
        const KeyCode k = mapping->native_key_to_events(api, native_key_packed);
        if (k != KeyCode::eUnknown) {
            return k;
        }
    }
    return mapNativeKeyToEventsDefault(api, native_key_packed);
}

uint64_t mapEventsKeyToNativePacked(WindowAPI api, KeyCode key, const WindowInputMapping* mapping) {
    if (mapping && mapping->events_key_to_native_packed) {
        const uint64_t n = mapping->events_key_to_native_packed(api, key);
        if (n != 0) {
            return n;
        }
    }
    return mapEventsKeyToNativePackedDefault(api, key);
}

MouseButton mapNativeMouseToEvents(WindowAPI api, uint64_t native_mouse_packed, const WindowInputMapping* mapping) {
    if (mapping && mapping->native_mouse_to_events) {
        const MouseButton b = mapping->native_mouse_to_events(api, native_mouse_packed);
        if (b != vne::events::MouseButton::eUnknown) {
            return b;
        }
    }
    return mapNativeMouseToEventsDefault(api, native_mouse_packed);
}

uint64_t mapEventsMouseToNativePacked(WindowAPI api, MouseButton button, const WindowInputMapping* mapping) {
    if (mapping && mapping->events_mouse_to_native_packed) {
        const uint64_t n = mapping->events_mouse_to_native_packed(api, button);
        if (n != 0) {
            return n;
        }
    }
    return mapEventsMouseToNativePackedDefault(api, button);
}

std::uint8_t mapNativeModifiersToEvents(WindowAPI api,
                                        std::uint64_t native_modifiers_packed,
                                        const WindowInputMapping* mapping) {
    if (mapping && mapping->native_modifiers_to_events) {
        return mapping->native_modifiers_to_events(api, native_modifiers_packed);
    }
    return mapNativeModifiersToEventsDefault(api, native_modifiers_packed);
}

std::uint64_t mapEventsModifiersToNativePacked(WindowAPI api,
                                               std::uint8_t events_modifiers,
                                               const WindowInputMapping* mapping) {
    if (mapping && mapping->events_modifiers_to_native_packed) {
        const uint64_t n = mapping->events_modifiers_to_native_packed(api, events_modifiers);
        if (n != 0) {
            return n;
        }
    }
    return mapEventsModifiersToNativePackedDefault(api, events_modifiers);
}

}  // namespace vne::xwin
