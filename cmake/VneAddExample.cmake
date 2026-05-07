#==============================================================================
# Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
# Licensed under the Apache License, Version 2.0 (the "License")
#==============================================================================
#
# vne_add_example(TARGET_NAME SOURCES src1.cpp ...)
#
# Injects the correct platform entry point (main.cpp / main.mm / AppDelegate)
# for the current target platform so that example source files are 100%
# platform-agnostic. Links vne::xwin, vne::logging, vne::events, and the
# shared vne_example_common runtime automatically.
#==============================================================================

set(_VNE_EXAMPLE_PLAT_DIR "${CMAKE_CURRENT_LIST_DIR}/../examples/common/platform"
    CACHE INTERNAL "Path to platform entry-point sources")

function(vne_add_example TARGET_NAME)
    cmake_parse_arguments(_ARG "" "" "SOURCES" ${ARGN})

    if(NOT _ARG_SOURCES)
        message(FATAL_ERROR "vne_add_example: no SOURCES given for target ${TARGET_NAME}")
    endif()

    #--------------------------------------------------------------------------
    # Platform-specific executable setup
    #--------------------------------------------------------------------------
    if(VNE_TARGET_PLATFORM STREQUAL "iOS")
        add_executable(${TARGET_NAME} MACOSX_BUNDLE
            ${_ARG_SOURCES}
            "${_VNE_EXAMPLE_PLAT_DIR}/ios/main.mm"
            "${_VNE_EXAMPLE_PLAT_DIR}/ios/app_delegate.h"
            "${_VNE_EXAMPLE_PLAT_DIR}/ios/app_delegate.mm"
        )
        set_target_properties(${TARGET_NAME} PROPERTIES
            MACOSX_BUNDLE            TRUE
            MACOSX_BUNDLE_INFO_PLIST "${_VNE_EXAMPLE_PLAT_DIR}/ios/Info.plist.in"
            MACOSX_BUNDLE_GUI_IDENTIFIER
                "com.vertexnova.vnewindow.${TARGET_NAME}"
            XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER
                "com.vertexnova.vnewindow.${TARGET_NAME}"
            XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"
        )
        target_link_libraries(${TARGET_NAME} PRIVATE
            "-framework UIKit"
            "-framework QuartzCore"
            "-framework Foundation"
        )

    elseif(VNE_TARGET_PLATFORM STREQUAL "macOS")
        add_executable(${TARGET_NAME} MACOSX_BUNDLE
            ${_ARG_SOURCES}
            "${_VNE_EXAMPLE_PLAT_DIR}/desktop/main.cpp"
        )

    elseif(VNE_TARGET_PLATFORM STREQUAL "Windows")
        add_executable(${TARGET_NAME}
            ${_ARG_SOURCES}
            "${_VNE_EXAMPLE_PLAT_DIR}/desktop/main.cpp"
        )

    else()
        # Linux (X11 / Wayland), Android stub, Web stub, etc.
        add_executable(${TARGET_NAME}
            ${_ARG_SOURCES}
            "${_VNE_EXAMPLE_PLAT_DIR}/desktop/main.cpp"
        )
    endif()

    #--------------------------------------------------------------------------
    # Common: shared runtime, public headers, link, MSVC options, output dir
    #--------------------------------------------------------------------------
    target_link_libraries(${TARGET_NAME} PRIVATE
        vne_example_common
        vne::xwin
        vne::logging
        vne::events
    )

    # examples/ is the include root so #include "common/example_base.h" works
    target_include_directories(${TARGET_NAME} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/.."
    )

    set_target_properties(${TARGET_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/examples"
    )

    if(MSVC AND TARGET vnexwin)
        get_target_property(_vx_type vnexwin TYPE)
        if(_vx_type STREQUAL "SHARED_LIBRARY")
            target_compile_options(${TARGET_NAME} PRIVATE /wd4251 /wd4275)
        endif()
    endif()
endfunction()
