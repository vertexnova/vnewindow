<p align="center">
  <img src="icons/vertexnova_logo_medallion_with_text.svg" alt="VertexNova VneCrossWindow" width="320"/>
</p>

<p align="center">
  <strong>Native window surfaces for the VertexNova ecosystem (vne::xwin, no GLFW)</strong>
</p>

<p align="center">
  <a href="https://github.com/vertexnova/vnecrosswindow/actions/workflows/ci.yml">
    <img src="https://github.com/vertexnova/vnecrosswindow/actions/workflows/ci.yml/badge.svg?branch=main" alt="CI"/>
  </a>
  <img src="https://img.shields.io/badge/C%2B%2B-20-blue.svg" alt="C++ Standard"/>
  <a href="https://codecov.io/gh/vertexnova/vnecrosswindow">
    <img src="https://codecov.io/gh/vertexnova/vnecrosswindow/branch/main/graph/badge.svg" alt="Coverage"/>
  </a>
  <img src="https://img.shields.io/badge/license-Apache%202.0-green.svg" alt="License"/>
</p>

---

# VneCrossWindow

Cross-platform **native windowing** library (`vne::xwin`): Win32, Cocoa, X11, optional Wayland (xdg-shell), Emscripten, UIKit, and Android `ANativeWindow`, plus a **null** backend for headless tests. CMake, internal deps (`vnecommon`, `vnelogging`, `vneevents`), tests, and optional examples follow the [VertexNova](https://github.com/vertexnova) conventions.

## Directory layout

| Path | Description |
|------|-------------|
| `cmake/vnecmake/` | CMake modules submodule (ProjectSetup, ProjectWarnings, VneUseDep) |
| `configs/` | Configured headers (e.g. `config.h.in`) |
| `deps/external/` | Third-party deps (e.g. googletest) |
| `deps/internal/` | VertexNova internal libs (vnecommon, vnelogging, vneevents) |
| `include/` | Public API headers (`vertexnova/xwin/`) |
| `src/` | Implementation |
| `tests/` | Unit tests (Google Test) |
| `docs/` | Doxygen input (`doxyfile.in`) and extra docs |
| `scripts/` | Helper scripts (build, format, generate-docs) |

## Prerequisites

- **CMake** 3.19 or newer  
- **C++20** compiler (e.g. GCC 10+, Clang 10+, MSVC 2019+)  
- **Doxygen** (optional, for `scripts/generate-docs.sh` and `-DENABLE_DOXYGEN=ON`)

## Dependencies

- **External:** Tests use [Google Test](https://github.com/google/googletest). Either add `deps/external/googletest` as a submodule (recommended tag: `v1.17.0`) or let CMake use FetchContent when the directory is missing.  
- **Internal:** **vnecmake** (required) is the CMake modules submodule at `cmake/vnecmake`. Libraries `vnecommon`, `vnelogging`, and `vneevents` live under `deps/internal/`. See [deps/README.md](deps/README.md).

From the project root:

```bash
git submodule update --init --recursive
```

(Add submodules first if your repo uses them; see `deps/README.md`.)

## Build

Builds use **`build/static`** or **`build/shared`** (one library type per directory), same layout as **vneio**. With **`VNE_XWIN_LIB_TYPE=shared`**, CMake enables position-independent code, applies the same export/visibility rules as vneio (`VNE_XWIN_BUILDING_DLL` / `VNE_XWIN_DLL`, hidden visibility on Clang/GCC), and installs **`VneXWinTargets.cmake`** (`vne::` namespace) next to the library for `find_package`-style consumers.

From the project root:

```bash
# Shared library (default)
cmake -B build/shared -DCMAKE_BUILD_TYPE=Debug -DVNE_XWIN_TESTS=ON
cmake --build build/shared

# Static library
cmake -B build/static -DCMAKE_BUILD_TYPE=Debug -DVNE_XWIN_LIB_TYPE=static -DVNE_XWIN_TESTS=ON
cmake --build build/static
```

Or use the platform scripts (they use `build/<lib_type>/...`):

```bash
# macOS (default: shared)
./scripts/build_macos.sh -t Debug -a configure_and_build
./scripts/build_macos.sh -l static -t Release -a configure_and_build   # static in build/static/...

# Linux
./scripts/build_linux.sh -t Debug -a configure_and_build
./scripts/build_linux.sh -l static -c clang -a test

# Windows
.\scripts\build_windows.ps1 -BuildType Debug -Action configure_and_build
.\scripts\build_windows.ps1 -LibType static -BuildType Release -Action configure_and_build   # static in build/static/...
```

Options: `-t` / `-BuildType` build type, `-a` / `-Action` action, `-l` / `-LibType` lib type (`static` | `shared`, default `shared`), `-clean` / `-Clean`, `-j N` / `-Jobs N`. macOS script also supports `-xcode` for Xcode project.

## Test

```bash
ctest -C Debug --test-dir build/shared
# or for static: ctest -C Debug --test-dir build/static
```

Or:

```bash
./scripts/build_macos.sh -a test
```

## Documentation

- **Extra docs:** `docs/` (legacy template diagrams may still live under `docs/vertexnova/template/`).
- **API docs:** Configure with Doxygen enabled and build the doc target:

  ```bash
  cmake -B build/shared -DENABLE_DOXYGEN=ON
  cmake --build build/shared --target vnexwin_doc_doxygen
  ```

  Output: `build/shared/docs/html/index.html`.

- **Script:** From project root:

  ```bash
  ./scripts/generate-docs.sh
  ```

  Use `--api-only` to only generate API docs, or `--validate` to only check links and coverage. See `./scripts/generate-docs.sh --help`.

## Format and tidy

- **clang-format:** Config in [.clang-format](.clang-format). Pin to **clang-format-17** for CI-identical output (see CI workflow).
  ```bash
  python3 scripts/clang_formatter.py all            # format (CI dirs)
  python3 scripts/clang_formatter.py all --dry-run  # check only
  ```
  Or `./scripts/format.sh` / `./scripts/format.sh -check`.
- **clang-tidy:** Config in [.clang-tidy](.clang-tidy). After a Clang `Release` configure (e.g. CI: `build/shared/Release`), run `clang-tidy -p build/shared/Release`.

## CI

GitHub Actions runs on push and pull requests to `main`: format check, clang-tidy, and build/test on Linux (GCC, Clang), macOS, and Windows. See [.github/workflows/ci.yml](.github/workflows/ci.yml).

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for build, test, and style. We follow the [Contributor Covenant](CODE_OF_CONDUCT.md) Code of Conduct.

## Releases

Releases are manual. The **VERSION** file at the repo root is the source of truth; CMake reads it at configure time and exposes it as `get_version()`.

To cut a release: update **VERSION**, add a dated entry to **CHANGELOG.md**, commit, create and push a tag (e.g. `git tag v1.0.0 && git push origin v1.0.0`), then create a GitHub Release from that tag and paste the CHANGELOG section.

## License

See [LICENSE](LICENSE).
