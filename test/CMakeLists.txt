include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)
enable_testing()

file(GLOB_RECURSE TEST_SOURCES "*.cpp")
add_executable(vmp_test ${TEST_SOURCES})
target_link_libraries(vmp_test
        PRIVATE
        vmp
        GTest::gtest_main
        GTest::gmock
)

include(GoogleTest)
gtest_discover_tests(vmp_test)
