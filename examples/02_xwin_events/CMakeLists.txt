#==============================================================================
# 02_xwin_events: vne::xwin + vne::events (Input, EventManager, optional callbacks)
#==============================================================================

add_executable(example_02_xwin_events main.cpp)

if(IOS)
    set_target_properties(example_02_xwin_events PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.vertexnova.vnewindow.example02"
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.vertexnova.vnewindow.example02"
    )
endif()

if(MSVC AND TARGET vnexwin)
    get_target_property(_vx_ex_type vnexwin TYPE)
    if(_vx_ex_type STREQUAL "SHARED_LIBRARY")
        target_compile_options(example_02_xwin_events PRIVATE /wd4251 /wd4275)
    endif()
endif()

target_link_libraries(example_02_xwin_events PRIVATE vne::xwin vne::logging vne::events)

target_include_directories(example_02_xwin_events PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

set_target_properties(example_02_xwin_events PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/examples"
)
