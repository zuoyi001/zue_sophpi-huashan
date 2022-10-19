#!/bin/bash
set -e # Exit immediately if a command exits with a non-zero status.
echo "building demo"

print_usage() {
    set +x
    echo -e "build and install sample programs."
    echo -e "Usage: ${0} PLATFORM_ARCH BOARD_DIR TOOLCHAIN_DIR"
    echo -e "Usage: PLATFORM_ARCH: arm32 or arm64"
    echo -e "Usage: BOARD_DIR: the directory to be mounted to the board"
    echo -e "Usage: TOOLCHAIN_DIR: the directory of toolchain, should match the PLATFORM_ARCH"
    echo "----------------------------------------"
}

# command_line_argument
if [ $# -ne 3 ]; then
    print_usage
    exit -1
fi

Platform_ARCH=$1
Board_DIR=$2
Toolchain_DIR=$3

export PATH="${Toolchain_DIR}/bin:$PATH"
if [ $Platform_ARCH == "arm32" ]
then
    Build_DIR=build_arm32
    echo "create ${Build_DIR}..."
    mkdir -p ${Build_DIR}
    cd ${Build_DIR}
    rm  -f CMakeCache.txt
    cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain/arm-linux-gnueabihf.toolchain.cmake -DPLATFORM_ARCH=arm32 ..

elif [ $Platform_ARCH == "arm64" ]
then
    Build_DIR=build_arm64
    echo "create ${Build_DIR}..."
    mkdir -p ${Build_DIR}
    cd ${Build_DIR}
    rm  -f CMakeCache.txt
    cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain/aarch64-linux-gnu.toolchain.cmake -DPLATFORM_ARCH=arm64 ..
else
    echo -e "error, unknow platform arch !!!"
    print_usage
    exit -1
fi

echo "make -j$(nproc)"
make -j$(nproc)
cd ..

readonly Bin=./bin
readonly Lib=../lib
readonly Model=../model
readonly Config=../model/model.yaml
readonly Script=./run.sh
readonly Thirdparty=./thirdparty

echo  "------------------------------------------------------------------------"
echo "copy ${Bin}/${Platform_ARCH} to ${Board_DIR}"
cp -r ${Bin}/${Platform_ARCH}/* ${Board_DIR}
echo "copy ${Lib}/${Platform_ARCH} to ${Board_DIR}"
cp -r ${Lib}/${Platform_ARCH}/* ${Board_DIR}
echo "copy ${Thirdparty}/image/share/${Platform_ARCH}/* to ${Board_DIR}/thirdparty/image/share"
mkdir -p ${Board_DIR}/thirdparty/image/share
cp -r ${Thirdparty}/image/share/${Platform_ARCH}/* ${Board_DIR}/thirdparty/image/share
mkdir -p ${Board_DIR}/thirdparty/sqlite3/lib
echo "copy ${Thirdparty}/sqlite3/lib/${Platform_ARCH}/* to ${Board_DIR}/thirdparty/sqlite3/lib"
cp -r ${Thirdparty}/sqlite3/lib/${Platform_ARCH}/* ${Board_DIR}/thirdparty/sqlite3/lib
echo "copy ${Model} to ${Board_DIR}"
cp -r ${Model} ${Board_DIR}
echo "copy ${Script} to ${Board_DIR}"
cp  ${Script} ${Board_DIR}

