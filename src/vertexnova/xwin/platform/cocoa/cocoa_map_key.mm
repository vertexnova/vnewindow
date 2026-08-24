/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   April 2026
 *
 * Autodoc:   yes
 * NSEvent virtual key code (CGKeyCode) -> vne::events::KeyCode.
 * Scan codes are independent of keyboard layout (US ANSI positions).
 * Reference: IOKit/hidsystem/ev_keymap.h and Carbon HIToolbox/Events.h
 * ----------------------------------------------------------------------
 */

#include "cocoa_map_key.h"

#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#else
#import <AppKit/AppKit.h>
#endif

namespace vne::xwin {

using vne::events::KeyCode;
using vne::events::ModifierKey;

// clang-format off
KeyCode mapCocoaKeyCode(uint16_t kc) {
    switch (kc) {
        // Alphanumeric row
        case 0x00: return KeyCode::eA;
        case 0x01: return KeyCode::eS;
        case 0x02: return KeyCode::eD;
        case 0x03: return KeyCode::eF;
        case 0x04: return KeyCode::eH;
        case 0x05: return KeyCode::eG;
        case 0x06: return KeyCode::eZ;
        case 0x07: return KeyCode::eX;
        case 0x08: return KeyCode::eC;
        case 0x09: return KeyCode::eV;
        case 0x0B: return KeyCode::eB;
        case 0x0C: return KeyCode::eQ;
        case 0x0D: return KeyCode::eW;
        case 0x0E: return KeyCode::eE;
        case 0x0F: return KeyCode::eR;
        case 0x10: return KeyCode::eY;
        case 0x11: return KeyCode::eT;
        case 0x12: return KeyCode::e1;
        case 0x13: return KeyCode::e2;
        case 0x14: return KeyCode::e3;
        case 0x15: return KeyCode::e4;
        case 0x16: return KeyCode::e6;
        case 0x17: return KeyCode::e5;
        case 0x18: return KeyCode::eEqual;
        case 0x19: return KeyCode::e9;
        case 0x1A: return KeyCode::e7;
        case 0x1B: return KeyCode::eMinus;
        case 0x1C: return KeyCode::e8;
        case 0x1D: return KeyCode::e0;
        case 0x1E: return KeyCode::eRightBracket;
        case 0x1F: return KeyCode::eO;
        case 0x20: return KeyCode::eU;
        case 0x21: return KeyCode::eLeftBracket;
        case 0x22: return KeyCode::eI;
        case 0x23: return KeyCode::eP;
        case 0x25: return KeyCode::eL;
        case 0x26: return KeyCode::eJ;
        case 0x27: return KeyCode::eApostrophe;
        case 0x28: return KeyCode::eK;
        case 0x29: return KeyCode::eSemicolon;
        case 0x2A: return KeyCode::eBackslash;
        case 0x2B: return KeyCode::eComma;
        case 0x2C: return KeyCode::eSlash;
        case 0x2D: return KeyCode::eN;
        case 0x2E: return KeyCode::eM;
        case 0x2F: return KeyCode::ePeriod;
        case 0x32: return KeyCode::eGraveAccent;
        // Function / control
        case 0x24: return KeyCode::eEnter;
        case 0x30: return KeyCode::eTab;
        case 0x31: return KeyCode::eSpace;
        case 0x33: return KeyCode::eBackspace;
        case 0x35: return KeyCode::eEscape;
        case 0x39: return KeyCode::eCapsLock;
        // Modifier keys (sent via flagsChanged:)
        case 0x37: return KeyCode::eLeftSuper;
        case 0x38: return KeyCode::eLeftShift;
        case 0x3A: return KeyCode::eLeftAlt;
        case 0x3B: return KeyCode::eLeftControl;
        case 0x3C: return KeyCode::eRightShift;
        case 0x3D: return KeyCode::eRightAlt;
        case 0x3E: return KeyCode::eRightControl;
        case 0x36: return KeyCode::eRightSuper;
        case 0x6E: return KeyCode::eMenu;
        // Function keys
        case 0x7A: return KeyCode::eF1;
        case 0x78: return KeyCode::eF2;
        case 0x63: return KeyCode::eF3;
        case 0x76: return KeyCode::eF4;
        case 0x60: return KeyCode::eF5;
        case 0x61: return KeyCode::eF6;
        case 0x62: return KeyCode::eF7;
        case 0x64: return KeyCode::eF8;
        case 0x65: return KeyCode::eF9;
        case 0x6D: return KeyCode::eF10;
        case 0x67: return KeyCode::eF11;
        case 0x6F: return KeyCode::eF12;
        case 0x69: return KeyCode::eF13;
        case 0x6B: return KeyCode::eF14;
        case 0x71: return KeyCode::eF15;
        case 0x6A: return KeyCode::eF16;
        case 0x40: return KeyCode::eF17;
        case 0x4F: return KeyCode::eF18;
        case 0x50: return KeyCode::eF19;
        case 0x5A: return KeyCode::eF20;
        // Navigation
        case 0x7B: return KeyCode::eLeft;
        case 0x7C: return KeyCode::eRight;
        case 0x7D: return KeyCode::eDown;
        case 0x7E: return KeyCode::eUp;
        case 0x74: return KeyCode::ePageUp;
        case 0x79: return KeyCode::ePageDown;
        case 0x73: return KeyCode::eHome;
        case 0x77: return KeyCode::eEnd;
        case 0x72: return KeyCode::eInsert;
        case 0x75: return KeyCode::eDelete;
        // Keypad
        case 0x52: return KeyCode::eKp0;
        case 0x53: return KeyCode::eKp1;
        case 0x54: return KeyCode::eKp2;
        case 0x55: return KeyCode::eKp3;
        case 0x56: return KeyCode::eKp4;
        case 0x57: return KeyCode::eKp5;
        case 0x58: return KeyCode::eKp6;
        case 0x59: return KeyCode::eKp7;
        case 0x5B: return KeyCode::eKp8;
        case 0x5C: return KeyCode::eKp9;
        case 0x45: return KeyCode::eKpAdd;
        case 0x4E: return KeyCode::eKpSubtract;
        case 0x43: return KeyCode::eKpMultiply;
        case 0x4B: return KeyCode::eKpDivide;
        case 0x41: return KeyCode::eKpDecimal;
        case 0x4C: return KeyCode::eKpEnter;
        case 0x51: return KeyCode::eKpEqual;
        case 0x47: return KeyCode::eNumLock;
        default:   return KeyCode::eUnknown;
    }
}
// clang-format on

uint64_t mapEventsKeyCodeToCocoaPacked(KeyCode target) {
    if (target == KeyCode::eUnknown) {
        return 0;
    }
    for (unsigned n = 0; n < 256; ++n) {
        if (mapCocoaKeyCode(static_cast<uint16_t>(n)) == target) {
            return static_cast<uint64_t>(n);
        }
    }
    return 0;
}

unsigned long mapEventsModifiersToCocoaFlags(uint8_t events_modifiers) {
    unsigned long f = 0;
    if ((events_modifiers & static_cast<uint8_t>(ModifierKey::eModShift)) != 0) {
        f |= (1UL << 17);
    }
    if ((events_modifiers & static_cast<uint8_t>(ModifierKey::eModCtrl)) != 0) {
        f |= (1UL << 18);
    }
    if ((events_modifiers & static_cast<uint8_t>(ModifierKey::eModAlt)) != 0) {
        f |= (1UL << 19);
    }
    if (((events_modifiers & static_cast<uint8_t>(ModifierKey::eModMeta)) != 0)
        || ((events_modifiers & static_cast<uint8_t>(ModifierKey::eModCmd)) != 0)) {
        f |= (1UL << 20);
    }
    if ((events_modifiers & static_cast<uint8_t>(ModifierKey::eModSuper)) != 0) {
        f |= (1UL << 20);
    }
    return f;
}

uint8_t mapCocoaModifiers(unsigned long flags) {
    uint8_t mods = 0;
    // NSEventModifierFlagShift = 1 << 17
    if ((flags & (1UL << 17)) != 0UL) {
        mods |= static_cast<uint8_t>(ModifierKey::eModShift);
    }
    // NSEventModifierFlagControl = 1 << 18
    if ((flags & (1UL << 18)) != 0UL) {
        mods |= static_cast<uint8_t>(ModifierKey::eModCtrl);
    }
    // NSEventModifierFlagOption (Alt) = 1 << 19
    if ((flags & (1UL << 19)) != 0UL) {
        mods |= static_cast<uint8_t>(ModifierKey::eModAlt);
    }
    // NSEventModifierFlagCommand = 1 << 20 - round-trip with mapEventsModifiersToCocoaFlags
    // which folds eModMeta, eModCmd, and eModSuper into this single Cocoa flag.
    if ((flags & (1UL << 20)) != 0UL) {
        mods |= static_cast<uint8_t>(ModifierKey::eModCmd);
        mods |= static_cast<uint8_t>(ModifierKey::eModMeta);
        mods |= static_cast<uint8_t>(ModifierKey::eModSuper);
    }
    return mods;
}

}  // namespace vne::xwin
