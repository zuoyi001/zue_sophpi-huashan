#!/bin/bash
set -e

CUR_DIR="$( cd "$(dirname "$0")" ; pwd -P )"
echo "SDK_PATH = $SDK_PATH"
pushd $CUR_DIR/awtk-linux-fb
SDK_PATH=$SDK_PATH scons -j$NPROC
popd