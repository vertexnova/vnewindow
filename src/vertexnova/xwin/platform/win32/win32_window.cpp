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
#include "win32_map_key.h"
#include "event_bridge.h"

#include <vertexnova/xwin/input_mapping.h>

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
    event_owner_ = owner;
}

void Win32Window_C::create_window(const WindowDescriptor& descriptor) {
    desc_ = descriptor;
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

    RECT r{0, 0, static_cast<LONG>(desc_.size.width), static_cast<LONG>(desc_.size.height)};
    DWORD style = WS_OVERLAPPEDWINDOW;
    if (!desc_.decorated) {
        style = WS_POPUP;
    }
    AdjustWindowRect(&r, style, FALSE);

    const std::wstring title = Utf8ToWide(desc_.title);
    hwnd_ = CreateWindowExW(0,
                            kClassName,
                            title.c_str(),
                            style,
                            desc_.position.x,
                            desc_.position.y,
                            r.right - r.left,
                            r.bottom - r.top,
                            nullptr,
                            nullptr,
                            hinst,
                            this);
    if (hwnd_) {
        open_ = true;
        ShowWindow(hwnd_, desc_.visible ? SW_SHOW : SW_HIDE);
        UpdateWindow(hwnd_);
    }
}

void Win32Window_C::destroy_window() {
    if (hwnd_) {
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    }
    open_ = false;
}

LRESULT CALLBACK Win32Window_C::StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    Win32Window_C* self = nullptr;
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = static_cast<Win32Window_C*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        self->hwnd_ = hwnd;
    } else {
        self = reinterpret_cast<Win32Window_C*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }
    if (!self) {
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    const LRESULT out = self->HandleMessage(hwnd, msg, wParam, lParam);
    if (msg == WM_NCDESTROY) {
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
        self->hwnd_ = nullptr;
    }
    return out;
}

LRESULT Win32Window_C::HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    const EventBridgeCallbacks empty_callbacks{};
    const EventBridgeCallbacks& cb = event_owner_ ? event_owner_->eventBridgeCallbacks() : empty_callbacks;

    switch (msg) {
        case WM_CLOSE:
            eventBridgeWindowClose(this, desc_, cb);
            open_ = false;
            if (event_owner_) {
                WindowEventData ev{};
                ev.type = WindowEventType::eClose;
                event_owner_->NotifyWindowEvent(this, ev);
            }
            DestroyWindow(hwnd);
            return 0;
        case WM_SIZE: {
            if (wParam != SIZE_MINIMIZED) {
                desc_.size.width = static_cast<uint32_t>(LOWORD(lParam));
                desc_.size.height = static_cast<uint32_t>(HIWORD(lParam));
                eventBridgeWindowResize(this, desc_, cb, desc_.size.width, desc_.size.height);
                if (event_owner_) {
                    WindowEventData ev{};
                    ev.type = WindowEventType::eResize;
                    ev.size = desc_.size;
                    event_owner_->NotifyWindowEvent(this, ev);
                }
            }
            return 0;
        }
        case WM_DESTROY:
            open_ = false;
            return 0;
        case WM_SETFOCUS:
            eventBridgeWindowFocus(this, desc_, cb, true);
            if (event_owner_) {
                WindowEventData ev{};
                ev.type = WindowEventType::eFocus;
                ev.focused = true;
                event_owner_->NotifyWindowEvent(this, ev);
            }
            return 0;
        case WM_KILLFOCUS:
            eventBridgeWindowFocus(this, desc_, cb, false);
            if (event_owner_) {
                WindowEventData ev{};
                ev.type = WindowEventType::eFocus;
                ev.focused = false;
                event_owner_->NotifyWindowEvent(this, ev);
            }
            return 0;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN: {
            const bool want_vne = desc_.enable_input || desc_.enable_events || static_cast<bool>(cb.onKeyDown);
            if (want_vne) {
                const vne::events::KeyCode kc = mapNativeKeyToEvents(WindowAPI::eWin32Window,
                                                                     packWin32NativeKey(wParam, lParam),
                                                                     desc_.input_mapping);
                if (kc != vne::events::KeyCode::eUnknown) {
                    const std::uint8_t mods =
                        mapNativeModifiersToEvents(WindowAPI::eWin32Window,
                                                   static_cast<std::uint64_t>(mapWin32ModifierFlags()),
                                                   desc_.input_mapping);
                    const bool repeat = (lParam & (1 << 30)) != 0;
                    eventBridgeKeyDown(this, desc_, cb, kc, mods, repeat);
                }
            }
            if (msg == WM_SYSKEYDOWN) {
                return DefWindowProcW(hwnd, msg, wParam, lParam);
            }
            return 0;
        }
        case WM_KEYUP:
        case WM_SYSKEYUP: {
            const bool want_vne = desc_.enable_input || desc_.enable_events || static_cast<bool>(cb.onKeyUp);
            if (want_vne) {
                const vne::events::KeyCode kc = mapNativeKeyToEvents(WindowAPI::eWin32Window,
                                                                     packWin32NativeKey(wParam, lParam),
                                                                     desc_.input_mapping);
                if (kc != vne::events::KeyCode::eUnknown) {
                    const std::uint8_t mods =
                        mapNativeModifiersToEvents(WindowAPI::eWin32Window,
                                                   static_cast<std::uint64_t>(mapWin32ModifierFlags()),
                                                   desc_.input_mapping);
                    eventBridgeKeyUp(this, desc_, cb, kc, mods);
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
            const bool want_vne = desc_.enable_input || desc_.enable_events || static_cast<bool>(cb.onMouseButton);
            if (want_vne) {
                const int x = GET_X_LPARAM(lParam);
                const int y = GET_Y_LPARAM(lParam);
                const std::uint8_t mods =
                    mapNativeModifiersToEvents(WindowAPI::eWin32Window,
                                               static_cast<std::uint64_t>(mapWin32ModifierFlags()),
                                               desc_.input_mapping);
                const vne::events::MouseButton btn =
                    mapNativeMouseToEvents(WindowAPI::eWin32Window, packWin32Mouse(msg, wParam), desc_.input_mapping);
                const bool down =
                    (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN || msg == WM_XBUTTONDOWN);
                eventBridgeMouseButton(this,
                                       desc_,
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
            const bool want_vne = desc_.enable_input || desc_.enable_events || static_cast<bool>(cb.onMouseMove);
            if (want_vne) {
                const int x = GET_X_LPARAM(lParam);
                const int y = GET_Y_LPARAM(lParam);
                const std::uint8_t mods =
                    mapNativeModifiersToEvents(WindowAPI::eWin32Window,
                                               static_cast<std::uint64_t>(mapWin32ModifierFlags()),
                                               desc_.input_mapping);
                eventBridgeMouseMove(this, desc_, cb, static_cast<double>(x), static_cast<double>(y), mods);
            }
            return 0;
        }
        case WM_MOUSEWHEEL: {
            const bool want_vne = desc_.enable_input || desc_.enable_events || static_cast<bool>(cb.onMouseScroll);
            if (want_vne) {
                const short delta = GET_WHEEL_DELTA_WPARAM(wParam);
                const float step = static_cast<float>(delta) / static_cast<float>(WHEEL_DELTA);
                eventBridgeMouseScroll(this, desc_, cb, 0.0F, step);
            }
            return 0;
        }
        case WM_MOUSEHWHEEL: {
            const bool want_vne = desc_.enable_input || desc_.enable_events || static_cast<bool>(cb.onMouseScroll);
            if (want_vne) {
                const short delta = GET_WHEEL_DELTA_WPARAM(wParam);
                const float step = static_cast<float>(delta) / static_cast<float>(WHEEL_DELTA);
                eventBridgeMouseScroll(this, desc_, cb, step, 0.0F);
            }
            return 0;
        }
        case WM_CHAR: {
            const bool want_text = desc_.enable_events || static_cast<bool>(cb.onTextInput);
            if (want_text) {
                // wParam is a UTF-16 code unit; handle surrogate pairs
                static wchar_t high_surrogate = 0;
                const wchar_t ch = static_cast<wchar_t>(wParam);
                if (ch >= 0xD800 && ch <= 0xDBFF) {
                    high_surrogate = ch;
                    return 0;
                }
                wchar_t wide[3] = {};
                int wide_len = 0;
                if (high_surrogate && ch >= 0xDC00 && ch <= 0xDFFF) {
                    wide[0] = high_surrogate;
                    wide[1] = ch;
                    wide_len = 2;
                } else {
                    wide[0] = ch;
                    wide_len = 1;
                }
                high_surrogate = 0;
                if (ch >= 0x20 || ch == '\t') {  // skip control chars except tab
                    char utf8[5] = {};
                    const int n = WideCharToMultiByte(CP_UTF8, 0, wide, wide_len, utf8, 4, nullptr, nullptr);
                    if (n > 0) {
                        utf8[n] = '\0';
                        eventBridgeTextInput(this, desc_, cb, utf8);
                    }
                }
            }
            return 0;
        }
        case WM_GETMINMAXINFO: {
            auto* mmi = reinterpret_cast<MINMAXINFO*>(lParam);
            if (desc_.limits.has_min_size) {
                mmi->ptMinTrackSize.x = static_cast<LONG>(desc_.limits.min_size.width);
                mmi->ptMinTrackSize.y = static_cast<LONG>(desc_.limits.min_size.height);
            }
            if (desc_.limits.has_max_size) {
                mmi->ptMaxTrackSize.x = static_cast<LONG>(desc_.limits.max_size.width);
                mmi->ptMaxTrackSize.y = static_cast<LONG>(desc_.limits.max_size.height);
            }
            return 0;
        }
        default:
            return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}

void Win32Window_C::Initialize(const WindowDescriptor& descriptor) {
    destroy_window();
    create_window(descriptor);
}

void Win32Window_C::PollEvents() {
    if (!hwnd_) {
        return;
    }
    MSG msg{};
    while (PeekMessageW(&msg, hwnd_, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

void Win32Window_C::SwapBuffers() {}

void Win32Window_C::SetTitle(const std::string& title) {
    desc_.title = title;
    if (hwnd_) {
        SetWindowTextW(hwnd_, Utf8ToWide(title).c_str());
    }
}

void Win32Window_C::SetWindowMode(WindowMode mode) {
    mode_ = mode;
    if (!hwnd_) {
        return;
    }
    if (mode == WindowMode::eFullscreen) {
        SetFullscreen(true);
        return;
    }
    if (fullscreen_) {
        SetFullscreen(false);
    }
    if (mode == WindowMode::eBorderless) {
        HMONITOR mon = MonitorFromWindow(hwnd_, MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi{};
        mi.cbSize = sizeof(mi);
        GetMonitorInfoW(mon, &mi);
        SetWindowLongW(hwnd_, GWL_STYLE, WS_POPUP | WS_VISIBLE);
        SetWindowPos(hwnd_,
                     HWND_TOP,
                     mi.rcMonitor.left,
                     mi.rcMonitor.top,
                     mi.rcMonitor.right - mi.rcMonitor.left,
                     mi.rcMonitor.bottom - mi.rcMonitor.top,
                     SWP_FRAMECHANGED | SWP_NOACTIVATE);
        return;
    }
    SetWindowLongW(hwnd_, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
    SetWindowPos(hwnd_, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER);
}

WindowMode Win32Window_C::GetWindowMode() const {
    return mode_;
}

void Win32Window_C::SetFullscreen(bool enabled) {
    if (!hwnd_ || enabled == fullscreen_) {
        return;
    }
    if (enabled) {
        // Save current style and rect
        saved_style_ = static_cast<DWORD>(GetWindowLongW(hwnd_, GWL_STYLE));
        GetWindowRect(hwnd_, &saved_rect_);
        // Get monitor covering the window
        HMONITOR mon = MonitorFromWindow(hwnd_, MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi{};
        mi.cbSize = sizeof(mi);
        GetMonitorInfoW(mon, &mi);
        SetWindowLongW(hwnd_, GWL_STYLE, WS_POPUP | WS_VISIBLE);
        SetWindowPos(hwnd_,
                     HWND_TOP,
                     mi.rcMonitor.left,
                     mi.rcMonitor.top,
                     mi.rcMonitor.right - mi.rcMonitor.left,
                     mi.rcMonitor.bottom - mi.rcMonitor.top,
                     SWP_FRAMECHANGED | SWP_NOACTIVATE);
    } else {
        SetWindowLongW(hwnd_, GWL_STYLE, saved_style_);
        SetWindowPos(hwnd_,
                     nullptr,
                     saved_rect_.left,
                     saved_rect_.top,
                     saved_rect_.right - saved_rect_.left,
                     saved_rect_.bottom - saved_rect_.top,
                     SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOZORDER);
        ShowWindow(hwnd_, SW_RESTORE);
    }
    fullscreen_ = enabled;
}

bool Win32Window_C::IsFullscreen() const {
    return fullscreen_;
}

void Win32Window_C::Minimize() {
    if (hwnd_) {
        ShowWindow(hwnd_, SW_MINIMIZE);
    }
}

void Win32Window_C::Maximize() {
    if (hwnd_) {
        ShowWindow(hwnd_, SW_MAXIMIZE);
    }
}

void Win32Window_C::Restore() {
    if (hwnd_) {
        ShowWindow(hwnd_, SW_RESTORE);
    }
}

void Win32Window_C::SetWindowLimits(const WindowLimits& limits) {
    desc_.limits = limits;
    // Limits are enforced in WM_GETMINMAXINFO inside HandleMessage
}

void Win32Window_C::SetCursor(WindowCursor cursor) {
    switch (cursor) {
        case WindowCursor::eHidden:
            while (ShowCursor(FALSE) >= 0) {
            }
            ClipCursor(nullptr);
            break;
        case WindowCursor::eDisabled:
            while (ShowCursor(FALSE) >= 0) {
            }
            if (hwnd_) {
                RECT r{};
                GetClientRect(hwnd_, &r);
                MapWindowPoints(hwnd_, nullptr, reinterpret_cast<POINT*>(&r), 2);
                ClipCursor(&r);
            }
            break;
        case WindowCursor::eNormal:
        default:
            while (ShowCursor(TRUE) < 0) {
            }
            ClipCursor(nullptr);
            break;
    }
}

void Win32Window_C::SetPosition(int x, int y) {
    desc_.position.x = x;
    desc_.position.y = y;
    if (hwnd_) {
        SetWindowPos(hwnd_, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }
}

void Win32Window_C::GetPosition(int& x, int& y) const {
    x = desc_.position.x;
    y = desc_.position.y;
    if (hwnd_) {
        RECT r{};
        if (GetWindowRect(hwnd_, &r)) {
            x = r.left;
            y = r.top;
        }
    }
}

void Win32Window_C::Resize(uint32_t width, uint32_t height) {
    desc_.size.width = width;
    desc_.size.height = height;
    if (hwnd_) {
        SetWindowPos(hwnd_,
                     nullptr,
                     0,
                     0,
                     static_cast<int>(width),
                     static_cast<int>(height),
                     SWP_NOMOVE | SWP_NOZORDER);
    }
}

void Win32Window_C::Close() {
    if (hwnd_) {
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    }
    open_ = false;
}

bool Win32Window_C::IsOpen() const {
    return open_ && hwnd_ != nullptr;
}

void* Win32Window_C::GetNativeWindow() const {
    return hwnd_;
}

NativeWindowHandle Win32Window_C::GetNativeHandle() const {
    NativeWindowHandle handle{};
    handle.api = WindowAPI::eWin32Window;
    handle.hwnd = hwnd_;
    return handle;
}

WindowAPI Win32Window_C::GetWindowAPI() const {
    return WindowAPI::eWin32Window;
}

int Win32Window_C::GetWidth() const {
    return static_cast<int>(desc_.size.width);
}

int Win32Window_C::GetHeight() const {
    return static_cast<int>(desc_.size.height);
}

float Win32Window_C::GetDPIScale() const {
    if (!hwnd_) {
        return 1.0F;
    }
    using GetDpiForWindowFn = UINT(WINAPI*)(HWND);
    static auto fn =
        reinterpret_cast<GetDpiForWindowFn>(GetProcAddress(GetModuleHandleW(L"user32.dll"), "GetDpiForWindow"));
    if (fn) {
        return static_cast<float>(fn(hwnd_)) / 96.0F;
    }
    return 1.0F;
}

std::string Win32Window_C::GetClipboardText() const {
    if (!OpenClipboard(hwnd_)) {
        return {};
    }
    HANDLE h = GetClipboardData(CF_UNICODETEXT);
    if (!h) {
        CloseClipboard();
        return {};
    }
    const auto* wstr = static_cast<const wchar_t*>(GlobalLock(h));
    if (!wstr) {
        CloseClipboard();
        return {};
    }
    const int len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
    std::string result;
    if (len > 1) {
        result.resize(static_cast<size_t>(len - 1));
        WideCharToMultiByte(CP_UTF8, 0, wstr, -1, result.data(), len, nullptr, nullptr);
    }
    GlobalUnlock(h);
    CloseClipboard();
    return result;
}

void Win32Window_C::SetClipboardText(const std::string& text) {
    if (!OpenClipboard(hwnd_)) {
        return;
    }
    EmptyClipboard();
    const int wlen = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
    if (wlen <= 0) {
        CloseClipboard();
        return;
    }
    HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, static_cast<SIZE_T>(wlen) * sizeof(wchar_t));
    if (!hg) {
        CloseClipboard();
        return;
    }
    auto* wstr = static_cast<wchar_t*>(GlobalLock(hg));
    if (wstr) {
        MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wstr, wlen);
        GlobalUnlock(hg);
        SetClipboardData(CF_UNICODETEXT, hg);
    } else {
        GlobalFree(hg);
    }
    CloseClipboard();
}

void Win32Window_C::SetWindowIcon(const uint8_t* rgba_pixels, uint32_t width, uint32_t height) {
    if (!hwnd_ || !rgba_pixels || width == 0 || height == 0) {
        return;
    }
    BITMAPV5HEADER bi{};
    bi.bV5Size = sizeof(BITMAPV5HEADER);
    bi.bV5Width = static_cast<LONG>(width);
    bi.bV5Height = -static_cast<LONG>(height);  // top-down
    bi.bV5Planes = 1;
    bi.bV5BitCount = 32;
    bi.bV5Compression = BI_BITFIELDS;
    bi.bV5RedMask = 0x00FF0000U;
    bi.bV5GreenMask = 0x0000FF00U;
    bi.bV5BlueMask = 0x000000FFU;
    bi.bV5AlphaMask = 0xFF000000U;

    void* bits = nullptr;
    HDC dc = GetDC(nullptr);
    HBITMAP color_bm = CreateDIBSection(dc, reinterpret_cast<BITMAPINFO*>(&bi), DIB_RGB_COLORS, &bits, nullptr, 0);
    ReleaseDC(nullptr, dc);
    if (!color_bm) {
        return;
    }
    auto* dst = static_cast<uint8_t*>(bits);
    const uint32_t px_count = width * height;
    for (uint32_t i = 0; i < px_count; ++i) {
        dst[i * 4 + 0] = rgba_pixels[i * 4 + 2];  // B
        dst[i * 4 + 1] = rgba_pixels[i * 4 + 1];  // G
        dst[i * 4 + 2] = rgba_pixels[i * 4 + 0];  // R
        dst[i * 4 + 3] = rgba_pixels[i * 4 + 3];  // A
    }
    HBITMAP mask_bm = CreateBitmap(static_cast<int>(width), static_cast<int>(height), 1, 1, nullptr);
    ICONINFO ii{};
    ii.fIcon = TRUE;
    ii.hbmColor = color_bm;
    ii.hbmMask = mask_bm;
    HICON icon = CreateIconIndirect(&ii);
    DeleteObject(color_bm);
    DeleteObject(mask_bm);
    if (!icon) {
        return;
    }
    SendMessageW(hwnd_, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(icon));
    SendMessageW(hwnd_, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(icon));
    DestroyIcon(icon);
}

}  // namespace vne::xwin
