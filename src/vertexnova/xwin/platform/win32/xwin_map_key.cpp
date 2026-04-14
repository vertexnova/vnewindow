/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 * ----------------------------------------------------------------------
 */

#include "xwin_map_key.h"

#include <windowsx.h>

namespace vne::xwin {

using vne::events::KeyCode;
using vne::events::ModifierKey;
using vne::events::MouseButton;

std::uint8_t xwinMapWin32ModifierFlags() {
    std::uint8_t m = 0;
    if ((GetKeyState(VK_SHIFT) & 0x8000) != 0) {
        m |= static_cast<std::uint8_t>(ModifierKey::eModShift);
    }
    if ((GetKeyState(VK_CONTROL) & 0x8000) != 0) {
        m |= static_cast<std::uint8_t>(ModifierKey::eModCtrl);
    }
    if ((GetKeyState(VK_MENU) & 0x8000) != 0) {
        m |= static_cast<std::uint8_t>(ModifierKey::eModAlt);
    }
    if (((GetKeyState(VK_LWIN) & 0x8000) != 0) || ((GetKeyState(VK_RWIN) & 0x8000) != 0)) {
        m |= static_cast<std::uint8_t>(ModifierKey::eModSuper);
    }
    return m;
}

KeyCode xwinMapWin32Key(WPARAM vk, LPARAM lParam) {
    const std::uint32_t scan = (static_cast<std::uint32_t>(lParam) >> 16U) & 0xFFU;
    const bool ext = (lParam & (1 << 24)) != 0;

    switch (vk) {
        case VK_SHIFT:
            return (scan == 0x36U) ? KeyCode::eRightShift : KeyCode::eLeftShift;
        case VK_CONTROL:
            return ext ? KeyCode::eRightControl : KeyCode::eLeftControl;
        case VK_MENU:
            return ext ? KeyCode::eRightAlt : KeyCode::eLeftAlt;
        case VK_SPACE:
            return KeyCode::eSpace;
        case VK_OEM_7:  // typical US quote
            return KeyCode::eApostrophe;
        case VK_OEM_COMMA:
            return KeyCode::eComma;
        case VK_OEM_MINUS:
            return KeyCode::eMinus;
        case VK_OEM_PERIOD:
            return KeyCode::ePeriod;
        case VK_OEM_2:  // /
            return KeyCode::eSlash;
        case VK_OEM_1:  // ;
            return KeyCode::eSemicolon;
        case VK_OEM_PLUS:
            return KeyCode::eEqual;
        case VK_OEM_4:  // [
            return KeyCode::eLeftBracket;
        case VK_OEM_5:  // backslash
            return KeyCode::eBackslash;
        case VK_OEM_6:  // ]
            return KeyCode::eRightBracket;
        case VK_OEM_3:  // `
            return KeyCode::eGraveAccent;
        case VK_BACK:
            return KeyCode::eBackspace;
        case VK_TAB:
            return KeyCode::eTab;
        case VK_CLEAR:
            return KeyCode::eUnknown;
        case VK_RETURN:
            return KeyCode::eEnter;
        case VK_PAUSE:
            return KeyCode::ePause;
        case VK_CAPITAL:
            return KeyCode::eCapsLock;
        case VK_ESCAPE:
            return KeyCode::eEscape;
        case VK_CONVERT:
        case VK_NONCONVERT:
        case VK_ACCEPT:
        case VK_MODECHANGE:
            return KeyCode::eUnknown;
        case VK_PRIOR:
            return KeyCode::ePageUp;
        case VK_NEXT:
            return KeyCode::ePageDown;
        case VK_END:
            return KeyCode::eEnd;
        case VK_HOME:
            return KeyCode::eHome;
        case VK_LEFT:
            return KeyCode::eLeft;
        case VK_UP:
            return KeyCode::eUp;
        case VK_RIGHT:
            return KeyCode::eRight;
        case VK_DOWN:
            return KeyCode::eDown;
        case VK_SELECT:
            return KeyCode::eUnknown;
        case VK_EXECUTE:
            return KeyCode::eUnknown;
        case VK_SNAPSHOT:
            return KeyCode::ePrintScreen;
        case VK_INSERT:
            return KeyCode::eInsert;
        case VK_DELETE:
            return KeyCode::eDelete;
        case VK_HELP:
            return KeyCode::eUnknown;
        case VK_SCROLL:
            return KeyCode::eScrollLock;
        case VK_NUMLOCK:
            return KeyCode::eNumLock;
        case VK_APPS:
            return KeyCode::eMenu;
        case VK_SLEEP:
            return KeyCode::eUnknown;
        case VK_NUMPAD0:
            return KeyCode::eKp0;
        case VK_NUMPAD1:
            return KeyCode::eKp1;
        case VK_NUMPAD2:
            return KeyCode::eKp2;
        case VK_NUMPAD3:
            return KeyCode::eKp3;
        case VK_NUMPAD4:
            return KeyCode::eKp4;
        case VK_NUMPAD5:
            return KeyCode::eKp5;
        case VK_NUMPAD6:
            return KeyCode::eKp6;
        case VK_NUMPAD7:
            return KeyCode::eKp7;
        case VK_NUMPAD8:
            return KeyCode::eKp8;
        case VK_NUMPAD9:
            return KeyCode::eKp9;
        case VK_MULTIPLY:
            return KeyCode::eKpMultiply;
        case VK_ADD:
            return KeyCode::eKpAdd;
        case VK_SEPARATOR:
            return KeyCode::eKpEnter;
        case VK_SUBTRACT:
            return KeyCode::eKpSubtract;
        case VK_DECIMAL:
            return KeyCode::eKpDecimal;
        case VK_DIVIDE:
            return KeyCode::eKpDivide;
        case VK_LWIN:
            return KeyCode::eLeftSuper;
        case VK_RWIN:
            return KeyCode::eRightSuper;
        case VK_LSHIFT:
            return KeyCode::eLeftShift;
        case VK_RSHIFT:
            return KeyCode::eRightShift;
        case VK_LCONTROL:
            return KeyCode::eLeftControl;
        case VK_RCONTROL:
            return KeyCode::eRightControl;
        case VK_LMENU:
            return KeyCode::eLeftAlt;
        case VK_RMENU:
            return KeyCode::eRightAlt;
        default:
            break;
    }

    if (vk >= static_cast<WPARAM>('0') && vk <= static_cast<WPARAM>('9')) {
        return static_cast<KeyCode>(static_cast<int>(KeyCode::e0) + static_cast<int>(vk - '0'));
    }
    if (vk >= static_cast<WPARAM>('A') && vk <= static_cast<WPARAM>('Z')) {
        return static_cast<KeyCode>(static_cast<int>(KeyCode::eA) + static_cast<int>(vk - 'A'));
    }

    if (vk >= VK_F1 && vk <= VK_F24) {
        return static_cast<KeyCode>(static_cast<int>(KeyCode::eF1) + static_cast<int>(vk - VK_F1));
    }

    return KeyCode::eUnknown;
}

MouseButton xwinMapWin32MouseButtonFromMessage(UINT msg, WPARAM wParam) {
    switch (msg) {
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_LBUTTONDBLCLK:
            return MouseButton::eLeft;
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_RBUTTONDBLCLK:
            return MouseButton::eRight;
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MBUTTONDBLCLK:
            return MouseButton::eMiddle;
        case WM_XBUTTONDOWN:
        case WM_XBUTTONUP:
        case WM_XBUTTONDBLCLK: {
            const WORD x = GET_XBUTTON_WPARAM(wParam);
            if (x == XBUTTON1) {
                return MouseButton::eButton3;
            }
            if (x == XBUTTON2) {
                return MouseButton::eButton4;
            }
            return MouseButton::eLeft;
        }
        default:
            return MouseButton::eLeft;
    }
}

}  // namespace vne::xwin
