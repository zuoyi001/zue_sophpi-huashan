#!/bin/bash

CUR_DIR="$( cd "$(dirname "$0")" ; pwd -P )"
SDK_PATH="$(dirname "$(which riscv64-unknown-linux-musl-g++)")"
echo "SDK_PATH = $SDK_PATH"

pushd $CUR_DIR/awtkcode/awtk-linux-fb
SDK_PATH=$SDK_PATH scons
popd
