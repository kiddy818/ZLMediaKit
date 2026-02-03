cmake_minimum_required(VERSION 3.10)
project(example_plugin)

set(CMAKE_CXX_STANDARD 11)

# Find the MediaServer source directory
set(MEDIAKIT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")

# Include directories
include_directories(
    ${MEDIAKIT_ROOT}/server
    ${MEDIAKIT_ROOT}/src
    ${MEDIAKIT_ROOT}/3rdpart
    ${MEDIAKIT_ROOT}/3rdpart/jsoncpp/include
    ${MEDIAKIT_ROOT}/3rdpart/ZLToolKit/src
)

# Create the plugin as a shared library
add_library(example_plugin SHARED example_plugin.cpp)

# Set the output name to just .so without the lib prefix
set_target_properties(example_plugin PROPERTIES PREFIX "")
set_target_properties(example_plugin PROPERTIES OUTPUT_NAME "example_plugin")

# On Linux, allow undefined symbols (they will be resolved from the main executable at runtime)
if(UNIX AND NOT APPLE)
    set_target_properties(example_plugin PROPERTIES LINK_FLAGS "-Wl,--unresolved-symbols=ignore-all")
endif()

