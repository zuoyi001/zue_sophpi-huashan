#!/bin/sh
INSTALL_PATH=$PWD
echo $INSTALL_PATH
#
export LD_LIBRARY_PATH="/lib:/lib/arm-linux-gnueabihf:/usr/lib:/usr/local/lib:/mnt/system/lib:/mnt/system/usr/lib:/mnt/system/usr/lib/3rd:$INSTALL_PATH/lib"
export PATH="/usr/local/bin:/usr/bin:/bin:/usr/local/sbin:/usr/sbin:/sbin:/mnt/system/usr/bin:/mnt/system/usr/sbin:/mnt/data/bin:/mnt/data/sbin"
export HASPUSER_PREFIX=/mnt/data/auth
##
#check cfbcopyarea.ko
if [ ! -d /sys/module/cfbcopyarea ]; then
    insmod /mnt/system/ko/cfbcopyarea.ko
fi
# check cfbfillrect.ko
if [ ! -d /sys/module/cfbfillrect ]; then
    insmod /mnt/system/ko/cfbfillrect.ko
fi
# check cfbimgblt.ko
if [ ! -d /sys/module/cfbimgblt ]; then
    insmod /mnt/system/ko/cfbimgblt.ko
fi
# check cvi_fb.ko
if [ ! -d /sys/module/cvi_fb ]; then
    insmod /mnt/system/ko/cvi_fb.ko
fi
# check TP module , GT911 driver
if [ ! -d /sys/module/goodix ]; then
    insmod $INSTALL_PATH/ko/goodix.ko
fi

##
# check panel init
#sample_dsi
#echo "panel init ..."
##
# repo
if [ ! -d $INSTALL_PATH/repo ];then
    mkdir $INSTALL_PATH/repo
fi
# save face image if FR pass
if [ ! -d $INSTALL_PATH/record ];then
    mkdir $INSTALL_PATH/record
fi
##

cd $INSTALL_PATH
./sac_application.bin

exit $?
