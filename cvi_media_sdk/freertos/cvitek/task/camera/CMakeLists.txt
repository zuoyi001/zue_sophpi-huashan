file(GLOB _SOURCES "src/${CHIP}/*.c")

if (CONFIG_CHIP_ARCH_mars STREQUAL "y")
add_compile_definitions(ARCH_RTOS_MARS)
endif()

include_directories(include)
include_directories(${CMAKE_INSTALL_INC_PREFIX}/arch)
include_directories(${CMAKE_INSTALL_INC_PREFIX}/common)
include_directories(${CMAKE_INSTALL_INC_PREFIX}/kernel)
include_directories(${CMAKE_INSTALL_INC_PREFIX}/driver/cif)
include_directories(${CMAKE_INSTALL_INC_PREFIX}/driver/vi)
include_directories(${CMAKE_INSTALL_INC_PREFIX}/driver/sensor)
include_directories(${CMAKE_INSTALL_INC_PREFIX}/driver/gpio)
include_directories(${CMAKE_INSTALL_INC_PREFIX}/driver/rtos_cmdqu)
#include_directories(${CMAKE_INSTALL_INC_PREFIX}/driver/fast_image)
include_directories(${CMAKE_INSTALL_INC_PREFIX}/hal/cif)
include_directories(${CMAKE_INSTALL_INC_PREFIX}/hal/config)
include_directories(${CMAKE_INSTALL_INC_PREFIX}/hal/snsr_i2c)
include_directories(../comm/include)

add_library(camera STATIC ${_SOURCES})
install(TARGETS camera DESTINATION lib)

