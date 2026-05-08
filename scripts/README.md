# VneTemplate Scripts

Build scripts for macOS, iOS, Linux, and Windows.

## Build Scripts

### Linux (`build_linux.sh`)

```bash
./scripts/build_linux.sh -t Debug -a configure_and_build
./scripts/build_linux.sh -c clang -j 20 -t Release
./scripts/build_linux.sh -interactive
./scripts/build_linux.sh -clean -t Debug
```

**Options:** `-t` build type, `-c` compiler (gcc|clang), `-a` action (configure|build|configure_and_build|test), `-j` jobs, `-clean`, `-interactive`, `-h`

### macOS (`build_macos.sh`)

```bash
./scripts/build_macos.sh -t Debug -a configure_and_build
./scripts/build_macos.sh -xcode -t Release
./scripts/build_macos.sh -xcode-only -t Release
./scripts/build_macos.sh -interactive
```

**Options:** `-t` build type, `-a` action, `-xcode` (also generate Xcode project), `-xcode-only`, `-j` jobs, `-clean`, `-interactive`, `-h`

### iOS (`build_ios.sh`)

```bash
./scripts/build_ios.sh -t Debug -a configure_and_build -simulator
./scripts/build_ios.sh -t Release -l static -device -j 8
./scripts/build_ios.sh -xcode-only -t Debug
./scripts/build_ios.sh -deployment-target 16.0 -simulator
./scripts/build_ios.sh -xcode-only --with-tests --with-examples -simulator
```

**Options:** `-t` build type, `-a` action, `-l` lib type (`static|shared`), `-simulator|-device`, `-deployment-target`, `--with-tests`, `--with-examples`, `-xcode`, `-xcode-only`, `-j` jobs, `-clean`, `-h`

### visionOS (`build_visionos.sh`)

```bash
# visionOS simulator (examples as .app bundles)
./scripts/build_visionos.sh -t Debug -simulator --with-examples -xcode

# Optional: specify deployment target
./scripts/build_visionos.sh -t Debug -simulator -deployment-target 1.0 --with-examples

# Then run:
#   ./scripts/run_visionos_simulator.sh <path-to-.app>
```

**Options:** `-t` build type, `-a` action, `-l` lib type (`static|shared`), `-simulator|-device`, `-deployment-target`, `--with-tests`, `--with-examples`, `-xcode`, `-xcode-only`, `-j` jobs, `-clean`, `-h`

### Windows

**Bash** (Git Bash / WSL): `build_windows.sh`

```bash
./scripts/build_windows.sh -t Debug -a configure_and_build
./scripts/build_windows.sh -j 8 -t Release
```

**Python** (recommended on Windows): `build_windows.py`

Run from a **Visual Studio Developer Command Prompt**:

```bash
python scripts/build_windows.py -t Debug -a configure_and_build
python scripts/build_windows.py --build-type Release --jobs 8
python scripts/build_windows.py --interactive
python scripts/build_windows.py --build-type Release --clean
```

**Options:** `-t` / `--build-type`, `-a` / `--action`, `-j` / `--jobs`, `--clean`, `--interactive`, `-h`
