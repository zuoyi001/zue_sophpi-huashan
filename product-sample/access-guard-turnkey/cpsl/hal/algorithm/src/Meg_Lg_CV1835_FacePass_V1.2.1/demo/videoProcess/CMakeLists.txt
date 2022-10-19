
include_directories(${CMAKE_SOURCE_DIR}/thirdparty/sqlite3/include)
link_directories("${CMAKE_SOURCE_DIR}/thirdparty/sqlite3/lib/${PLATFORM_ARCH}")
set(SQLITE_LIBS "sqlite3")

add_executable(videoProcess videoProcess.cpp)
target_link_libraries(videoProcess sample_common ${SQLITE_LIBS} -ldl -pthread)
