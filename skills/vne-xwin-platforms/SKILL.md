---
name: vne-xwin-platforms
description: >
  Conventions for vnewindow platform backends (Win32, Cocoa, X11, Wayland,
  UIKit, Android, WASM, null), native handles, and thread rules. Use this skill
  when editing src/vertexnova/xwin/platform/, public window APIs, or
  NativeWindowHandle.
---

# vne::xwin Platform Backends

vnewindow provides native window surfaces (`vne::xwin`) without GLFW. Rendering
is external: `IWindow::swapBuffers()` may be a no-op when a GL/Metal/Vulkan
context is owned by the RHI, not by this library.

## 1. Layout

| Path | Role |
|------|------|
| `include/vertexnova/xwin/` | Public C++ API |
| `src/vertexnova/xwin/` | Shared types, factory, event emitter |
| `src/vertexnova/xwin/platform/null/` | Headless backend for tests |
| `.../platform/win32/` | Win32 |
| `.../platform/cocoa/` | macOS AppKit (`.mm`) |
| `.../platform/linux/x11/` | X11 (optional XCB handle bridge) |
| `.../platform/linux/wayland/` | Wayland xdg-shell (optional) |
| `.../platform/uikit/` | iOS / visionOS UIKit (`.mm`) |
| `.../platform/android/` | `ANativeWindow` |
| `.../platform/wasm/` | Emscripten canvas |
| `examples/` | Cross-platform `ExampleBase` programs |

CMake backend toggles live in `src/vertexnova/xwin/CMakeLists.txt`
(`VNE_XWIN_ENABLE_*`). Do not enable a backend on the wrong OS.

## 2. Public types

- `IWindow` / `IWindowManager` are the interfaces; concrete classes stay in
  `src/` and are not part of the installed API.
- Construct managers with `WindowFactory`. Tests use `WindowAPI::eNullWindow`.
- `NativeWindowHandle` fields are **non-owning**. Do not `release`/`DestroyWindow`
  through the handle; the backend `IWindow` owns the native object.
- Fill only the fields that match `handle.api`. Leave others defaulted (null / 0).
- Input mapping converts native tokens to `vne::events` types
  (`include/vertexnova/xwin/input_mapping.h`).

## 3. Threading and lifecycle

- AppKit and UIKit: initialize, poll, and mutate windows on the thread that
  owns the native event loop (typically main). Document exceptions on the method.
- `IWindow::create()` already calls `initialize(descriptor)`. Do not initialize
  twice unless a backend documents reinitialization.
- On `ApplicationLifecycle::eLowMemory`, release caches; mobile hosts may kill
  the process if ignored.
- Multi-window is desktop-oriented. UIKit/Android/WASM often have one scene;
  `IWindowManager` methods that cannot apply must return false / no-op, not crash.

## 4. Events

- Window backends emit through the existing event bridge (`event_emitter`);
  do not invent a second event path.
- Honor descriptor flags that disable events or input; see
  `tests/event_emitter_test.cpp`.
- Key/mouse/touch mapping must round-trip through the helpers in `input_mapping`
  and the per-platform `*_map_key` files.

## 5. When adding a backend change

1. Keep the public `IWindow` / `IWindowManager` surface stable.
2. Update the matching null-backend test if the contract is shared.
3. If only a real window can show the behavior, extend an example under
   `examples/` and the manual checklist in `examples/README.md`.
4. Do not add GLFW or other windowing libraries.

See [vne-coding-style](../vne-coding-style/SKILL.md) and
[vne-testing](../vne-testing/SKILL.md).
