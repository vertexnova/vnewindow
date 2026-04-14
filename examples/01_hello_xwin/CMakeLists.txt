#==============================================================================
# 01_hello_xwin: minimal vne::xwin usage (factory + null manager, version)
#==============================================================================

add_executable(example_01_hello_xwin main.cpp)

target_link_libraries(example_01_hello_xwin PRIVATE vne::xwin vne::logging)

target_include_directories(example_01_hello_xwin PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

set_target_properties(example_01_hello_xwin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/examples"
)
