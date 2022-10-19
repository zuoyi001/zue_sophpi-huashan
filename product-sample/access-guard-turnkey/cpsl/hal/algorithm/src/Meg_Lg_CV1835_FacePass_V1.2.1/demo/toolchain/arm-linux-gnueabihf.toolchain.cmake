# set cross-compiled system type, it's better not use the type which cmake cannot recognized.
SET ( CMAKE_SYSTEM_NAME Linux )
SET ( CMAKE_SYSTEM_PROCESSOR arm )
# when hislicon SDK was installed, toolchain was installed in the path as below:
SET ( CMAKE_C_COMPILER "arm-linux-gnueabihf-gcc" )
SET ( CMAKE_CXX_COMPILER "arm-linux-gnueabihf-g++" )

# set searching rules for cross-compiler
SET ( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER )
SET ( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY )
SET ( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY )

# set ${CMAKE_C_FLAGS} and ${CMAKE_CXX_FLAGS}flag for cross-compiled process
SET ( CMAKE_CXX_FLAGS "-std=c++14 -march=armv7-a -mfloat-abi=hard -mfpu=neon-vfpv4 -mtune=cortex-a53 -fopenmp ${CMAKE_CXX_FLAGS}" )
add_compile_options(-fPIC -fno-lto  -Wall)
# disable automatic generation of FMAs (would break exact
# predicates)
add_compile_options(-ffp-contract=off)

# Explicitly set the build type to Release if no other type is specified
# on the command line.  Without this, cmake defaults to an unoptimized,
# non-debug build, which almost nobody wants.
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type specified, defaulting to Release")
    set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    message(STATUS "Configuring in debug mode")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -Wall -g -ggdb")
elseif(CMAKE_BUILD_TYPE MATCHES Release)
    message(STATUS "Configuring in release mode")
    set( CMAKE_BUILD_TYPE "Release" )
    add_compile_options(-Os -s -fPIC -fvisibility=hidden  -ffunction-sections -fdata-sections)
    add_compile_options(
        #"$<$<COMPILE_LANGUAGE:C>:${c_flags}>"
        "$<$<COMPILE_LANGUAGE:CXX>:${-Wnon-virtual-dtor}>"
        "$<$<COMPILE_LANGUAGE:CXX>:${-fvisibility-inlines-hidden}>"
        )
    add_link_options(-Wl,--gc-sections -Wl,--strip-all)
    #优先链接内部函数，将静态库中的所有函数隐藏
    add_link_options(-Wl,-Bsymbolic -Wl,--exclude-libs,ALL)
endif()
