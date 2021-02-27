cmake_minimum_required(VERSION 3.16)

project(VkRender)

set(ROOT_FOLDER ${PROJECT_SOURCE_DIR}/..)
set(EXTERN_FOLDER ${ROOT_FOLDER}/extern)

if (WIN32)
    add_definitions(-DWIN32)
elseif (APPLE)
    add_definitions(-DAPPLE)
elseif (UNIX)
    add_definitions(-DLINUX)
endif ()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${ROOT_FOLDER}/common)
include_directories(${PROJECT_SOURCE_DIR}/../extern/PhysX/physx/include)
include_directories(${PROJECT_SOURCE_DIR}/../extern/PhysX/pxshared/include)

add_library(${PROJECT_NAME} SHARED Physics.cpp
)
target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan)
target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/../extern/glfw/build/src/libglfw3.a)

# if (APPLE)
#     target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
#     target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
#     target_link_libraries(${PROJECT_NAME} "-framework IOKit")
#     target_link_libraries(${PROJECT_NAME} "-framework CoreVideo")
# elseif (UNIX)
#     target_link_libraries(${PROJECT_NAME} dl)
#     target_link_libraries(${PROJECT_NAME} pthread)
# endif ()




install(TARGETS ${PROJECT_NAME} DESTINATION ${PROJECT_SOURCE_DIR}/../Bin)
#install(FILES main.cpp DESTINATION src)
