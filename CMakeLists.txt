cmake_minimum_required(VERSION 3.16)

project(HurricaneEngine3D)

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}")

add_subdirectory (VkRender)
add_subdirectory (Launcher)
