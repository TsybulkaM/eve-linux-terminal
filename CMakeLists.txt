cmake_minimum_required(VERSION 3.10)

project(Eve-linux-driver VERSION 1.0 LANGUAGES C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

file(GLOB SOURCES "src/*.c")

add_executable(eveld ${SOURCES})

target_include_directories(eveld PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
)

set(CMAKE_MODULE_PATH 
    "${PROJECT_SOURCE_DIR}/cmake;${PROJECT_SOURCE_DIR}/lib/EVE-Library/CMake;${CMAKE_MODULE_PATH}"
)

set(EVE_BUILD_EXAMPLES OFF CACHE BOOL "Build EVE examples")
set(EVE_BUILD_TESTS OFF CACHE BOOL "Build EVE tests")
option(BUILD_EXAMPLES "Build examples" OFF)
option(BUILD_DEMOS "Build demos" OFF)

add_subdirectory(lib/EVE-Library EXCLUDE_FROM_ALL)

target_include_directories(eveld PRIVATE 
    "${PROJECT_SOURCE_DIR}/lib/EVE-Library"
)

target_link_libraries(eveld PRIVATE eve)
