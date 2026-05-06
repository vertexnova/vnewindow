/* Implementation of X11 KeySym and modifier mapping to vne::events types. */
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

#include "xwin_map_key.h"

#include <X11/keysym.h>
#include <limits>

namespace vne::xwin {
namespace {
constexpr std::uint64_t kPrintableAsciiFirst = 32U;
constexpr std::uint64_t kPrintableAsciiLast = 126U;
}  // namespace

using vne::events::KeyCode;
using vne::events::ModifierKey;

std::uint8_t mapX11Modifiers(unsigned int state) {
    std::uint8_t m = 0;
    if ((state & ShiftMask) != 0U) {
        m |= static_cast<std::uint8_t>(ModifierKey::eModShift);
    }
    if ((state & ControlMask) != 0U) {
        m |= static_cast<std::uint8_t>(ModifierKey::eModCtrl);
    }
    if ((state & Mod1Mask) != 0U) {
        m |= static_cast<std::uint8_t>(ModifierKey::eModAlt);
    }
    if ((state & Mod4Mask) != 0U) {
        m |= static_cast<std::uint8_t>(ModifierKey::eModSuper);
    }
    return m;
}

KeyCode mapX11Keysym(KeySym sym) {
    if (sym >= XK_a && sym <= XK_z) {
        return static_cast<KeyCode>(static_cast<int>(KeyCode::eA) + static_cast<int>(sym - XK_a));
    }
    if (sym >= XK_A && sym <= XK_Z) {
        return static_cast<KeyCode>(static_cast<int>(KeyCode::eA) + static_cast<int>(sym - XK_A));
    }
    if (sym >= XK_0 && sym <= XK_9) {
        return static_cast<KeyCode>(static_cast<int>(KeyCode::e0) + static_cast<int>(sym - XK_0));
    }

    switch (sym) {
        case XK_space:
            return KeyCode::eSpace;
        case XK_apostrophe:
            return KeyCode::eApostrophe;
        case XK_comma:
            return KeyCode::eComma;
        case XK_minus:
            return KeyCode::eMinus;
        case XK_period:
            return KeyCode::ePeriod;
        case XK_slash:
            return KeyCode::eSlash;
        case XK_semicolon:
            return KeyCode::eSemicolon;
        case XK_equal:
            return KeyCode::eEqual;
        case XK_bracketleft:
            return KeyCode::eLeftBracket;
        case XK_backslash:
            return KeyCode::eBackslash;
        case XK_bracketright:
            return KeyCode::eRightBracket;
        case XK_grave:
            return KeyCode::eGraveAccent;
        case XK_BackSpace:
            return KeyCode::eBackspace;
        case XK_Tab:
            return KeyCode::eTab;
        case XK_Return:
        case XK_Linefeed:
            return KeyCode::eEnter;
        case XK_Pause:
            return KeyCode::ePause;
        case XK_Scroll_Lock:
            return KeyCode::eScrollLock;
        case XK_Escape:
            return KeyCode::eEscape;
        case XK_Delete:
            return KeyCode::eDelete;
        case XK_Insert:
            return KeyCode::eInsert;
        case XK_Home:
            return KeyCode::eHome;
        case XK_End:
            return KeyCode::eEnd;
        case XK_Page_Up:
            return KeyCode::ePageUp;
        case XK_Page_Down:
            return KeyCode::ePageDown;
        case XK_Left:
            return KeyCode::eLeft;
        case XK_Up:
            return KeyCode::eUp;
        case XK_Right:
            return KeyCode::eRight;
        case XK_Down:
            return KeyCode::eDown;
        case XK_Num_Lock:
            return KeyCode::eNumLock;
        case XK_Print:
        case XK_Sys_Req:
            return KeyCode::ePrintScreen;
        case XK_Caps_Lock:
            return KeyCode::eCapsLock;
        case XK_Shift_L:
            return KeyCode::eLeftShift;
        case XK_Shift_R:
            return KeyCode::eRightShift;
        case XK_Control_L:
            return KeyCode::eLeftControl;
        case XK_Control_R:
            return KeyCode::eRightControl;
        case XK_Alt_L:
        case XK_Meta_L:
            return KeyCode::eLeftAlt;
        case XK_Alt_R:
        case XK_Meta_R:
            return KeyCode::eRightAlt;
        case XK_Super_L:
        case XK_Hyper_L:
            return KeyCode::eLeftSuper;
        case XK_Super_R:
        case XK_Hyper_R:
            return KeyCode::eRightSuper;
        case XK_Menu:
            return KeyCode::eMenu;
        case XK_KP_0:
            return KeyCode::eKp0;
        case XK_KP_1:
            return KeyCode::eKp1;
        case XK_KP_2:
            return KeyCode::eKp2;
        case XK_KP_3:
            return KeyCode::eKp3;
        case XK_KP_4:
            return KeyCode::eKp4;
        case XK_KP_5:
            return KeyCode::eKp5;
        case XK_KP_6:
            return KeyCode::eKp6;
        case XK_KP_7:
            return KeyCode::eKp7;
        case XK_KP_8:
            return KeyCode::eKp8;
        case XK_KP_9:
            return KeyCode::eKp9;
        case XK_KP_Decimal:
            return KeyCode::eKpDecimal;
        case XK_KP_Divide:
            return KeyCode::eKpDivide;
        case XK_KP_Multiply:
            return KeyCode::eKpMultiply;
        case XK_KP_Subtract:
            return KeyCode::eKpSubtract;
        case XK_KP_Add:
            return KeyCode::eKpAdd;
        case XK_KP_Enter:
            return KeyCode::eKpEnter;
        case XK_KP_Equal:
            return KeyCode::eKpEqual;
        default:
            break;
    }

    if (sym >= XK_F1 && sym <= XK_F24) {
        return static_cast<KeyCode>(static_cast<int>(KeyCode::eF1) + static_cast<int>(sym - XK_F1));
    }

    if (sym >= static_cast<KeySym>(kPrintableAsciiFirst) && sym <= static_cast<KeySym>(kPrintableAsciiLast)) {
        return static_cast<KeyCode>(static_cast<std::int16_t>(sym));
    }

    return KeyCode::eUnknown;
}

std::uint64_t mapEventsKeyCodeToX11Keysym(KeyCode target) {
    if (target == KeyCode::eUnknown) {
        return 0;
    }
    for (std::uint64_t sym = kPrintableAsciiFirst; sym <= static_cast<std::uint64_t>(std::numeric_limits<std::uint16_t>::max()); ++sym) {
        if (mapX11Keysym(static_cast<KeySym>(sym)) == target) {
            return sym;
        }
    }
    return 0;
}

}  // namespace vne::xwin
