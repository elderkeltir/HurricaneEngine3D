cmake_minimum_required(VERSION 3.16)

project(Launcher)

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
include_directories(${PROJECT_SOURCE_DIR}/../VkRender)
#include_directories(${PROJECT_SOURCE_DIR}/../PxPhys/)

add_executable(${PROJECT_NAME} main.cpp
)
# add_subdirectory(${PROJECT_SOURCE_DIR}/../VkRender build)
target_link_libraries(${PROJECT_NAME} VkRender)
#target_link_libraries(${PROJECT_NAME} PxPhys)


install(TARGETS ${PROJECT_NAME} DESTINATION ${PROJECT_SOURCE_DIR}/../Bin)
#install(FILES main.cpp DESTINATION src)

# set_target_properties(${PROJECT_NAME} PROPERTIES
#     INSTALL_RPATH "."
#     BUILD_WITH_INSTALL_RPATH 1
# )