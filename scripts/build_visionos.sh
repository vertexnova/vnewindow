#!/bin/bash

#==============================================================================
# VneWindow visionOS Build Script
#==============================================================================
# This script builds vnewindow + examples for visionOS (device or simulator)
# using Xcode + CMake generator.
#
# Prerequisites:
#   - Xcode installed with visionOS + xrsimulator SDKs
#
# Usage:
#   ./scripts/build_visionos.sh [options]
#
# Options:
#   -t <build_type>            Debug|Release|RelWithDebInfo|MinSizeRel (default: Debug)
#   -a <action>                configure|build|configure_and_build|test|xcode|xcode_build (default: configure_and_build)
#   -l <lib_type>              static|shared (default: shared)
#   -clean                     Remove build dirs first
#   -j <jobs>                  Parallel jobs for xcodebuild (default: 10)
#   -xcode                     Also generate dedicated Xcode project dir
#   -xcode-only                Only generate dedicated Xcode project dir
#   -simulator                 Build for visionOS simulator (default)
#   -device                    Build for visionOS device
#   -deployment-target <ver>  visionOS deployment target (default: 1.0)
#   --with-tests               Enable VNE_XWIN_TESTS=ON
#   --with-examples            Enable VNE_XWIN_EXAMPLES=ON
#   -h|--help                  Print help
#==============================================================================

set -e
trap 'echo "ERROR: command failed at line $LINENO"; exit 1' ERR

JOBS=10
ARGS=()

while [[ $# -gt 0 ]]; do
    case $1 in
        -j|--jobs)
            if [[ -n "$2" && "$2" =~ ^[0-9]+$ ]]; then
                JOBS="$2"
                shift 2
            else
                echo "Invalid number of jobs: $2"; exit 1
            fi
            ;;
        -j*)
            JOBS="${1#-j}"
            [[ "$JOBS" =~ ^[0-9]+$ ]] || { echo "Invalid jobs: $JOBS"; exit 1; }
            shift
            ;;
        *)
            ARGS+=("$1")
            shift
            ;;
    esac
done
set -- "${ARGS[@]}"

usage() {
    echo "Usage: $0 [-t <build_type>] [-a <action>] [-l <lib_type>] [-clean] [-j <jobs>] \\"
    echo "          [-xcode] [-xcode-only] [-simulator|-device] [-deployment-target <version>] [--with-tests] [--with-examples]"
    echo ""
    echo "Options:"
    echo "  -t <build_type>    Debug|Release|RelWithDebInfo|MinSizeRel (default: Debug)"
    echo "  -a <action>        configure|build|configure_and_build|test|xcode|xcode_build (default: configure_and_build)"
    echo "  -l <lib_type>      static|shared (default: shared)"
    echo "  -clean             Clean build and xcode directories first"
    echo "  -xcode             Also generate dedicated Xcode project dir"
    echo "  -xcode-only        Only generate dedicated Xcode project dir"
    echo "  -simulator         Build for visionOS simulator (default; arm64 on Apple Silicon)"
    echo "  -device            Build for visionOS device"
    echo "  -deployment-target visionOS deployment target (default: 1.0; X.Y)"
    echo "  --with-tests       Enable VNE_XWIN_TESTS=ON"
    echo "  --with-examples    Enable VNE_XWIN_EXAMPLES=ON"
    echo ""
    exit 1
}

BUILD_TYPE="Debug"
ACTION="configure_and_build"
LIB_TYPE="shared"
CLEAN=false
TARGET="simulator"
XCODE_ONLY=false
GENERATE_XCODE=false
VISIONOS_DEPLOYMENT_TARGET="1.0"
WITH_TESTS=false
WITH_EXAMPLES=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -t|--type|--build-type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        -a|--action)
            ACTION="$2"
            shift 2
            ;;
        -l|--lib-type)
            LIB_TYPE="$2"
            shift 2
            ;;
        -clean|--clean)
            CLEAN=true
            shift
            ;;
        -xcode|--xcode)
            GENERATE_XCODE=true
            shift
            ;;
        -xcode-only|--xcode-only)
            XCODE_ONLY=true
            ACTION="xcode"
            shift
            ;;
        -simulator|--simulator)
            TARGET="simulator"
            shift
            ;;
        -device|--device)
            TARGET="device"
            shift
            ;;
        -deployment-target|--deployment-target)
            VISIONOS_DEPLOYMENT_TARGET="$2"
            shift 2
            ;;
        --with-tests)
            WITH_TESTS=true
            shift
            ;;
        --with-examples)
            WITH_EXAMPLES=true
            shift
            ;;
        -h|--help)
            usage
            ;;
        *)
            echo "Unknown option: $1"
            usage
            ;;
    esac
done

if [[ "$LIB_TYPE" != "static" && "$LIB_TYPE" != "shared" ]]; then
    echo "Invalid --lib-type: $LIB_TYPE (expected static or shared)"; exit 1
fi

check_visionos_environment() {
    echo "=== Checking visionOS Development Environment ==="

    if ! command -v xcodebuild >/dev/null 2>&1; then
        echo "ERROR: Xcode not found. Please install Xcode."
        exit 1
    fi

    if ! xcodebuild -showsdks | grep -q "xros"; then
        echo "ERROR: visionOS SDK not found. Install visionOS SDK in Xcode."
        exit 1
    fi

    if ! xcodebuild -showsdks | grep -q "xrsimulator"; then
        echo "ERROR: visionOS Simulator SDK not found. Install visionOS Simulator SDK in Xcode."
        exit 1
    fi

    if [[ ! "$VISIONOS_DEPLOYMENT_TARGET" =~ ^[0-9]+\.[0-9]+$ ]]; then
        echo "ERROR: Invalid visionOS deployment target format: $VISIONOS_DEPLOYMENT_TARGET"
        echo "Expected format: X.Y (e.g., 1.0, 1.1, 2.0)"
        exit 1
    fi

    echo "visionOS development environment check passed."
    echo ""
}

get_visionos_sdk_paths() {
    if [ "$TARGET" = "device" ]; then
        VISIONOS_SDK_PATH=$(xcrun --sdk xros --show-sdk-path 2>/dev/null)
        if [ -z "$VISIONOS_SDK_PATH" ]; then
            echo "ERROR: visionOS Device SDK not found (xros)."
            exit 1
        fi
        echo "Using visionOS Device SDK: $VISIONOS_SDK_PATH"
    else
        VISIONOS_SDK_PATH=$(xcrun --sdk xrsimulator --show-sdk-path 2>/dev/null)
        if [ -z "$VISIONOS_SDK_PATH" ]; then
            echo "ERROR: visionOS Simulator SDK not found (xrsimulator)."
            exit 1
        fi
        echo "Using visionOS Simulator SDK: $VISIONOS_SDK_PATH"
    fi
}

check_visionos_environment
get_visionos_sdk_paths

COMPILER="clang"
COMPILER_VERSION=$(clang --version | awk 'NR==1 {print $4}' | sed 's/(.*)//')
if [ "$COMPILER_VERSION" = "version" ]; then
    COMPILER_VERSION=$(clang --version | awk 'NR==1 {print $3}')
fi

echo "visionOS :: $COMPILER-${COMPILER_VERSION}"

PROJECT_ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
BUILD_DIR="$PROJECT_ROOT/build/${LIB_TYPE}/${BUILD_TYPE}/build-visionos-$COMPILER-${COMPILER_VERSION}"
XCODE_DIR="$PROJECT_ROOT/build/${LIB_TYPE}/${BUILD_TYPE}/xcode-visionos-$COMPILER-${COMPILER_VERSION}"

if [ "$CLEAN" = true ]; then
    echo "Cleaning build directory: $BUILD_DIR"
    rm -rf "$BUILD_DIR"
    echo "Cleaning Xcode directory: $XCODE_DIR"
    rm -rf "$XCODE_DIR"
fi

mkdir -p "$BUILD_DIR"
mkdir -p "$XCODE_DIR"

CMAKE_ARGS=(
    "-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
    "-DVNE_XWIN_LIB_TYPE=$LIB_TYPE"
    "-DVNE_XWIN_DEV=OFF"
    "-DVNE_XWIN_TESTS=$( [ "$WITH_TESTS" = true ] && echo ON || echo OFF )"
    "-DVNE_XWIN_EXAMPLES=$( [ "$WITH_EXAMPLES" = true ] && echo ON || echo OFF )"
    "-DVNE_TARGET_PLATFORM=visionOS"
    "-DCMAKE_SYSTEM_NAME=visionOS"
    "-DCMAKE_OSX_DEPLOYMENT_TARGET=$VISIONOS_DEPLOYMENT_TARGET"
    "-DCMAKE_C_COMPILER=clang"
    "-DCMAKE_CXX_COMPILER=clang++"
    "-DCMAKE_CXX_STANDARD=20"
    "-DCMAKE_CXX_STANDARD_REQUIRED=ON"
    "-DCMAKE_CXX_EXTENSIONS=OFF"
)

if [ "$TARGET" = "device" ]; then
    CMAKE_ARGS+=(
        "-DCMAKE_OSX_SYSROOT=xros"
        "-DCMAKE_OSX_ARCHITECTURES=arm64"
    )
else
    CMAKE_ARGS+=(
        "-DCMAKE_OSX_SYSROOT=xrsimulator"
        # visionOS simulator on Apple Silicon is arm64-only in practice.
        "-DCMAKE_OSX_ARCHITECTURES=arm64"
    )
fi

run_cmake_configure() {
    local build_path="$1"
    echo "Configuring CMake (Xcode) in: $build_path"
    echo "CMake arguments: ${CMAKE_ARGS[*]}"

    cd "$build_path"
    cmake -G "Xcode" "${CMAKE_ARGS[@]}" "$PROJECT_ROOT"
    cd - > /dev/null
}

run_xcode_build() {
    local xcode_path="$1"
    echo "Building with Xcode in: $xcode_path"

    cd "$xcode_path"

    local xcode_config
    case "$BUILD_TYPE" in
        Release) xcode_config="Release" ;;
        RelWithDebInfo) xcode_config="RelWithDebInfo" ;;
        MinSizeRel) xcode_config="MinSizeRel" ;;
        *) xcode_config="Debug" ;;
    esac

    local xcode_target="xros"
    if [ "$TARGET" = "simulator" ]; then
        xcode_target="xrsimulator"
    fi

    local xcodeproj_candidates=()
    shopt -s nullglob
    xcodeproj_candidates=( *.xcodeproj )
    shopt -u nullglob

    local xcodeproj_file="${xcodeproj_candidates[0]}"
    if [ -z "$xcodeproj_file" ]; then
        echo "ERROR: No Xcode project found in $xcode_path"
        exit 1
    fi

    xcodebuild -project "$xcodeproj_file" -scheme ALL_BUILD -configuration "$xcode_config" -sdk "$xcode_target" -jobs "$JOBS"
    cd - > /dev/null
}

echo "=== VneWindow visionOS Build Script ==="
echo "Build Type: $BUILD_TYPE"
echo "Lib Type: $LIB_TYPE"
echo "Target: $TARGET"
echo "Deployment Target: $VISIONOS_DEPLOYMENT_TARGET"
echo "Action: $ACTION"
echo "Tests Enabled: $WITH_TESTS"
echo "Examples Enabled: $WITH_EXAMPLES"
echo "Jobs: $JOBS"
echo ""

case $ACTION in
    "configure")
        run_cmake_configure "$BUILD_DIR"
        ;;
    "build"|"configure_and_build")
        run_cmake_configure "$BUILD_DIR"
        run_xcode_build "$BUILD_DIR"
        if [ "$GENERATE_XCODE" = true ]; then
            run_cmake_configure "$XCODE_DIR"
        fi
        ;;
    "xcode_build")
        run_cmake_configure "$BUILD_DIR"
        run_xcode_build "$BUILD_DIR"
        ;;
    "xcode"|"xcode-only")
        run_cmake_configure "$XCODE_DIR"
        ;;
    "test")
        if [ "$WITH_TESTS" != true ]; then
            echo "Note: Tests are disabled unless --with-tests is set"
        fi
        run_cmake_configure "$BUILD_DIR"
        run_xcode_build "$BUILD_DIR"
        if [ "$WITH_TESTS" = true ]; then
            echo "Running tests with CTest..."
            ctest --test-dir "$BUILD_DIR" -C "$BUILD_TYPE" --output-on-failure
        else
            echo "Skipping test execution because --with-tests was not set."
        fi
        ;;
    *)
        echo "ERROR: Unknown action: $ACTION"
        usage
        ;;
esac

echo ""
echo "=== Build completed successfully ==="
if [ "$ACTION" = "xcode" ] || [ "$ACTION" = "xcode-only" ]; then
    echo "Xcode project generated in: $XCODE_DIR"
    echo "To open in Xcode: open $XCODE_DIR/*.xcodeproj"
else
    echo "Xcode project generated in: $BUILD_DIR"
    echo "To open in Xcode: open $BUILD_DIR/*.xcodeproj"
    if [ "$GENERATE_XCODE" = true ]; then
        echo "Additional Xcode project in: $XCODE_DIR"
    fi
fi
echo "Build artifacts in: $BUILD_DIR"
echo "Target: $TARGET (visionOS $VISIONOS_DEPLOYMENT_TARGET)"

