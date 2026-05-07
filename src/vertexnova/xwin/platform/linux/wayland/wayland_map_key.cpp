/*
 * XKB keysym → vne::events::KeyCode.
 * XKB_KEY_* constants are numerically identical to X11 XK_* constants.
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

#include "wayland_map_key.h"

#include <cstdint>
#include <xkbcommon/xkbcommon.h>

namespace vne::xwin {

namespace {
constexpr std::uint32_t kAsciiPrintableMin = 32U;   // space
constexpr std::uint32_t kAsciiPrintableMax = 126U;  // '~'
constexpr std::uint32_t kModifierShiftBit = 0U;     // ShiftMask bit index
constexpr std::uint32_t kModifierControlBit = 2U;   // ControlMask
constexpr std::uint32_t kModifierMod1Bit = 3U;      // Mod1Mask (Alt)
constexpr std::uint32_t kModifierMod4Bit = 6U;      // Mod4Mask (Super)
constexpr std::uint64_t kKeysymScanStart = 32U;
constexpr std::uint64_t kKeysymScanEnd = 0x10000U;
}  // namespace

using vne::events::KeyCode;
using vne::events::ModifierKey;

// clang-format off
KeyCode mapWaylandKeysym(uint32_t sym) {
    // Lowercase a-z
    if (sym >= XKB_KEY_a && sym <= XKB_KEY_z) {
        return static_cast<KeyCode>(static_cast<int>(KeyCode::eA) + static_cast<int>(sym - XKB_KEY_a));
    }
    // Uppercase A-Z
    if (sym >= XKB_KEY_A && sym <= XKB_KEY_Z) {
        return static_cast<KeyCode>(static_cast<int>(KeyCode::eA) + static_cast<int>(sym - XKB_KEY_A));
    }
    // Digits 0-9
    if (sym >= XKB_KEY_0 && sym <= XKB_KEY_9) {
        return static_cast<KeyCode>(static_cast<int>(KeyCode::e0) + static_cast<int>(sym - XKB_KEY_0));
    }
    // Function keys F1-F25
    if (sym >= XKB_KEY_F1 && sym <= XKB_KEY_F25) {
        return static_cast<KeyCode>(static_cast<int>(KeyCode::eF1) + static_cast<int>(sym - XKB_KEY_F1));
    }
    // Printable ASCII passthrough (space .. tilde)
    if (sym >= kAsciiPrintableMin && sym <= kAsciiPrintableMax) {
        return static_cast<KeyCode>(static_cast<std::int16_t>(sym));
    }

    switch (sym) {
        case XKB_KEY_space:         return KeyCode::eSpace;
        case XKB_KEY_apostrophe:    return KeyCode::eApostrophe;
        case XKB_KEY_comma:         return KeyCode::eComma;
        case XKB_KEY_minus:         return KeyCode::eMinus;
        case XKB_KEY_period:        return KeyCode::ePeriod;
        case XKB_KEY_slash:         return KeyCode::eSlash;
        case XKB_KEY_semicolon:     return KeyCode::eSemicolon;
        case XKB_KEY_equal:         return KeyCode::eEqual;
        case XKB_KEY_bracketleft:   return KeyCode::eLeftBracket;
        case XKB_KEY_backslash:     return KeyCode::eBackslash;
        case XKB_KEY_bracketright:  return KeyCode::eRightBracket;
        case XKB_KEY_grave:         return KeyCode::eGraveAccent;
        case XKB_KEY_BackSpace:     return KeyCode::eBackspace;
        case XKB_KEY_Tab:           return KeyCode::eTab;
        case XKB_KEY_Return:        return KeyCode::eEnter;
        case XKB_KEY_Pause:         return KeyCode::ePause;
        case XKB_KEY_Scroll_Lock:   return KeyCode::eScrollLock;
        case XKB_KEY_Escape:        return KeyCode::eEscape;
        case XKB_KEY_Delete:        return KeyCode::eDelete;
        case XKB_KEY_Insert:        return KeyCode::eInsert;
        case XKB_KEY_Home:          return KeyCode::eHome;
        case XKB_KEY_End:           return KeyCode::eEnd;
        case XKB_KEY_Page_Up:       return KeyCode::ePageUp;
        case XKB_KEY_Page_Down:     return KeyCode::ePageDown;
        case XKB_KEY_Left:          return KeyCode::eLeft;
        case XKB_KEY_Up:            return KeyCode::eUp;
        case XKB_KEY_Right:         return KeyCode::eRight;
        case XKB_KEY_Down:          return KeyCode::eDown;
        case XKB_KEY_Num_Lock:      return KeyCode::eNumLock;
        case XKB_KEY_Print:         return KeyCode::ePrintScreen;
        case XKB_KEY_Caps_Lock:     return KeyCode::eCapsLock;
        case XKB_KEY_Shift_L:       return KeyCode::eLeftShift;
        case XKB_KEY_Shift_R:       return KeyCode::eRightShift;
        case XKB_KEY_Control_L:     return KeyCode::eLeftControl;
        case XKB_KEY_Control_R:     return KeyCode::eRightControl;
        case XKB_KEY_Alt_L:
        case XKB_KEY_Meta_L:        return KeyCode::eLeftAlt;
        case XKB_KEY_Alt_R:
        case XKB_KEY_Meta_R:        return KeyCode::eRightAlt;
        case XKB_KEY_Super_L:       return KeyCode::eLeftSuper;
        case XKB_KEY_Super_R:       return KeyCode::eRightSuper;
        case XKB_KEY_Menu:          return KeyCode::eMenu;
        case XKB_KEY_KP_0:          return KeyCode::eKp0;
        case XKB_KEY_KP_1:          return KeyCode::eKp1;
        case XKB_KEY_KP_2:          return KeyCode::eKp2;
        case XKB_KEY_KP_3:          return KeyCode::eKp3;
        case XKB_KEY_KP_4:          return KeyCode::eKp4;
        case XKB_KEY_KP_5:          return KeyCode::eKp5;
        case XKB_KEY_KP_6:          return KeyCode::eKp6;
        case XKB_KEY_KP_7:          return KeyCode::eKp7;
        case XKB_KEY_KP_8:          return KeyCode::eKp8;
        case XKB_KEY_KP_9:          return KeyCode::eKp9;
        case XKB_KEY_KP_Decimal:    return KeyCode::eKpDecimal;
        case XKB_KEY_KP_Divide:     return KeyCode::eKpDivide;
        case XKB_KEY_KP_Multiply:   return KeyCode::eKpMultiply;
        case XKB_KEY_KP_Subtract:   return KeyCode::eKpSubtract;
        case XKB_KEY_KP_Add:        return KeyCode::eKpAdd;
        case XKB_KEY_KP_Enter:      return KeyCode::eKpEnter;
        case XKB_KEY_KP_Equal:      return KeyCode::eKpEqual;
        default:                    return KeyCode::eUnknown;
    }
}
// clang-format on

uint8_t mapWaylandModifiers(uint32_t depressed, uint32_t latched, uint32_t locked) {
    // XKB modifier indices: Shift=1, Lock=2, Control=4, Mod1(Alt)=8, Mod4(Super)=64
    // These are the standard X11/XKB modifier masks; actual bit positions depend on the keymap,
    // but these are almost universally correct for a standard PC layout.
    const uint32_t active = depressed | latched | locked;
    uint8_t mods = 0;
    if ((active & (1U << kModifierShiftBit)) != 0U) {
        mods |= static_cast<uint8_t>(ModifierKey::eModShift);
    }
    if ((active & (1U << kModifierControlBit)) != 0U) {
        mods |= static_cast<uint8_t>(ModifierKey::eModCtrl);
    }
    if ((active & (1U << kModifierMod1Bit)) != 0U) {
        mods |= static_cast<uint8_t>(ModifierKey::eModAlt);
    }
    if ((active & (1U << kModifierMod4Bit)) != 0U) {
        mods |= static_cast<uint8_t>(ModifierKey::eModSuper);
    }
    return mods;
}

std::uint64_t mapEventsKeyCodeToWaylandKeysym(KeyCode target) {
    if (target == KeyCode::eUnknown) {
        return 0;
    }
    for (std::uint64_t sym = kKeysymScanStart; sym < kKeysymScanEnd; ++sym) {
        if (mapWaylandKeysym(static_cast<uint32_t>(sym)) == target) {
            return sym;
        }
    }
    return 0;
}

}  // namespace vne::xwin
