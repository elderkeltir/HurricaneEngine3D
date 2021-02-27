cmake_minimum_required(VERSION 3.16)

project(VkRender)

set(ROOT_FOLDER ${PROJECT_SOURCE_DIR}/..)
set(EXTERN_FOLDER ${ROOT_FOLDER}/extern)

if (WIN32)
    add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-DNOMINMAX)
    add_definitions(-DVKRENDER_EXPORTS)
elseif (APPLE)
    add_definitions(-DVK_USE_PLATFORM_METAL_EXT)
    add_definitions(-DVK_ENABLE_BETA_EXTENSIONS)
elseif (UNIX)
    add_definitions(-DVK_USE_PLATFORM_XCB_KHR)
endif ()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG")

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${ROOT_FOLDER}/Common)
include_directories(${EXTERN_FOLDER}/volk)
include_directories(${EXTERN_FOLDER}/glfw/include)
include_directories(${EXTERN_FOLDER}/meshoptimizer/src)
if (WIN32)
    include_directories($ENV{VULKAN_SDK}/Include)
endif()

add_library(${PROJECT_NAME} SHARED Render.cpp
    ${EXTERN_FOLDER}/volk/volk.c
    ${EXTERN_FOLDER}/meshoptimizer/src/allocator.cpp
    ${EXTERN_FOLDER}/meshoptimizer/src/clusterizer.cpp
    ${EXTERN_FOLDER}/meshoptimizer/src/indexcodec.cpp
    ${EXTERN_FOLDER}/meshoptimizer/src/indexgenerator.cpp
    ${EXTERN_FOLDER}/meshoptimizer/src/overdrawanalyzer.cpp
    ${EXTERN_FOLDER}/meshoptimizer/src/overdrawoptimizer.cpp
    ${EXTERN_FOLDER}/meshoptimizer/src/simplifier.cpp
    ${EXTERN_FOLDER}/meshoptimizer/src/spatialorder.cpp
    ${EXTERN_FOLDER}/meshoptimizer/src/stripifier.cpp
    ${EXTERN_FOLDER}/meshoptimizer/src/vcacheanalyzer.cpp
    ${EXTERN_FOLDER}/meshoptimizer/src/vcacheoptimizer.cpp
    ${EXTERN_FOLDER}/meshoptimizer/src/vertexcodec.cpp
    ${EXTERN_FOLDER}/meshoptimizer/src/vertexfilter.cpp
    ${EXTERN_FOLDER}/meshoptimizer/src/vfetchanalyzer.cpp
    ${EXTERN_FOLDER}/meshoptimizer/src/vfetchoptimizer.cpp
)

if (NOT WIN32)
    find_package(PkgConfig REQUIRED)
    find_package(Vulkan REQUIRED FATAL_ERROR)
    target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan)
    target_link_libraries(${PROJECT_NAME} ${EXTERN_FOLDER}/glfw/build/src/libglfw3.a)
else ()
    target_link_libraries(${PROJECT_NAME} ${EXTERN_FOLDER}/glfw/build/src/Debug/glfw3.lib)
    target_link_libraries(${PROJECT_NAME} $ENV{VULKAN_SDK}/Lib/vulkan-1.lib)
endif()

if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
elseif (UNIX)
    target_link_libraries(${PROJECT_NAME} dl)
    target_link_libraries(${PROJECT_NAME} pthread)
endif ()
