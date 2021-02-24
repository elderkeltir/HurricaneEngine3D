cmake_minimum_required(VERSION 3.16)

project(VkRender)

if (WIN32)
    add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
endif (WIN32)

if (UNIX)
    add_definitions(-DVK_USE_PLATFORM_XCB_KHR)
endif (UNIX)

if (APPLE)
    add_definitions(-DVK_USE_PLATFORM_METAL_KHR)
endif (APPLE)


set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

find_package(PkgConfig REQUIRED)
find_package(Vulkan REQUIRED FATAL_ERROR)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/../extern/volk)
include_directories(${PROJECT_SOURCE_DIR}/../extern/glfw/include)
include_directories(${PROJECT_SOURCE_DIR}/../extern/meshoptimizer/src)

add_executable(${PROJECT_NAME} main.cpp Render.cpp
    ${PROJECT_SOURCE_DIR}/../extern/volk/volk.c
    ${PROJECT_SOURCE_DIR}/../extern/meshoptimizer/src/allocator.cpp
    ${PROJECT_SOURCE_DIR}/../extern/meshoptimizer/src/clusterizer.cpp
    ${PROJECT_SOURCE_DIR}/../extern/meshoptimizer/src/indexcodec.cpp
    ${PROJECT_SOURCE_DIR}/../extern/meshoptimizer/src/indexgenerator.cpp
    ${PROJECT_SOURCE_DIR}/../extern/meshoptimizer/src/overdrawanalyzer.cpp
    ${PROJECT_SOURCE_DIR}/../extern/meshoptimizer/src/overdrawoptimizer.cpp
    ${PROJECT_SOURCE_DIR}/../extern/meshoptimizer/src/simplifier.cpp
    ${PROJECT_SOURCE_DIR}/../extern/meshoptimizer/src/spatialorder.cpp
    ${PROJECT_SOURCE_DIR}/../extern/meshoptimizer/src/stripifier.cpp
    ${PROJECT_SOURCE_DIR}/../extern/meshoptimizer/src/vcacheanalyzer.cpp
    ${PROJECT_SOURCE_DIR}/../extern/meshoptimizer/src/vcacheoptimizer.cpp
    ${PROJECT_SOURCE_DIR}/../extern/meshoptimizer/src/vertexcodec.cpp
    ${PROJECT_SOURCE_DIR}/../extern/meshoptimizer/src/vertexfilter.cpp
    ${PROJECT_SOURCE_DIR}/../extern/meshoptimizer/src/vfetchanalyzer.cpp
    ${PROJECT_SOURCE_DIR}/../extern/meshoptimizer/src/vfetchoptimizer.cpp
)
target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan)
target_link_libraries(${PROJECT_NAME} dl)
target_link_libraries(${PROJECT_NAME} pthread)
target_link_libraries(${PROJECT_NAME} /home/ami/Projects/HurricaneEngine3D/extern/glfw/build/src/libglfw3.a) #${PROJECT_SOURCE_DIR}/../extern/glfw/build/src/


install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(FILES main.cpp DESTINATION src)
