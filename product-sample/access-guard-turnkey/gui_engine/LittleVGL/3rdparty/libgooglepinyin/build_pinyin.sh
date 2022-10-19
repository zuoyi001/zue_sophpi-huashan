#!/bin/bash
mkdir -p build
cd build
cmake .. -DCMAKE_CXX_COMPILER=/data/tarzan.zhao/Mars_sdk/host-tools/gcc/riscv64-linux-musl-x86_64/bin/riscv64-unknown-linux-musl-g++ -DCMAKE_INSTALL_PREFIX=../install_dir
#cmake .. -DCMAKE_CXX_COMPILER=$TOOLCHAIN_PATH/arm-linux-gnueabihf-g++ -DCMAKE_INSTALL_PREFIX=../install_dir
make
