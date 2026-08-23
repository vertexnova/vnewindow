#==============================================================================
# Copyright (c) 2026 Ajeet Singh Yadav. All rights reserved.
# Licensed under the Apache License, Version 2.0 (the "License")
#==============================================================================

if(NOT TARGET vnexwin)
    return()
endif()

# Load the vne_add_example() helper
include(${PROJECT_SOURCE_DIR}/cmake/VneAddExample.cmake)

#------------------------------------------------------------------------------
# vne_example_common — shared runner runtime compiled once, linked by all examples
#------------------------------------------------------------------------------
add_library(vne_example_common STATIC
    common/example_runner.cpp
)

target_link_libraries(vne_example_common PUBLIC
    vne::xwin
    vne::logging
    vne::events
)

# examples/ is the include root so #include "common/example_base.h" works
target_include_directories(vne_example_common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(MSVC AND TARGET vnexwin)
    get_target_property(_vx_rt_type vnexwin TYPE)
    if(_vx_rt_type STREQUAL "SHARED_LIBRARY")
        target_compile_options(vne_example_common PRIVATE /wd4251 /wd4275)
    endif()
endif()

#------------------------------------------------------------------------------
# Examples
#------------------------------------------------------------------------------
add_subdirectory(01_hello_xwin)
add_subdirectory(02_xwin_events)
add_subdirectory(03_multi_window)
add_subdirectory(04_window_state)
add_subdirectory(05_native_handle)
