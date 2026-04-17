/* ---------------------------------------------------------------------
 * Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License")
 *
 * Author:    Ajeet Singh Yadav
 * Created:   April 2026
 *
 * DOM KeyboardEvent.code → vne::events::KeyCode mapping.
 * Code strings follow the W3C UI Events KeyboardEvent code values spec.
 * ----------------------------------------------------------------------
 */

#include "wasm_map_key.h"

#include <cstring>
#include <unordered_map>
#include <string>

namespace vne::xwin {

using vne::events::KeyCode;
using vne::events::MouseButton;
using vne::events::ModifierKey;

static const std::unordered_map<std::string, KeyCode>& keyMap() {
    // clang-format off
    static const std::unordered_map<std::string, KeyCode> kMap = {
        // Alphanumeric
        {"KeyA", KeyCode::eA}, {"KeyB", KeyCode::eB}, {"KeyC", KeyCode::eC},
        {"KeyD", KeyCode::eD}, {"KeyE", KeyCode::eE}, {"KeyF", KeyCode::eF},
        {"KeyG", KeyCode::eG}, {"KeyH", KeyCode::eH}, {"KeyI", KeyCode::eI},
        {"KeyJ", KeyCode::eJ}, {"KeyK", KeyCode::eK}, {"KeyL", KeyCode::eL},
        {"KeyM", KeyCode::eM}, {"KeyN", KeyCode::eN}, {"KeyO", KeyCode::eO},
        {"KeyP", KeyCode::eP}, {"KeyQ", KeyCode::eQ}, {"KeyR", KeyCode::eR},
        {"KeyS", KeyCode::eS}, {"KeyT", KeyCode::eT}, {"KeyU", KeyCode::eU},
        {"KeyV", KeyCode::eV}, {"KeyW", KeyCode::eW}, {"KeyX", KeyCode::eX},
        {"KeyY", KeyCode::eY}, {"KeyZ", KeyCode::eZ},
        // Digits
        {"Digit0", KeyCode::e0}, {"Digit1", KeyCode::e1}, {"Digit2", KeyCode::e2},
        {"Digit3", KeyCode::e3}, {"Digit4", KeyCode::e4}, {"Digit5", KeyCode::e5},
        {"Digit6", KeyCode::e6}, {"Digit7", KeyCode::e7}, {"Digit8", KeyCode::e8},
        {"Digit9", KeyCode::e9},
        // Punctuation
        {"Space",        KeyCode::eSpace},
        {"Quote",        KeyCode::eApostrophe},
        {"Comma",        KeyCode::eComma},
        {"Minus",        KeyCode::eMinus},
        {"Period",       KeyCode::ePeriod},
        {"Slash",        KeyCode::eSlash},
        {"Semicolon",    KeyCode::eSemicolon},
        {"Equal",        KeyCode::eEqual},
        {"BracketLeft",  KeyCode::eLeftBracket},
        {"Backslash",    KeyCode::eBackslash},
        {"BracketRight", KeyCode::eRightBracket},
        {"Backquote",    KeyCode::eGraveAccent},
        // Control
        {"Escape",       KeyCode::eEscape},
        {"Enter",        KeyCode::eEnter},
        {"NumpadEnter",  KeyCode::eKpEnter},
        {"Tab",          KeyCode::eTab},
        {"Backspace",    KeyCode::eBackspace},
        {"Insert",       KeyCode::eInsert},
        {"Delete",       KeyCode::eDelete},
        {"CapsLock",     KeyCode::eCapsLock},
        {"ScrollLock",   KeyCode::eScrollLock},
        {"NumLock",      KeyCode::eNumLock},
        {"PrintScreen",  KeyCode::ePrintScreen},
        {"Pause",        KeyCode::ePause},
        // Navigation
        {"ArrowRight", KeyCode::eRight},
        {"ArrowLeft",  KeyCode::eLeft},
        {"ArrowDown",  KeyCode::eDown},
        {"ArrowUp",    KeyCode::eUp},
        {"PageUp",     KeyCode::ePageUp},
        {"PageDown",   KeyCode::ePageDown},
        {"Home",       KeyCode::eHome},
        {"End",        KeyCode::eEnd},
        // Function keys
        {"F1",  KeyCode::eF1},  {"F2",  KeyCode::eF2},  {"F3",  KeyCode::eF3},
        {"F4",  KeyCode::eF4},  {"F5",  KeyCode::eF5},  {"F6",  KeyCode::eF6},
        {"F7",  KeyCode::eF7},  {"F8",  KeyCode::eF8},  {"F9",  KeyCode::eF9},
        {"F10", KeyCode::eF10}, {"F11", KeyCode::eF11}, {"F12", KeyCode::eF12},
        {"F13", KeyCode::eF13}, {"F14", KeyCode::eF14}, {"F15", KeyCode::eF15},
        {"F16", KeyCode::eF16}, {"F17", KeyCode::eF17}, {"F18", KeyCode::eF18},
        {"F19", KeyCode::eF19}, {"F20", KeyCode::eF20},
        // Modifier keys
        {"ShiftLeft",    KeyCode::eLeftShift},
        {"ShiftRight",   KeyCode::eRightShift},
        {"ControlLeft",  KeyCode::eLeftControl},
        {"ControlRight", KeyCode::eRightControl},
        {"AltLeft",      KeyCode::eLeftAlt},
        {"AltRight",     KeyCode::eRightAlt},
        {"MetaLeft",     KeyCode::eLeftSuper},
        {"MetaRight",    KeyCode::eRightSuper},
        {"ContextMenu",  KeyCode::eMenu},
        // Numpad
        {"Numpad0", KeyCode::eKp0}, {"Numpad1", KeyCode::eKp1},
        {"Numpad2", KeyCode::eKp2}, {"Numpad3", KeyCode::eKp3},
        {"Numpad4", KeyCode::eKp4}, {"Numpad5", KeyCode::eKp5},
        {"Numpad6", KeyCode::eKp6}, {"Numpad7", KeyCode::eKp7},
        {"Numpad8", KeyCode::eKp8}, {"Numpad9", KeyCode::eKp9},
        {"NumpadDecimal",  KeyCode::eKpDecimal},
        {"NumpadDivide",   KeyCode::eKpDivide},
        {"NumpadMultiply", KeyCode::eKpMultiply},
        {"NumpadSubtract", KeyCode::eKpSubtract},
        {"NumpadAdd",      KeyCode::eKpAdd},
        {"NumpadEqual",    KeyCode::eKpEqual},
    };
    // clang-format on
    return kMap;
}

KeyCode mapEmscriptenKey(const char* code) {
    if (!code) { return KeyCode::eUnknown; }
    const auto& m = keyMap();
    const auto it = m.find(code);
    return (it != m.end()) ? it->second : KeyCode::eUnknown;
}

MouseButton mapEmscriptenMouseButton(unsigned short button) {
    switch (button) {
        case 0: return MouseButton::eLeft;
        case 1: return MouseButton::eMiddle;
        case 2: return MouseButton::eRight;
        case 3: return MouseButton::eButton3;
        case 4: return MouseButton::eButton4;
        default: return MouseButton::eLeft;
    }
}

uint8_t mapEmscriptenModifiers(bool shift, bool ctrl, bool alt, bool meta) {
    uint8_t mods = 0;
    if (shift) { mods |= static_cast<uint8_t>(ModifierKey::eModShift); }
    if (ctrl)  { mods |= static_cast<uint8_t>(ModifierKey::eModCtrl);  }
    if (alt)   { mods |= static_cast<uint8_t>(ModifierKey::eModAlt);   }
    if (meta)  { mods |= static_cast<uint8_t>(ModifierKey::eModMeta);  }
    return mods;
}

}  // namespace vne::xwin
