#!/bin/bash
# sudo apt-get install sshpass
DAILY_BUILD="2022-09-15"
PREBUILT_PATH="ftp://10.80.0.5/prebuilt/access-guard-turnkey"
FTP_USER="ftp_admin2"
FTP_PWD="NjW@Uv8II="
DOWNLOAD_CMD="wget --user=$FTP_USER --password=$FTP_PWD --timeout=5 --waitretry=0 --tries=3 --retry-connrefused"
#
CVI_SOC=`echo $CONFIG_CVI_SOC | sed 's/\"//g'`
#
if [ $SDK_VER == "lib32" ]; then
LIB_FOLDER=32bit
elif [ $SDK_VER == "lib64" ]; then
LIB_FOLDER=64bit
elif [ $SDK_VER == "musl_riscv64" ]; then
LIB_FOLDER=musl_riscv64
else
LIB_FOLDER=uclibc
fi

#
AI_SRCPATH=$PREBUILT_PATH/ai_models/$DAILY_BUILD
if [ ! -d $TOP_DIR/prepare ]; then
    echo $TOP_DIR/prepare " folder not exist."
    mkdir -p ${TOP_DIR}/prepare
    echo "download ai_models.tar.gz to "$TOP_DIR/prepare
	echo "$DOWNLOAD_CMD $AI_SRCPATH/ai_models.tar.gz  -P $TOP_DIR/prepare/"
    $DOWNLOAD_CMD $AI_SRCPATH/ai_models.tar.gz  -P $TOP_DIR/prepare
    tar -xvf $TOP_DIR/prepare/ai_models.tar.gz -C $TOP_DIR/prepare
else
    echo $TOP_DIR/prepare " folder already exist."
fi

#
PRE_SRCPATH=$PREBUILT_PATH/mars_master/cr1823_wevb_0007a_spinand/$DAILY_BUILD/sdk_release/extra/$MW_SDK_VER

if [ -d $TOP_DIR/prepare/cvi_aisdk/$CVI_SOC/$LIB_FOLDER ]; then
    echo $TOP_DIR/prepare/cvi_aisdk/$CVI_SOC/$LIB_FOLDER " folder already exist."
    exit 0
fi
#
echo MW_SDK_VER=$MW_SDK_VER" prepare $MW_SDK_VER libraries."
mkdir -p $TOP_DIR/prepare/cvi_aisdk/$CVI_SOC/$LIB_FOLDER
echo "download cvitek_tpu_sdk.tar.gz to "$TOP_DIR/prepare/cvi_aisdk/$CVI_SOC/$LIB_FOLDER
echo "$DOWNLOAD_CMD $PRE_SRCPATH/cvitek_tpu_sdk.tar.gz  $TOP_DIR/prepare/cvi_aisdk/$CVI_SOC/$LIB_FOLDER"
$DOWNLOAD_CMD $PRE_SRCPATH/cvitek_tpu_sdk.tar.gz  -P $TOP_DIR/prepare/cvi_aisdk/$CVI_SOC/$LIB_FOLDER
tar -xvf $TOP_DIR/prepare/cvi_aisdk/$CVI_SOC/$LIB_FOLDER/cvitek_tpu_sdk.tar.gz -C $TOP_DIR/prepare/cvi_aisdk/$CVI_SOC/$LIB_FOLDER
#
echo "download cvitek_ive_sdk.tar.gz to "$TOP_DIR/prepare/$LIB_FOLDER
$DOWNLOAD_CMD $PRE_SRCPATH/cvitek_ive_sdk.tar.gz -P $TOP_DIR/prepare/cvi_aisdk/$CVI_SOC/$LIB_FOLDER
tar -xvf $TOP_DIR/prepare/cvi_aisdk/$CVI_SOC/$LIB_FOLDER/cvitek_ive_sdk.tar.gz -C $TOP_DIR/prepare/cvi_aisdk/$CVI_SOC/$LIB_FOLDER
#
echo "download cvitek_ai_sdk.tar.gz to "$TOP_DIR/prepare/$LIB_FOLDER
$DOWNLOAD_CMD $PRE_SRCPATH/cvitek_ai_sdk.tar.gz  -P $TOP_DIR/prepare/cvi_aisdk/$CVI_SOC/$LIB_FOLDER
tar -xvf $TOP_DIR/prepare/cvi_aisdk/$CVI_SOC/$LIB_FOLDER/cvitek_ai_sdk.tar.gz -C $TOP_DIR/prepare/cvi_aisdk/$CVI_SOC/$LIB_FOLDER
