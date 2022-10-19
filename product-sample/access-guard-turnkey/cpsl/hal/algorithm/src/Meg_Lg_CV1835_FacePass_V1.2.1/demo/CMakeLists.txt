cmake_minimum_required(VERSION 3.13.5)
project(FacePass_CV1835)

set(CMAKE_CXX_STANDARD 14)

set( EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/${PLATFORM_ARCH} )

if(NOT DEVICE)
    message(STATUS "No device type specified, defaulting to CV1835")
    string(TOLOWER "CV1835" DEVICE)
else()
    string(TOLOWER DEVICE DEVICE)
endif()

include_directories(../include)
link_directories(../lib/${PLATFORM_ARCH})

include_directories(thirdparty/image/include)
link_directories(thirdparty/image/share/${PLATFORM_ARCH})

include_directories(thirdparty/sqlite3/include)
link_directories(thirdparty/sqlite3/lib/${PLATFORM_ARCH})

include_directories(include)

set(OPENCV_LIBS "opencv_core -lopencv_imgproc -lopencv_imgcodecs")

file(GLOB COMMON_FILES "common/*.cpp")
add_library("sample_common" ${COMMON_FILES})

target_link_libraries(sample_common
        "facesdk_${DEVICE}"
        ${OPENCV_LIBS})

add_subdirectory(time_test)
add_subdirectory(search)
add_subdirectory(irprocess)
add_subdirectory(track)
add_subdirectory(videoProcess)
add_subdirectory(calibrate)
