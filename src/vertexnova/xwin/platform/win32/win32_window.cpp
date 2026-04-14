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

#include "win32_window.h"

#include "win32_window_manager.h"
#include "xwin_map_key.h"
#include "xwin_vne_events_bridge.h"

#include <vertexnova/events/types.h>

#include <windowsx.h>

namespace vne::xwin {

namespace {

constexpr wchar_t kClassName[] = L"VneXWinWnd";

std::wstring Utf8ToWide(const std::string& utf8) {
    if (utf8.empty()) {
        return std::wstring();
    }
    int need = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), static_cast<int>(utf8.size()), nullptr, 0);
    if (need <= 0) {
        return std::wstring();
    }
    std::wstring out(static_cast<size_t>(need), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), static_cast<int>(utf8.size()), out.data(), need);
    return out;
}

}  // namespace

Win32Window_C::Win32Window_C() = default;

Win32Window_C::~Win32Window_C() {
    destroy_window();
}

void Win32Window_C::SetEventOwner(Win32WindowManager_C* owner) {
    _event_owner = owner;
}

void Win32Window_C::create_window(const WindowDescriptor_C& descriptor) {
    _desc = descriptor;
    HINSTANCE hinst = GetModuleHandleW(nullptr);

    WNDCLASSEXW info{};
    if (GetClassInfoExW(hinst, kClassName, &info) == 0) {
        WNDCLASSEXW wc{};
        wc.cbSize = sizeof(wc);
        wc.style = CS_OWNDC;
        wc.lpfnWndProc = &Win32Window_C::StaticWndProc;
        wc.hInstance = hinst;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.lpszClassName = kClassName;
        if (RegisterClassExW(&wc) == 0) {
            return;
        }
    }

    RECT r{0, 0, static_cast<LONG>(_desc.size.width), static_cast<LONG>(_desc.size.height)};
    DWORD style = WS_OVERLAPPEDWINDOW;
    if (!_desc.decorated) {
        style = WS_POPUP;
    }
    AdjustWindowRect(&r, style, FALSE);

    const std::wstring title = Utf8ToWide(_desc.title);
    _hwnd = CreateWindowExW(0,
                            kClassName,
                            title.c_str(),
                            style,
                            _desc.position.x,
                            _desc.position.y,
                            r.right - r.left,
                            r.bottom - r.top,
                            nullptr,
                            nullptr,
                            hinst,
                            this);
    if (_hwnd) {
        _open = true;
        ShowWindow(_hwnd, _desc.visible ? SW_SHOW : SW_HIDE);
        UpdateWindow(_hwnd);
    }
}

void Win32Window_C::destroy_window() {
    if (_hwnd) {
        DestroyWindow(_hwnd);
        _hwnd = nullptr;
    }
    _open = false;
}

LRESULT CALLBACK Win32Window_C::StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    Win32Window_C* self = nullptr;
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = static_cast<Win32Window_C*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        self->_hwnd = hwnd;
    } else {
        self = reinterpret_cast<Win32Window_C*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }
    if (!self) {
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    const LRESULT out = self->HandleMessage(hwnd, msg, wParam, lParam);
    if (msg == WM_NCDESTROY) {
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
        self->_hwnd = nullptr;
    }
    return out;
}

LRESULT Win32Window_C::HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    const XWinVneEventCallbacks_C empty_callbacks{};
    const XWinVneEventCallbacks_C& cb = _event_owner ? _event_owner->vneEventCallbacks() : empty_callbacks;

    switch (msg) {
        case WM_CLOSE:
            xwinVneBridgeWindowClose(this, _desc, cb);
            _open = false;
            if (_event_owner) {
                WindowEventData_C ev{};
                ev.type = WindowEventType_TP::CLOSE;
                _event_owner->NotifyWindowEvent(this, ev);
            }
            DestroyWindow(hwnd);
            return 0;
        case WM_SIZE: {
            if (wParam != SIZE_MINIMIZED) {
                _desc.size.width = static_cast<uint32_t>(LOWORD(lParam));
                _desc.size.height = static_cast<uint32_t>(HIWORD(lParam));
                xwinVneBridgeWindowResize(this, _desc, cb, _desc.size.width, _desc.size.height);
                if (_event_owner) {
                    WindowEventData_C ev{};
                    ev.type = WindowEventType_TP::RESIZE;
                    ev.size = _desc.size;
                    _event_owner->NotifyWindowEvent(this, ev);
                }
            }
            return 0;
        }
        case WM_DESTROY:
            _open = false;
            return 0;
        case WM_SETFOCUS:
            xwinVneBridgeWindowFocus(this, _desc, cb, true);
            if (_event_owner) {
                WindowEventData_C ev{};
                ev.type = WindowEventType_TP::FOCUS;
                ev.focused = true;
                _event_owner->NotifyWindowEvent(this, ev);
            }
            return 0;
        case WM_KILLFOCUS:
            xwinVneBridgeWindowFocus(this, _desc, cb, false);
            if (_event_owner) {
                WindowEventData_C ev{};
                ev.type = WindowEventType_TP::FOCUS;
                ev.focused = false;
                _event_owner->NotifyWindowEvent(this, ev);
            }
            return 0;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN: {
            const bool want_vne = _desc.enable_input || _desc.enable_events || static_cast<bool>(cb.on_key_down);
            if (want_vne) {
                const vne::events::KeyCode kc = xwinMapWin32Key(wParam, lParam);
                if (kc != vne::events::KeyCode::eUnknown) {
                    const std::uint8_t mods = xwinMapWin32ModifierFlags();
                    const bool repeat = (lParam & (1 << 30)) != 0;
                    xwinVneBridgeKeyDown(this, _desc, cb, kc, mods, repeat);
                }
            }
            if (msg == WM_SYSKEYDOWN) {
                return DefWindowProcW(hwnd, msg, wParam, lParam);
            }
            return 0;
        }
        case WM_KEYUP:
        case WM_SYSKEYUP: {
            const bool want_vne = _desc.enable_input || _desc.enable_events || static_cast<bool>(cb.on_key_up);
            if (want_vne) {
                const vne::events::KeyCode kc = xwinMapWin32Key(wParam, lParam);
                if (kc != vne::events::KeyCode::eUnknown) {
                    const std::uint8_t mods = xwinMapWin32ModifierFlags();
                    xwinVneBridgeKeyUp(this, _desc, cb, kc, mods);
                }
            }
            if (msg == WM_SYSKEYUP) {
                return DefWindowProcW(hwnd, msg, wParam, lParam);
            }
            return 0;
        }
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_XBUTTONDOWN:
        case WM_XBUTTONUP: {
            const bool want_vne = _desc.enable_input || _desc.enable_events || static_cast<bool>(cb.on_mouse_button);
            if (want_vne) {
                const int x = GET_X_LPARAM(lParam);
                const int y = GET_Y_LPARAM(lParam);
                const std::uint8_t mods = xwinMapWin32ModifierFlags();
                const vne::events::MouseButton btn = xwinMapWin32MouseButtonFromMessage(msg, wParam);
                const bool down =
                    (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN || msg == WM_XBUTTONDOWN);
                xwinVneBridgeMouseButton(this,
                                         _desc,
                                         cb,
                                         btn,
                                         down,
                                         static_cast<double>(x),
                                         static_cast<double>(y),
                                         mods);
            }
            return 0;
        }
        case WM_MOUSEMOVE: {
            const bool want_vne = _desc.enable_input || _desc.enable_events || static_cast<bool>(cb.on_mouse_move);
            if (want_vne) {
                const int x = GET_X_LPARAM(lParam);
                const int y = GET_Y_LPARAM(lParam);
                const std::uint8_t mods = xwinMapWin32ModifierFlags();
                xwinVneBridgeMouseMove(this, _desc, cb, static_cast<double>(x), static_cast<double>(y), mods);
            }
            return 0;
        }
        case WM_MOUSEWHEEL: {
            const bool want_vne = _desc.enable_input || _desc.enable_events || static_cast<bool>(cb.on_mouse_scroll);
            if (want_vne) {
                const short delta = GET_WHEEL_DELTA_WPARAM(wParam);
                const float step = static_cast<float>(delta) / static_cast<float>(WHEEL_DELTA);
                xwinVneBridgeMouseScroll(this, _desc, cb, 0.0F, step);
            }
            return 0;
        }
        case WM_MOUSEHWHEEL: {
            const bool want_vne = _desc.enable_input || _desc.enable_events || static_cast<bool>(cb.on_mouse_scroll);
            if (want_vne) {
                const short delta = GET_WHEEL_DELTA_WPARAM(wParam);
                const float step = static_cast<float>(delta) / static_cast<float>(WHEEL_DELTA);
                xwinVneBridgeMouseScroll(this, _desc, cb, step, 0.0F);
            }
            return 0;
        }
        default:
            return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}

void Win32Window_C::Initialize(const WindowDescriptor_C& descriptor) {
    destroy_window();
    create_window(descriptor);
}

void Win32Window_C::PollEvents() {
    if (!_hwnd) {
        return;
    }
    MSG msg{};
    while (PeekMessageW(&msg, _hwnd, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

void Win32Window_C::SwapBuffers() {}

void Win32Window_C::SetTitle(const std::string& title) {
    _desc.title = title;
    if (_hwnd) {
        SetWindowTextW(_hwnd, Utf8ToWide(title).c_str());
    }
}

void Win32Window_C::SetWindowMode(WindowMode_TP mode) {
    _mode = mode;
}

WindowMode_TP Win32Window_C::GetWindowMode() const {
    return _mode;
}

void Win32Window_C::SetFullscreen(bool enabled) {
    (void)enabled;
}

bool Win32Window_C::IsFullscreen() const {
    return false;
}

void Win32Window_C::SetPosition(int x, int y) {
    _desc.position.x = x;
    _desc.position.y = y;
    if (_hwnd) {
        SetWindowPos(_hwnd, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }
}

void Win32Window_C::GetPosition(int& x, int& y) const {
    x = _desc.position.x;
    y = _desc.position.y;
    if (_hwnd) {
        RECT r{};
        if (GetWindowRect(_hwnd, &r)) {
            x = r.left;
            y = r.top;
        }
    }
}

void Win32Window_C::Resize(uint32_t width, uint32_t height) {
    _desc.size.width = width;
    _desc.size.height = height;
    if (_hwnd) {
        SetWindowPos(_hwnd,
                     nullptr,
                     0,
                     0,
                     static_cast<int>(width),
                     static_cast<int>(height),
                     SWP_NOMOVE | SWP_NOZORDER);
    }
}

void Win32Window_C::Close() {
    if (_hwnd) {
        DestroyWindow(_hwnd);
        _hwnd = nullptr;
    }
    _open = false;
}

bool Win32Window_C::IsOpen() const {
    return _open && _hwnd != nullptr;
}

void* Win32Window_C::GetNativeWindow() const {
    return _hwnd;
}

WindowAPI_TP Win32Window_C::GetWindowAPI() const {
    return WindowAPI_TP::WIN32_WINDOW;
}

int Win32Window_C::GetWidth() const {
    return static_cast<int>(_desc.size.width);
}

int Win32Window_C::GetHeight() const {
    return static_cast<int>(_desc.size.height);
}

float Win32Window_C::GetDPIScale() const {
    if (!_hwnd) {
        return 1.0F;
    }
    using GetDpiForWindowFn = UINT(WINAPI*)(HWND);
    static auto fn =
        reinterpret_cast<GetDpiForWindowFn>(GetProcAddress(GetModuleHandleW(L"user32.dll"), "GetDpiForWindow"));
    if (fn) {
        return static_cast<float>(fn(_hwnd)) / 96.0F;
    }
    return 1.0F;
}

}  // namespace vne::xwin
