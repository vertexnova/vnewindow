#!/bin/bash

#==============================================================================
# VneTemplate iOS Build Script
#==============================================================================
# Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
# Licensed under the Apache License, Version 2.0 (the "License")
#
# Author:    Ajeet Singh Yadav
# Created:   May 2026
#
# This script builds VneTemplate for iOS devices and simulator.
#==============================================================================

set -e

JOBS=10
ARGS=()

while [[ $# -gt 0 ]]; do
    case $1 in
        -j|--jobs)
            if [[ -n "$2" && "$2" =~ ^[0-9]+$ ]]; then
                JOBS="$2"
                shift 2
            else
                echo "Invalid number of jobs: $2"
                exit 1
            fi
            ;;
        -j*)
            JOBS="${1#-j}"
            if [[ ! "$JOBS" =~ ^[0-9]+$ ]]; then
                echo "Invalid number of jobs: $JOBS"
                exit 1
            fi
            shift
            ;;
        *)
            ARGS+=("$1")
            shift
            ;;
    esac
done
set -- "${ARGS[@]}"

PLATFORM="iOS"
COMPILER="clang"

usage() {
  echo "Usage: $0 [-t <build_type>] [-a <action>] [-l <lib_type>] [-clean] [-j <jobs>] [-xcode] [-xcode-only] [-simulator] [-device] [-deployment-target <version>] [--with-tests] [--with-examples]"
  echo "Options:"
  echo "  -t <build_type>    Debug|Release|RelWithDebInfo|MinSizeRel"
  echo "  -a <action>        configure|build|configure_and_build|test|xcode|xcode_build"
  echo "  -l <lib_type>      static|shared (default: shared). Build dir: build/<lib_type>/..."
  echo "  -clean             Clean build and xcode directories first"
  echo "  -j <jobs>          Number of parallel jobs (default: 10)"
  echo "  -xcode             Also generate dedicated Xcode project dir"
  echo "  -xcode-only        Only generate dedicated Xcode project dir"
  echo "  -simulator         Build for iOS Simulator (default)"
  echo "  -device            Build for iOS Device"
  echo "  -deployment-target iOS deployment target (default: 15.0)"
  echo "  --with-tests       Enable VNE_XWIN_TESTS=ON (off by default for iOS)"
  echo "  --with-examples    Enable VNE_XWIN_EXAMPLES=ON (off by default for iOS)"
  echo ""
  echo "Examples:"
  echo "  $0 -t Debug -a configure_and_build -simulator"
  echo "  $0 -t Release -l static -device -j 8"
  echo "  $0 -xcode-only -t Debug"
  exit 1
}

check_ios_environment() {
  echo "=== Checking iOS Development Environment ==="

  if ! command -v xcodebuild >/dev/null 2>&1; then
    echo "ERROR: Xcode not found. Please install Xcode from the App Store."
    exit 1
  fi

  local xcode_version
  xcode_version=$(xcodebuild -version | awk 'NR==1 {print}')
  echo "Found: $xcode_version"

  if ! xcodebuild -showsdks | rg -q "iphoneos"; then
    echo "ERROR: iOS SDK not found. Please install iOS SDK in Xcode."
    exit 1
  fi

  if ! xcodebuild -showsdks | rg -q "iphonesimulator"; then
    echo "ERROR: iOS Simulator SDK not found. Please install iOS Simulator SDK in Xcode."
    exit 1
  fi

  echo "iOS development environment check passed."
  echo ""
}

get_ios_sdk_paths() {
  if [ "$TARGET" = "device" ]; then
    IOS_SDK_PATH=$(xcrun --sdk iphoneos --show-sdk-path)
    echo "Using iOS Device SDK: $IOS_SDK_PATH"
  else
    IOS_SDK_PATH=$(xcrun --sdk iphonesimulator --show-sdk-path)
    echo "Using iOS Simulator SDK: $IOS_SDK_PATH"
  fi

  if [[ ! $IOS_DEPLOYMENT_TARGET =~ ^[0-9]+\.[0-9]+$ ]]; then
    echo "ERROR: Invalid iOS deployment target format: $IOS_DEPLOYMENT_TARGET"
    echo "Expected format: X.Y (e.g., 15.0, 16.0, 17.0, 18.0)"
    exit 1
  fi

  echo "Using iOS deployment target: $IOS_DEPLOYMENT_TARGET"
}

BUILD_TYPE="Debug"
ACTION="configure_and_build"
LIB_TYPE="shared"
CLEAN=false
TARGET="simulator"
XCODE_ONLY=false
GENERATE_XCODE=false
IOS_DEPLOYMENT_TARGET="15.0"
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
            IOS_DEPLOYMENT_TARGET="$2"
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
    echo "Invalid --lib-type: $LIB_TYPE (expected static or shared)"
    exit 1
fi

check_ios_environment
get_ios_sdk_paths

COMPILER_VERSION=$(clang --version | awk 'NR==1 {print $4}' | sed 's/(.*)//')
if [ "$COMPILER_VERSION" = "version" ]; then
  COMPILER_VERSION=$(clang --version | awk 'NR==1 {print $3}')
fi

echo "$PLATFORM :: $COMPILER-${COMPILER_VERSION}"

PROJECT_ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
BUILD_DIR="$PROJECT_ROOT/build/${LIB_TYPE}/${BUILD_TYPE}/build-ios-$COMPILER-${COMPILER_VERSION}"
XCODE_DIR="$PROJECT_ROOT/build/${LIB_TYPE}/${BUILD_TYPE}/xcode-ios-$COMPILER-${COMPILER_VERSION}"

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
    "-DVNE_TARGET_PLATFORM=iOS"
    "-DCMAKE_SYSTEM_NAME=iOS"
    "-DCMAKE_OSX_DEPLOYMENT_TARGET=$IOS_DEPLOYMENT_TARGET"
    "-DCMAKE_C_COMPILER=clang"
    "-DCMAKE_CXX_COMPILER=clang++"
    "-DCMAKE_CXX_STANDARD=20"
    "-DCMAKE_CXX_STANDARD_REQUIRED=ON"
    "-DCMAKE_CXX_EXTENSIONS=OFF"
)

if [ "$TARGET" = "device" ]; then
    CMAKE_ARGS+=(
        "-DCMAKE_OSX_SYSROOT=iphoneos"
        "-DCMAKE_OSX_ARCHITECTURES=arm64"
    )
else
    CMAKE_ARGS+=(
        "-DCMAKE_OSX_SYSROOT=iphonesimulator"
        "-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64"
    )
fi

run_cmake_configure() {
    local build_path="$1"
    echo "Configuring CMake (Xcode) in: $build_path"
    echo "CMake arguments: ${CMAKE_ARGS[*]}"

    cd "$build_path"
    cmake -G "Xcode" "${CMAKE_ARGS[@]}" "$PROJECT_ROOT"

    if [ $? -ne 0 ]; then
        echo "ERROR: CMake configuration failed"
        exit 1
    fi
    cd - > /dev/null
}

run_xcode_build() {
    local xcode_path="$1"
    echo "Building with Xcode in: $xcode_path"

    cd "$xcode_path"

    local xcode_config="Debug"
    if [ "$BUILD_TYPE" = "Release" ]; then
        xcode_config="Release"
    fi

    local xcode_target="iphoneos"
    if [ "$TARGET" = "simulator" ]; then
        xcode_target="iphonesimulator"
    fi

    local xcodeproj_file
    local xcodeproj_candidates=()
    shopt -s nullglob
    xcodeproj_candidates=( *.xcodeproj )
    shopt -u nullglob
    xcodeproj_file="${xcodeproj_candidates[0]}"
    if [ -z "$xcodeproj_file" ]; then
        echo "ERROR: No Xcode project found in $xcode_path"
        exit 1
    fi

    xcodebuild -project "$xcodeproj_file" -scheme ALL_BUILD -configuration "$xcode_config" -sdk "$xcode_target" -jobs "$JOBS"

    if [ $? -ne 0 ]; then
        echo "ERROR: Xcode build failed"
        exit 1
    fi
    cd - > /dev/null
}

echo "=== VneTemplate iOS Build Script ==="
echo "Platform: $PLATFORM"
echo "Compiler: $COMPILER"
echo "Build Type: $BUILD_TYPE"
echo "Lib Type: $LIB_TYPE"
echo "Target: $TARGET"
echo "iOS Deployment Target: $IOS_DEPLOYMENT_TARGET"
echo "Action: $ACTION"
echo "Generate Xcode: $GENERATE_XCODE"
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
            echo "Note: Tests are disabled for iOS builds unless --with-tests is set"
        fi
        run_cmake_configure "$BUILD_DIR"
        run_xcode_build "$BUILD_DIR"
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
echo "Target: $TARGET (iOS $IOS_DEPLOYMENT_TARGET)"
