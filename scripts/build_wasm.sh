#!/bin/bash

#==============================================================================
# VneWindow WebAssembly / Emscripten Build Script
#==============================================================================
# Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
# Licensed under the Apache License, Version 2.0 (the "License")
#
# Author:    Ajeet Singh Yadav
# Created:   May 2026
#
# Autodoc:   yes
#
# Builds vnewindow and its examples for WebAssembly using Emscripten.
# Emscripten must be installed and activated before calling this script:
#
#   source /path/to/emsdk/emsdk_env.sh
#
# Usage:
#   ./scripts/build_wasm.sh [options]
#
# Options:
#   -t <build_type>   Debug|Release|RelWithDebInfo|MinSizeRel  (default: Debug)
#   -a <action>       configure|build|configure_and_build|serve  (default: configure_and_build)
#   -l <lib_type>     static|shared  (default: static)
#   -clean            Remove the build directory first
#   -j <jobs>         Parallel jobs  (default: 10)
#   --with-examples   Enable examples (default; explicit only)
#   --no-examples     Disable examples (VNE_XWIN_EXAMPLES=OFF)
#   --serve           Start a local HTTP server after building
#   -h|--help         Print this help message
#==============================================================================

set -e
trap 'echo "ERROR: command failed at line $LINENO"; exit 1' ERR

JOBS=10
ARGS=()

while [[ $# -gt 0 ]]; do
    case $1 in
        -j|--jobs)
            if [[ -n "$2" && "$2" =~ ^[0-9]+$ ]]; then
                JOBS="$2"; shift 2
            else
                echo "Invalid number of jobs: $2"; exit 1
            fi
            ;;
        -j*)
            JOBS="${1#-j}"
            [[ "$JOBS" =~ ^[0-9]+$ ]] || { echo "Invalid jobs: $JOBS"; exit 1; }
            shift
            ;;
        *) ARGS+=("$1"); shift ;;
    esac
done
set -- "${ARGS[@]}"

usage() {
    echo "Usage: $0 [-t <build_type>] [-a <action>] [-l <lib_type>] [-clean] [-j <jobs>] [--with-examples|--no-examples] [--serve]"
    echo ""
    echo "Options:"
    echo "  -t <build_type>   Debug|Release|RelWithDebInfo|MinSizeRel  (default: Debug)"
    echo "  -a <action>       configure|build|configure_and_build|serve"
    echo "  -l <lib_type>     static|shared  (default: static)"
    echo "  -clean            Remove the build directory first"
    echo "  -j <jobs>         Parallel jobs  (default: 10)"
    echo "  --with-examples   Build examples (default; flag is optional)"
    echo "  --no-examples     Omit examples (library-only wasm build)"
    echo "  --serve           Start a local HTTP server on port 8080 after building"
    echo "  -h|--help         Print this help"
    echo ""
    echo "Prerequisites:"
    echo "  Install Emscripten: https://emscripten.org/docs/getting_started/downloads.html"
    echo "  Activate:  source /path/to/emsdk/emsdk_env.sh"
    exit 1
}

BUILD_TYPE="Debug"
ACTION="configure_and_build"
LIB_TYPE="static"
CLEAN=false
WITH_EXAMPLES=true
SERVE=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -t|--build-type) BUILD_TYPE="$2"; shift 2 ;;
        -a|--action)     ACTION="$2"; shift 2 ;;
        -l|--lib-type)   LIB_TYPE="$2"; shift 2 ;;
        -clean|--clean)  CLEAN=true; shift ;;
        --with-examples) WITH_EXAMPLES=true; shift ;;
        --no-examples)   WITH_EXAMPLES=false; shift ;;
        --serve)         SERVE=true; shift ;;
        -h|--help)       usage ;;
        *) echo "Unknown option: $1"; usage ;;
    esac
done

# ---------------------------------------------------------------------------
# Environment checks
# ---------------------------------------------------------------------------
check_emscripten() {
    if ! command -v emcc >/dev/null 2>&1; then
        echo ""
        echo "ERROR: emcc not found in PATH."
        echo ""
        echo "Install Emscripten and activate it:"
        echo "  git clone https://github.com/emscripten-core/emsdk.git"
        echo "  cd emsdk"
        echo "  ./emsdk install latest"
        echo "  ./emsdk activate latest"
        echo "  source ./emsdk_env.sh"
        echo ""
        exit 1
    fi

    if ! command -v emcmake >/dev/null 2>&1; then
        echo "ERROR: emcmake not found. Make sure the full Emscripten SDK is activated."
        exit 1
    fi

    local emcc_ver
    emcc_ver=$(emcc --version | head -n 1)
    echo "Found: $emcc_ver"
}

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------
PROJECT_ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
BUILD_DIR="$PROJECT_ROOT/build/wasm/${LIB_TYPE}/${BUILD_TYPE}"
SERVE_DIR="$BUILD_DIR/bin/examples"

# ---------------------------------------------------------------------------
# CMake arguments
# ---------------------------------------------------------------------------
CMAKE_ARGS=(
    "-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
    "-DVNE_XWIN_LIB_TYPE=$LIB_TYPE"
    "-DVNE_XWIN_DEV=OFF"
    "-DVNE_XWIN_TESTS=OFF"
    "-DVNE_XWIN_EXAMPLES=$( [ "$WITH_EXAMPLES" = true ] && echo ON || echo OFF )"
    "-DVNE_TARGET_PLATFORM=Web"
    "-DCMAKE_CXX_STANDARD=20"
    "-DCMAKE_CXX_STANDARD_REQUIRED=ON"
    "-DCMAKE_CXX_EXTENSIONS=OFF"
)

# ---------------------------------------------------------------------------
# Generator detection: prefer Ninja, fall back to Unix Makefiles
# ---------------------------------------------------------------------------
if command -v ninja >/dev/null 2>&1; then
    CMAKE_GENERATOR="Ninja"
    BUILD_TOOL_DESC="Ninja"
else
    CMAKE_GENERATOR="Unix Makefiles"
    BUILD_TOOL_DESC="make"
fi

# ---------------------------------------------------------------------------
# Actions
# ---------------------------------------------------------------------------
run_configure() {
    echo "=== Configuring CMake (Emscripten, $BUILD_TOOL_DESC) in: $BUILD_DIR ==="
    echo "CMake args: ${CMAKE_ARGS[*]}"
    echo ""

    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    emcmake cmake -G "$CMAKE_GENERATOR" "${CMAKE_ARGS[@]}" "$PROJECT_ROOT"
    cd - > /dev/null
}

run_build() {
    echo "=== Building with $BUILD_TOOL_DESC in: $BUILD_DIR ==="
    cd "$BUILD_DIR"
    emmake cmake --build . --parallel "$JOBS"
    cd - > /dev/null
}

run_serve() {
    if [ ! -d "$SERVE_DIR" ]; then
        echo "WARNING: Example output directory not found: $SERVE_DIR"
        echo "Run a build with examples enabled first (default; do not pass --no-examples)."
        exit 1
    fi

    echo ""
    echo "=== Starting HTTP server ==="
    echo "Serving: $SERVE_DIR"
    echo "Open in browser: http://localhost:8080"
    echo "Press Ctrl+C to stop."
    echo ""

    local py_cmd=""
    if command -v python3 >/dev/null 2>&1; then
        py_cmd=python3
    elif command -v python >/dev/null 2>&1 \
        && python -c 'import sys; sys.exit(0 if sys.version_info >= (3, 0) else 1)' 2>/dev/null; then
        py_cmd=python
    fi
    if [ -z "$py_cmd" ]; then
        echo "ERROR: Python 3 not found. Install Python 3 to use --serve."
        exit 1
    fi
    cd "$SERVE_DIR"
    "$py_cmd" -m http.server 8080
}

# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------
echo "=== VneWindow WebAssembly Build Script ==="
echo "Build type : $BUILD_TYPE"
echo "Lib type   : $LIB_TYPE"
echo "Action     : $ACTION"
echo "Examples   : $WITH_EXAMPLES"
echo "Serve      : $SERVE"
echo "Jobs       : $JOBS"
echo "Build dir  : $BUILD_DIR"
echo ""

if [ "$ACTION" != serve ]; then
    check_emscripten
fi

if [ "$CLEAN" = true ]; then
    echo "Cleaning: $BUILD_DIR"
    rm -rf "$BUILD_DIR"
fi

case $ACTION in
    configure)
        run_configure
        ;;
    build)
        if [ ! -d "$BUILD_DIR" ]; then
            run_configure
        fi
        run_build
        ;;
    configure_and_build)
        run_configure
        run_build
        ;;
    serve)
        run_serve
        exit 0
        ;;
    *)
        echo "Unknown action: $ACTION"; usage
        ;;
esac

echo ""
echo "=== Build completed successfully ==="
echo "Artifacts : $BUILD_DIR/bin/examples/"
echo ""
echo "To open in a browser, serve the output directory:"
echo "  cd $SERVE_DIR"
echo "  python3 -m http.server 8080"
echo "  open http://localhost:8080"
echo ""
echo "Or re-run with --serve:"
echo "  $0 -t $BUILD_TYPE --serve"
echo ""

if [ "$SERVE" = true ]; then
    run_serve
fi
