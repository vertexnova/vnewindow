#==============================================================================
# Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
# Licensed under the Apache License, Version 2.0 (the "License")
#
# vne::xwin — native window surfaces (no GLFW)
#==============================================================================

#------------------------------------------------------------------------------
# Backend options (defaults: native desktop/mobile for current OS; Wayland opt-in)
#------------------------------------------------------------------------------
option(VNE_XWIN_ENABLE_NULL "Build null/headless backend (tests)" ON)
option(VNE_XWIN_ENABLE_WIN32 "Build Win32 backend" ON)
option(VNE_XWIN_ENABLE_COCOA "Build macOS Cocoa backend" ON)
option(VNE_XWIN_ENABLE_X11 "Build Linux X11 backend" ON)
option(VNE_XWIN_ENABLE_WAYLAND "Build Linux Wayland (xdg-shell) backend" OFF)
option(VNE_XWIN_ENABLE_WASM "Build Emscripten wasm backend" ON)
option(VNE_XWIN_ENABLE_UIKIT "Build iOS UIKit backend" ON)
option(VNE_XWIN_ENABLE_ANDROID "Build Android ANativeWindow backend" ON)

set(_vx_win32 0)
set(_vx_cocoa 0)
set(_vx_x11 0)
set(_vx_wayland 0)
set(_vx_wasm 0)
set(_vx_uikit 0)
set(_vx_android 0)
set(_vx_null 0)

if(VNE_XWIN_ENABLE_NULL)
    set(_vx_null 1)
endif()

if(WIN32 AND VNE_XWIN_ENABLE_WIN32)
    set(_vx_win32 1)
endif()

set(_vx_is_ios 0)
if(IOS OR CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set(_vx_is_ios 1)
endif()

if(APPLE AND NOT _vx_is_ios AND NOT CMAKE_SYSTEM_NAME STREQUAL "visionOS" AND VNE_XWIN_ENABLE_COCOA)
    set(_vx_cocoa 1)
endif()

if(APPLE AND _vx_is_ios AND VNE_XWIN_ENABLE_UIKIT)
    set(_vx_uikit 1)
endif()

if(ANDROID AND VNE_XWIN_ENABLE_ANDROID)
    set(_vx_android 1)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten" AND VNE_XWIN_ENABLE_WASM)
    set(_vx_wasm 1)
endif()

if(UNIX AND NOT APPLE AND NOT ANDROID AND NOT EMSCRIPTEN)
    if(VNE_XWIN_ENABLE_X11)
        find_package(X11 QUIET)
        if(X11_FOUND)
            set(_vx_x11 1)
        else()
            message(STATUS "vne::xwin: X11 not found — X11 backend disabled")
        endif()
    endif()

    if(VNE_XWIN_ENABLE_WAYLAND)
        find_package(PkgConfig QUIET)
        set(_wl_proto_dir "")
        if(PkgConfig_FOUND)
            pkg_get_variable(_wl_proto_dir wayland-protocols pkgdatadir)
        endif()
        find_program(WAYLAND_SCANNER_EXECUTABLE wayland-scanner)
        if(_wl_proto_dir AND EXISTS "${_wl_proto_dir}/stable/xdg-shell/xdg-shell.xml" AND WAYLAND_SCANNER_EXECUTABLE)
            pkg_check_modules(WAYLAND_CLIENT QUIET wayland-client)
            if(WAYLAND_CLIENT_FOUND)
                set(_vx_wayland 1)
            else()
                message(STATUS "vne::xwin: wayland-client not found — Wayland backend disabled")
            endif()
        else()
            message(STATUS "vne::xwin: wayland-protocols or wayland-scanner missing — Wayland backend disabled")
        endif()
    endif()
endif()

set(XWIN_SOURCES
    xwin_version.cpp
    window.cpp
    window_factory.cpp
    time_step.cpp
    event_bridge.cpp
    platform/null/null_window.cpp
    platform/null/null_window_manager.cpp
)

if(_vx_win32)
    list(APPEND XWIN_SOURCES
        platform/win32/win32_window.cpp
        platform/win32/win32_window_manager.cpp
        platform/win32/xwin_map_key.cpp
    )
endif()

if(_vx_cocoa OR _vx_uikit)
    enable_language(OBJCXX)
endif()

if(_vx_cocoa)
    list(APPEND XWIN_SOURCES
        platform/cocoa/cocoa_window.mm
        platform/cocoa/cocoa_map_key.mm
        platform/cocoa/cocoa_window_manager.mm
    )
endif()

if(_vx_uikit)
    list(APPEND XWIN_SOURCES
        platform/ios/uikit/uikit_window.mm
        platform/ios/uikit/uikit_window_manager.mm
    )
endif()

if(_vx_x11)
    list(APPEND XWIN_SOURCES
        platform/linux/x11/x11_window.cpp
        platform/linux/x11/x11_window_manager.cpp
        platform/linux/x11/xwin_map_key.cpp
    )
endif()

if(_vx_wayland)
    set(_wl_gen "${CMAKE_CURRENT_BINARY_DIR}/generated/wayland")
    set(_wl_xdg_xml "${_wl_proto_dir}/stable/xdg-shell/xdg-shell.xml")
    add_custom_command(
        OUTPUT "${_wl_gen}/xdg-shell-client-protocol.h" "${_wl_gen}/xdg-shell-protocol.c"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${_wl_gen}"
        COMMAND "${WAYLAND_SCANNER_EXECUTABLE}" client-header "${_wl_xdg_xml}" "${_wl_gen}/xdg-shell-client-protocol.h"
        COMMAND "${WAYLAND_SCANNER_EXECUTABLE}" private-code "${_wl_xdg_xml}" "${_wl_gen}/xdg-shell-protocol.c"
        DEPENDS "${_wl_xdg_xml}"
        VERBATIM
    )
    add_custom_target(vnexwin_wayland_protocols DEPENDS
        "${_wl_gen}/xdg-shell-client-protocol.h"
        "${_wl_gen}/xdg-shell-protocol.c"
    )
    list(APPEND XWIN_SOURCES
        platform/linux/wayland/wayland_window.cpp
        platform/linux/wayland/wayland_window_manager.cpp
        platform/linux/wayland/wayland_map_key.cpp
        "${_wl_gen}/xdg-shell-protocol.c"
    )
    set_source_files_properties("${_wl_gen}/xdg-shell-protocol.c" PROPERTIES LANGUAGE C)
endif()

if(_vx_wasm)
    list(APPEND XWIN_SOURCES
        platform/web/wasm/wasm_window.cpp
        platform/web/wasm/wasm_map_key.cpp
        platform/web/wasm/wasm_window_manager.cpp
    )
endif()

if(_vx_android)
    list(APPEND XWIN_SOURCES
        platform/android/android_window.cpp
        platform/android/android_window_manager.cpp
    )
endif()

if(VNE_XWIN_LIB_TYPE STREQUAL "static")
    add_library(vnexwin STATIC ${XWIN_SOURCES})
else()
    add_library(vnexwin SHARED ${XWIN_SOURCES})
endif()

add_library(vne::xwin ALIAS vnexwin)

# Shared library ABI aligned with vneio (export/import macros, visibility, MSVC STL in exported types).
get_target_property(_vnexwin_lib_type vnexwin TYPE)
if(_vnexwin_lib_type STREQUAL "SHARED_LIBRARY")
    target_compile_definitions(vnexwin
        PRIVATE VNE_XWIN_BUILDING_DLL
        INTERFACE VNE_XWIN_DLL
    )
    if(MSVC)
        target_compile_options(vnexwin PRIVATE /wd4251 /wd4275)
    else()
        set_target_properties(vnexwin PROPERTIES
            CXX_VISIBILITY_PRESET hidden
            VISIBILITY_INLINES_HIDDEN ON
        )
    endif()
endif()

set_target_properties(vnexwin PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

if(_vx_cocoa)
    set_source_files_properties(
        platform/cocoa/cocoa_window.mm
        platform/cocoa/cocoa_map_key.mm
        platform/cocoa/cocoa_window_manager.mm
        PROPERTIES COMPILE_OPTIONS "-fobjc-arc"
    )
endif()

if(_vx_uikit)
    set_source_files_properties(
        platform/ios/uikit/uikit_window.mm
        platform/ios/uikit/uikit_window_manager.mm
        PROPERTIES COMPILE_OPTIONS "-fobjc-arc"
    )
endif()

if(_vx_wayland)
    add_dependencies(vnexwin vnexwin_wayland_protocols)
endif()

target_include_directories(vnexwin
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        $<$<BOOL:${_vx_wayland}>:${_wl_gen}>
)

target_compile_features(vnexwin PUBLIC cxx_std_20)
target_link_libraries(vnexwin PUBLIC vne::common vne::logging PRIVATE vne::xwin::BuildSettings)
if(TARGET vne::events)
    target_link_libraries(vnexwin PUBLIC vne::events)
endif()

if(_vx_win32)
    target_link_libraries(vnexwin PRIVATE user32 gdi32)
endif()

if(_vx_cocoa)
    target_link_libraries(vnexwin PRIVATE "-framework Cocoa" "-framework QuartzCore")
endif()

if(_vx_uikit)
    target_link_libraries(vnexwin PRIVATE "-framework UIKit" "-framework QuartzCore")
endif()

if(_vx_x11)
    target_link_libraries(vnexwin PRIVATE X11::X11)
endif()

if(_vx_wayland)
    target_link_libraries(vnexwin PRIVATE ${WAYLAND_CLIENT_LIBRARIES})
    target_include_directories(vnexwin PRIVATE ${WAYLAND_CLIENT_INCLUDE_DIRS})
    # xkbcommon is needed for Wayland key symbol mapping
    pkg_check_modules(XKBCOMMON QUIET xkbcommon)
    if(XKBCOMMON_FOUND)
        target_link_libraries(vnexwin PRIVATE ${XKBCOMMON_LIBRARIES})
        target_include_directories(vnexwin PRIVATE ${XKBCOMMON_INCLUDE_DIRS})
    endif()
endif()

if(_vx_wasm)
    if(TARGET emhtml5)
        target_link_libraries(vnexwin PUBLIC emhtml5)
    endif()
endif()

if(_vx_android)
    target_link_libraries(vnexwin PRIVATE android log)
endif()

target_compile_definitions(vnexwin
    PRIVATE
        VNE_XWIN_HAS_NULL=${_vx_null}
        VNE_XWIN_HAS_WIN32=${_vx_win32}
        VNE_XWIN_HAS_COCOA=${_vx_cocoa}
        VNE_XWIN_HAS_X11=${_vx_x11}
        VNE_XWIN_HAS_WAYLAND=${_vx_wayland}
        VNE_XWIN_HAS_WASM=${_vx_wasm}
        VNE_XWIN_HAS_UIKIT=${_vx_uikit}
        VNE_XWIN_HAS_ANDROID=${_vx_android}
)

set_target_properties(vnexwin PROPERTIES EXPORT_NAME xwin OUTPUT_NAME "vnexwin")
