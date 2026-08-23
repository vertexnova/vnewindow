#==============================================================================
# Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
# Licensed under the Apache License, Version 2.0 (the "License")
#
# Author:    Ajeet Singh Yadav
# Created:   February 2026
#
# Autodoc:   yes
#==============================================================================

if(NOT TARGET vnexwin)
    return()
endif()

#==============================================================================
# Setup Google Test
#==============================================================================
if(EXISTS "${PROJECT_SOURCE_DIR}/deps/external/googletest/CMakeLists.txt")
    message(STATUS "Using Google Test from deps/external/googletest (v1.17.0)")
    add_subdirectory(${PROJECT_SOURCE_DIR}/deps/external/googletest ${CMAKE_BINARY_DIR}/deps/external/googletest)
    include(GoogleTest)
    set(GTEST_LIBRARIES gtest_main gmock)
    set(GTEST_INCLUDE_DIRS
        ${PROJECT_SOURCE_DIR}/deps/external/googletest/googletest/include
        ${PROJECT_SOURCE_DIR}/deps/external/googletest/googlemock/include
    )
else()
    find_package(GTest QUIET)
    if(GTest_FOUND)
        message(STATUS "Using system Google Test")
        set(GTEST_LIBRARIES GTest::gtest_main GTest::gmock)
        set(GTEST_INCLUDE_DIRS "")
    else()
        message(STATUS "deps/external/googletest not found, using FetchContent")
        include(FetchContent)
        FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG        v1.17.0
        )
        FetchContent_MakeAvailable(googletest)
        set(GTEST_LIBRARIES gtest_main gmock)
        set(GTEST_INCLUDE_DIRS ${googletest_SOURCE_DIR}/googletest/include ${googletest_SOURCE_DIR}/googlemock/include)
    endif()
endif()

#==============================================================================
# Test sources
#==============================================================================
set(TEST_SOURCES
    window_factory_test.cpp
    null_window_test.cpp
    null_window_manager_test.cpp
    event_emitter_test.cpp
)

#==============================================================================
# Build test executable
#==============================================================================
add_executable(vnexwin_tests ${TEST_SOURCES})

if(IOS)
    # iOS simulator install requires a bundle identifier for app targets.
    set_target_properties(vnexwin_tests PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.vertexnova.vnewindow.tests"
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.vertexnova.vnewindow.tests"
    )
endif()

# Shared vnexwin + MSVC: STL in headers from exported types (same pattern as vneio tests).
if(MSVC AND TARGET vnexwin)
    get_target_property(_vnexwin_tests_lib_type vnexwin TYPE)
    if(_vnexwin_tests_lib_type STREQUAL "SHARED_LIBRARY")
        target_compile_options(vnexwin_tests PRIVATE /wd4251 /wd4275)
    endif()
endif()

target_include_directories(vnexwin_tests
    PRIVATE
        ${GTEST_INCLUDE_DIRS}
        ${VNE_INCLUDE_DIR}
        ${VNE_SRC_DIR}
)

target_link_libraries(vnexwin_tests
    PRIVATE
        ${GTEST_LIBRARIES}
        vne::xwin
)

# Tests call vne::events APIs directly; link the same vneevents module vnexwin uses so
# EventManager / Input singletons are not duplicated inside the test exe on Windows DLL builds.
if(TARGET vne::events)
    target_link_libraries(vnexwin_tests PRIVATE vne::events)
endif()

if(VNE_XWIN_LIB_TYPE STREQUAL "shared")
    if(WIN32)
        if(TARGET vneevents)
            add_custom_command(TARGET vnexwin_tests POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_FILE:vneevents>
                    $<TARGET_FILE_DIR:vnexwin_tests>
                VERBATIM)
        endif()
        if(TARGET vnexwin)
            add_custom_command(TARGET vnexwin_tests POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_FILE:vnexwin>
                    $<TARGET_FILE_DIR:vnexwin_tests>
                VERBATIM)
        endif()
    else()
        set(_vnexwin_tests_rpaths "")
        if(TARGET vneevents)
            list(APPEND _vnexwin_tests_rpaths "$<TARGET_FILE_DIR:vneevents>")
        endif()
        if(TARGET vnexwin)
            list(APPEND _vnexwin_tests_rpaths "$<TARGET_FILE_DIR:vnexwin>")
        endif()
        if(_vnexwin_tests_rpaths)
            set_target_properties(vnexwin_tests PROPERTIES
                BUILD_RPATH "${_vnexwin_tests_rpaths}"
            )
        endif()
    endif()
endif()

add_test(NAME vnexwin_tests COMMAND vnexwin_tests)
