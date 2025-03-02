file(GLOB EXAMPLE_SOURCES "*.cpp")

foreach (EXAMPLE_FILE ${EXAMPLE_SOURCES})
    get_filename_component(EXAMPLE_NAME ${EXAMPLE_FILE} NAME_WE)
    add_executable(${EXAMPLE_NAME} ${EXAMPLE_FILE}
            experiment.cpp)
    target_link_libraries(${EXAMPLE_NAME} PRIVATE vmp)
endforeach ()