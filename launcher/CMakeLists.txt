cmake_minimum_required(VERSION 3.16)

project(Launcher)

set(ROOT_FOLDER ${PROJECT_SOURCE_DIR}/..)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG")

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${ROOT_FOLDER}/include)
include_directories(${ROOT_FOLDER}/physics)
include_directories(${ROOT_FOLDER}/renderer)

add_executable(${PROJECT_NAME} 
    main.cpp
    Engine.cpp
)

target_link_libraries(${PROJECT_NAME} physics)
target_link_libraries(${PROJECT_NAME} renderer)
