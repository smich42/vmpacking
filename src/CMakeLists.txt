file(GLOB_RECURSE LIB_SOURCES "*.cpp")
file(GLOB_RECURSE LIB_HEADERS "*.h")

add_library(vmp STATIC ${LIB_SOURCES} ${LIB_HEADERS})
target_include_directories(vmp PUBLIC
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/include
)
