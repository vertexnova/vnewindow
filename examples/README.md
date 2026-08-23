# VneWindow Examples

Cross-platform example programs for `vne::xwin`. Each example is a single
`example.cpp` file that works unchanged on every supported platform.

## How it works

`vne_add_example()` (defined in `cmake/VneAddExample.cmake`) injects the
correct platform entry point automatically:

<!-- markdownlint-disable MD060 -->
| Platform | Entry point injected |
|----------|----------------------|
| macOS | `common/platform/desktop/main.cpp` (Cocoa backend) |
| iOS | `common/platform/ios/main.mm` + `app_delegate.mm` (UIKit + CADisplayLink) |
| visionOS | `common/platform/visionos/main.mm` + `app_delegate.mm` + `scene_delegate.mm` (UIKit) |
| Linux | `common/platform/desktop/main.cpp` (X11 or Wayland) |
| Windows | `common/platform/desktop/main.cpp` (Win32) |

The shared `ExampleRunner` (`common/example_runner.h`) manages window creation,
the event loop, and lifecycle. Example source files only implement `ExampleBase`:

```cpp
class MyExample : public vne::xwin::examples::ExampleBase {
    ExampleConfig configure() override { return {"My Window", 800, 600}; }
    void onInit(IWindow& w, IWindowManager& mgr) override { /* setup */ }
    bool onFrame(float dt) override { return true; /* false = close */ }
};

std::unique_ptr<ExampleBase> createExample() {
    return std::make_unique<MyExample>();
}
```

## Building

```bash
# macOS (Cocoa window)
cmake -B build/shared -DVNE_XWIN_EXAMPLES=ON && cmake --build build/shared

# iOS simulator (UIKit window via Xcode)
scripts/build_ios.sh -t Debug -simulator --with-examples -xcode

# visionOS simulator (UIKit window via Xcode)
scripts/build_visionos.sh -t Debug -simulator --with-examples -xcode
# Then run:
#   scripts/run_visionos_simulator.sh <path-to-.app>

# Linux (X11 / Wayland)
cmake -B build/shared -DVNE_XWIN_EXAMPLES=ON && cmake --build build/shared

# Dev mode (tests + examples)
cmake -B build/shared -DVNE_XWIN_DEV=ON && cmake --build build/shared
```

Executables land in `build/<lib_type>/bin/examples/`.

## Available Examples

| Example | Description |
|---------|-------------|
| `example_01_hello_xwin` | Open a native window; log build/version info. |
| `example_02_xwin_events` | Log key/mouse/touch/window/lifecycle events from `vne::events`. |
| `example_03_multi_window` | Open/focus/close multiple windows; log events with per-window id routing. |

<!-- markdownlint-enable MD060 -->

### Manual test checklist — `example_03_multi_window`

Run on macOS, Win32, or Linux (desktop backends with multi-window support):

1. Build and run: `./build/shared/bin/examples/example_03_multi_window` (macOS `.app` path may differ).
2. Press `N` twice — three windows should appear (main + two children).
3. Click in each window — mouse events should show the matching `[win=<id> title="..."]`.
4. Press `F` — focus should cycle; `[WINDOW] FOCUS` events should match the focused window.
5. Press `C` while a child is focused — only that child closes; the app keeps running.
6. Close the main window or press `ESC` — all windows close and the app exits cleanly.

On CI (null backend), the runner performs a smoke cycle and exits without opening windows.

**Web (WASM):** run `./scripts/build_wasm.sh --serve` and open `example_03_multi_window.html`. Multiple windows appear as panels inside the page desktop (`#vne-desktop`); click a panel to focus it before typing.

## Adding a New Example

1. Create `examples/04_my_example/example.cpp` implementing `ExampleBase`.
2. Add to `examples/04_my_example/CMakeLists.txt`:

   ```cmake
   vne_add_example(example_04_my_example SOURCES example.cpp)
   ```

3. Add `add_subdirectory(04_my_example)` to `examples/CMakeLists.txt`.

No platform-specific code needed in `example.cpp`.
