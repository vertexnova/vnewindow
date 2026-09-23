# Testing Strategy

How `vne::xwin` is tested in this repository and what belongs in interactive desktop validation.

## Layers

| Layer | Where | What it covers |
|-------|--------|----------------|
| **Unit tests** | `tests/` (`vnexwin_tests`) | Factory, null window/manager, event emitter bridge behavior |
| **Example smoke** | `examples/` + `ExampleRunner` | On the **null** backend, open/init is skipped; runner runs one clean cycle and exits (CI-safe) |
| **Desktop / device validation** | Manual examples + **`vnetestbed`** | Real backends (Cocoa, Win32, X11/Wayland, UIKit, WASM shell): multi-window, DPI, focus, input |

Unit tests are headless-friendly. They do not substitute for clicking through a real window on each platform.

## Unit tests

Sources (see `tests/CMakeLists.txt`):

- `window_factory_test.cpp` — factory availability, selection, errors
- `null_window_test.cpp` / `null_window_manager_test.cpp` — null backend contracts
- `event_emitter_test.cpp` — emission / gating behavior used by backends

Enable and run:

```bash
cmake -B build/shared -DCMAKE_BUILD_TYPE=Debug -DVNE_XWIN_TESTS=ON
cmake --build build/shared
ctest -C Debug --test-dir build/shared --output-on-failure
```

Or with platform scripts, e.g. `./scripts/build_macos.sh -a test`.

Dev builds can turn on tests and examples together: `-DVNE_XWIN_DEV=ON`.

## CI

Pipelines pass `-DVNE_XWIN_CI=ON`, which enables the test suite appropriate for the agent OS. Prefer the **null** path for automated smoke where a display is unavailable. Format and clang-tidy jobs also run on `src/`, `include/`, and `tests/` (see `.github/workflows/ci.yml`).

## Examples vs vnetestbed

- **Examples** (`example_01` … `example_03`) are the in-repo demos and the first place to reproduce API regressions. On null/CI they smoke-exit; on desktop they exercise real windows. See [examples/README.md](../examples/README.md) (including the multi-window checklist).
- **`vnetestbed`** is the broader interactive host for longer-lived desktop/device scenarios (input, lifecycle, multi-window stress) that are awkward as one-shot CI processes. Use it after unit tests pass when validating platform-specific behavior.

## Guidance

- Add or update a **unit test** when changing factory rules, null contracts, or event emission gating.
- Add or extend an **example** when documenting a new user-facing workflow.
- Use **desktop / vnetestbed** for focus, DPI, clipboard, multi-window, and mobile/web lifecycle checks that unit tests cannot cover.
