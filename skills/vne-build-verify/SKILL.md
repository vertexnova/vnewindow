---
name: vne-build-verify
description: >
  Standard format, build, and test pipeline to run after modifying C++ or
  Objective-C++ in vnewindow. Use this skill to verify a change before finishing.
---

# Build and Verify

Run this pipeline before completing any change to source. Prefer the
`./scripts/build_*.sh` scripts (or the Windows scripts) over ad-hoc IDE binaries.

This is not vnegfx: there is no `VNE_GFX_*`, no `build/phase3`, no
`init_submodules.sh full` / private `vnerhi`, and no shader regen step.

## 1. Format

C/C++ scope matches CI: `src`, `include`, `examples` (if present), `tests`.

```bash
python3 scripts/clang_formatter.py all              # format everything
python3 scripts/clang_formatter.py all --dry-run    # CI-style check (no writes)
python3 scripts/clang_formatter.py src              # limit to a scope
python3 scripts/clang_formatter.py --file <path>    # a single file
```

Optional wrapper: `./scripts/format.sh` or `./scripts/format.sh -check`.

CI enforces clang-format-17. `.clang-format` and `.clang-tidy` run in separate CI
actions, so format locally rather than hand-adjusting whitespace.

## 2. Dependencies (first checkout or when deps change)

```bash
git submodule update --init --recursive
```

Internal deps are `vnecommon`, `vnelogging`, and `vneevents` under `deps/internal/`.
CMake modules live in `cmake/vnecmake/`. See [deps/README.md](../../deps/README.md).

## 3. Build (macOS)

Scripts use `build/<lib_type>/<build_type>/...` (`lib_type` default: shared):

```bash
./scripts/build_macos.sh -t Debug -a configure_and_build
./scripts/build_macos.sh -l static -t Release -a configure_and_build
```

Or drive CMake into `build/shared` or `build/static` as in the README:

```bash
cmake -B build/shared -DCMAKE_BUILD_TYPE=Debug -DVNE_XWIN_DEV=ON
cmake --build build/shared
```

CMake flags for this repo:

| Flag | Meaning |
|------|---------|
| `VNE_XWIN_LIB_TYPE` | `shared` (default) or `static` |
| `VNE_XWIN_DEV` | Tests + examples |
| `VNE_XWIN_CI` | CI: tests on, examples off |
| `VNE_XWIN_TESTS` | Test suite |
| `VNE_XWIN_EXAMPLES` | Example programs |

Re-run with `-clean` when toggling test/example flags, since that changes the
CMake cache. Do not pass `--dev` or `VNE_GFX_DEV` (those are vnegfx).

## 4. Test

```bash
ctest -C Debug --test-dir build/shared --output-on-failure
```

Or via the script: `./scripts/build_macos.sh -t Debug -a test`. Filter a
single GoogleTest with `--gtest_filter=Suite.TestName`. See
[vne-testing](../vne-testing/SKILL.md) for test conventions.

Headless unit tests use the null backend (`WindowAPI::eNullWindow`). They must
not require a display.

## 5. Other platforms

Same idea; pick the matching script (documented in
[scripts/README.md](../../scripts/README.md) and [README.md](../../README.md)):

| Platform | Script |
|----------|--------|
| Linux | `./scripts/build_linux.sh -t Debug -a configure_and_build` |
| Web / WASM | `./scripts/build_wasm.sh -t Release -a configure_and_build` |
| iOS | `./scripts/build_ios.sh -xcode-only -t Debug -simulator` |
| visionOS | `./scripts/build_visionos.sh -xcode-only -t Debug -simulator` |
| Windows | `python scripts/build_windows.py -t Debug -a configure_and_build` |
| Windows (pwsh) | `.\scripts\build_windows.ps1 -BuildType Debug -Action configure_and_build` |

Add `--with-examples` on iOS/visionOS/wasm when you need example apps. There is
no `scripts/build_android.sh` in this repo; Android is a CMake backend
(`VNE_XWIN_ENABLE_ANDROID`) built from an NDK/Android tree.

Source of truth: [CONTRIBUTING.md](../../CONTRIBUTING.md),
[scripts/README.md](../../scripts/README.md), [README.md](../../README.md).
