#==============================================================================
# 01_hello_xwin: minimal vne::xwin usage (factory + null manager, version)
#==============================================================================

add_executable(example_01_hello_xwin main.cpp)

if(IOS)
    set_target_properties(example_01_hello_xwin PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.vertexnova.vnewindow.example01"
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.vertexnova.vnewindow.example01"
    )
endif()

if(MSVC AND TARGET vnexwin)
    get_target_property(_vx_ex_type vnexwin TYPE)
    if(_vx_ex_type STREQUAL "SHARED_LIBRARY")
        target_compile_options(example_01_hello_xwin PRIVATE /wd4251 /wd4275)
    endif()
endif()

target_link_libraries(example_01_hello_xwin PRIVATE vne::xwin vne::logging)

target_include_directories(example_01_hello_xwin PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

set_target_properties(example_01_hello_xwin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/examples"
)
