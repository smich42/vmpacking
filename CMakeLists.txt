cmake_minimum_required(VERSION 3.14)

project(vmp)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=address,undefined")
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif ()

file(GLOB_RECURSE LIB_SOURCES "src/**.cpp")
file(GLOB_RECURSE LIB_HEADERS "include/**.h")

add_library(vmp STATIC ${LIB_SOURCES} ${LIB_HEADERS}
        examples/basic_example.cpp)

target_include_directories(vmp SYSTEM PUBLIC
        "${CMAKE_SOURCE_DIR}/include"
        "${CMAKE_SOURCE_DIR}/src"
        "${CMAKE_SOURCE_DIR}/src/parsers"
)

add_subdirectory(src)
add_subdirectory(examples)
