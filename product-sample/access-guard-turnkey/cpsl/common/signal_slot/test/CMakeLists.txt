cmake_minimum_required(VERSION 3.11.4)

project(cvi_signal_slot_test)

include(${TOP_DIR}/cmake/googletest.cmake)

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/test_main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/unit_test.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME}
    cvi_signal_slot
    -pthread
    gtest
    gtest_main
)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)
