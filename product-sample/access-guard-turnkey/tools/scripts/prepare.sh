#!/bin/bash
# sudo apt-get install sshpass
DAILY_BUILD="2021-10-08"
PREBUILT_PATH="wangliang@10.80.40.55:/home/wangliang/for_sac_prebuilt"
SSH_PWD="sshpass -p "123456""
#
CVI_SOC=`echo $CONFIG_CVI_SOC | sed 's/\"//g'`
#
if [ $SDK_VER == "lib32" ]; then
LIB_FOLDER=32bit
else
LIB_FOLDER=64bit
fi

#
AI_SRCPATH=$PREBUILT_PATH/ai_models/$DAILY_BUILD
if [ ! -d $TOP_DIR/prepare ]; then
    echo $TOP_DIR/prepare " folder not exist."
    mkdir -p ${TOP_DIR}/prepare
    echo "download ai_models.tar.gz to "$TOP_DIR/prepare
	echo "$SSH_PWD scp -P 22 $AI_SRCPATH/ai_models.tar.gz $TOP_DIR/prepare"
    $SSH_PWD scp -P 22 $AI_SRCPATH/ai_models.tar.gz $TOP_DIR/prepare
    tar -xvf $TOP_DIR/prepare/ai_models.tar.gz -C $TOP_DIR/prepare
else
    echo $TOP_DIR/prepare " folder already exist."
fi

#
if [ $CVI_SOC == "cv183x" ]; then
PRE_SRCPATH=$PREBUILT_PATH/cv183x_master/cv1835_wevb_0001a/$DAILY_BUILD/sdk_release/extra/$MW_SDK_VER
else
PRE_SRCPATH=$PREBUILT_PATH/cv182x_master/cv1825_wevb_0005a/$DAILY_BUILD/sdk_release/extra/$MW_SDK_VER
fi
#
if [ -d $TOP_DIR/prepare/cvi_aisdk/$CVI_SOC/$LIB_FOLDER ]; then
    echo $TOP_DIR/prepare/cvi_aisdk/$CVI_SOC/$LIB_FOLDER " folder already exist."
    exit 0
fi
#
echo MW_SDK_VER=$MW_SDK_VER" prepare $MW_SDK_VER libraries."
mkdir -p $TOP_DIR/prepare/cvi_aisdk/$CVI_SOC/$LIB_FOLDER
echo "download cvitek_tpu_sdk.tar.gz to "$TOP_DIR/prepare/cvi_aisdk/$CVI_SOC/$LIB_FOLDER
$SSH_PWD scp -P 22 $PRE_SRCPATH/cvitek_tpu_sdk.tar.gz  $TOP_DIR/prepare/cvi_aisdk/$CVI_SOC/$LIB_FOLDER
tar -xvf $TOP_DIR/prepare/cvi_aisdk/$CVI_SOC/$LIB_FOLDER/cvitek_tpu_sdk.tar.gz -C $TOP_DIR/prepare/cvi_aisdk/$CVI_SOC/$LIB_FOLDER
#
echo "download cvitek_ive_sdk.tar.gz to "$TOP_DIR/prepare/$LIB_FOLDER
$SSH_PWD scp -P 22 $PRE_SRCPATH/cvitek_ive_sdk.tar.gz  $TOP_DIR/prepare/cvi_aisdk/$CVI_SOC/$LIB_FOLDER
tar -xvf $TOP_DIR/prepare/cvi_aisdk/$CVI_SOC/$LIB_FOLDER/cvitek_ive_sdk.tar.gz -C $TOP_DIR/prepare/cvi_aisdk/$CVI_SOC/$LIB_FOLDER
#
echo "download cvitek_ai_sdk.tar.gz to "$TOP_DIR/prepare/$LIB_FOLDER
$SSH_PWD scp -P 22 $PRE_SRCPATH/cvitek_ai_sdk.tar.gz  $TOP_DIR/prepare/cvi_aisdk/$CVI_SOC/$LIB_FOLDER
tar -xvf $TOP_DIR/prepare/cvi_aisdk/$CVI_SOC/$LIB_FOLDER/cvitek_ai_sdk.tar.gz -C $TOP_DIR/prepare/cvi_aisdk/$CVI_SOC/$LIB_FOLDER
