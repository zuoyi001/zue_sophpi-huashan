cmake_minimum_required(VERSION 3.13)

project(
    cvi_signal_slot
    VERSION 1.0.0
    LANGUAGES C CXX
)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/install/)
if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE "Release")
endif ()

set(TOP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../)

include(${TOP_DIR}/cmake/compile_options.cmake)
include(${TOP_DIR}/cmake/summary.cmake)

set(CVI_LOG_DIR ${TOP_DIR}/cpsl/common/log)
set(CVI_LOG_INCLUDE_DIR ${CVI_LOG_DIR}/include)
set(CVI_LOG_LIB_DIR ${CVI_LOG_DIR}/lib)
set(CVI_LOG_LIBS
    -lcvi_log
)

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cvi_signal_slot.cpp
)

set(PUBLIC_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}/cvi_signal_slot.h
)

# build shared library
add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_include_directories(${PROJECT_NAME}
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
PRIVATE
    ${CVI_LOG_INCLUDE_DIR}
)
target_link_directories(${PROJECT_NAME}
PUBLIC
    ${CVI_LOG_LIB_DIR}
)
target_link_libraries(${PROJECT_NAME}
PRIVATE
    ${CVI_LOG_LIBS}
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    PUBLIC_HEADER "${PUBLIC_HEADERS}"
)
install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION include/${PROJECT_NAME}
)

# build static library if need
if (${PROJECT_NAME}_BUILD_STATIC_LIB)
    add_library(${PROJECT_NAME}_static STATIC ${SOURCES})
    target_include_directories(${PROJECT_NAME}_static
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CVI_LOG_INCLUDE_DIR}
    )
    target_link_directories(${PROJECT_NAME}_static
    PUBLIC
        ${CVI_LOG_LIB_DIR}
    )
    target_link_libraries(${PROJECT_NAME}_static
    PRIVATE
        ${CVI_LOG_LIBS}
    )
    set_target_properties(${PROJECT_NAME}_static
        PROPERTIES OUTPUT_NAME ${PROJECT_NAME}
        PUBLIC_HEADER "${PUBLIC_HEADERS}"
    )
    install(TARGETS ${PROJECT_NAME}_static
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include/${PROJECT_NAME}
    )
endif()

# build test
if (${PROJECT_NAME}_BUILD_TEST)
    add_subdirectory(test)
endif()
