cmake_minimum_required(VERSION 3.16)

project(PxPhys)

set(ROOT_FOLDER ${PROJECT_SOURCE_DIR}/..)
set(EXTERN_FOLDER ${ROOT_FOLDER}/extern)
set(PHYSX_FOLDER ${EXTERN_FOLDER}/PhysX)

if (WIN32)
    add_definitions(-DWIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-DNOMINMAX)
    add_definitions(-DPXPHYS_EXPORTS)
    set(PHYSX_BIN_FOLDER ${PHYSX_FOLDER}/physx/bin/win.x86_64.vc142.mt/debug)
elseif (APPLE)
    add_definitions(-DAPPLE)
    set(PHYSX_BIN_FOLDER ${PHYSX_FOLDER}/physx/bin/mac.x86_64/debug)
elseif (UNIX)
    add_definitions(-DLINUX)
    set(PHYSX_BIN_FOLDER ${PHYSX_FOLDER}/physx/bin/linux.clang/debug)
endif ()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${ROOT_FOLDER}/common)
include_directories(${PHYSX_FOLDER}/physx/include)
include_directories(${PHYSX_FOLDER}/pxshared/include)

add_library(${PROJECT_NAME} SHARED Physics.cpp
)

if (WIN32)
    set_property(TARGET ${PROJECT_NAME} PROPERTY
         MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    target_link_libraries(${PROJECT_NAME} ${PHYSX_BIN_FOLDER}/PhysX_64.lib)
    target_link_libraries(${PROJECT_NAME} ${PHYSX_BIN_FOLDER}/PhysXCommon_64.lib)
    target_link_libraries(${PROJECT_NAME} ${PHYSX_BIN_FOLDER}/PhysXFoundation_64.lib)
    target_link_libraries(${PROJECT_NAME} ${PHYSX_BIN_FOLDER}/PhysXExtensions_static_64.lib)
    target_link_libraries(${PROJECT_NAME} ${PHYSX_BIN_FOLDER}/PhysXPvdSDK_static_64.lib)

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PHYSX_BIN_FOLDER}/PhysX_64.dll"
        "${CMAKE_BINARY_DIR}/$<CONFIGURATION>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PHYSX_BIN_FOLDER}/PhysXCommon_64.dll"
        "${CMAKE_BINARY_DIR}/$<CONFIGURATION>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PHYSX_BIN_FOLDER}/PhysXFoundation_64.dll"
        "${CMAKE_BINARY_DIR}/$<CONFIGURATION>"
)
elseif(UNIX)
    find_library(PhysX_64_lib PhysX_64 HINTS ${PHYSX_BIN_FOLDER})
    find_library(PhysXCommon_64_lib PhysXCommon_64 HINTS ${PHYSX_BIN_FOLDER})
    find_library(PhysXFoundation_64_lib PhysXFoundation_64 HINTS ${PHYSX_BIN_FOLDER})
    target_link_libraries(${PROJECT_NAME} ${PhysX_64_lib})
    target_link_libraries(${PROJECT_NAME} ${PhysXCommon_64_lib})
    target_link_libraries(${PROJECT_NAME} ${PhysXFoundation_64_lib})

    target_link_libraries(${PROJECT_NAME} ${PHYSX_BIN_FOLDER}/libPhysXPvdSDK_static_64.a)
    target_link_libraries(${PROJECT_NAME} ${PHYSX_BIN_FOLDER}/libPhysXExtensions_static_64.a)
    target_link_libraries(${PROJECT_NAME} ${PHYSX_BIN_FOLDER}/libPhysXCharacterKinematic_static_64.a)
endif()

