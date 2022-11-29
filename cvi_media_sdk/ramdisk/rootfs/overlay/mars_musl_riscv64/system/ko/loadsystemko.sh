#!/bin/sh
${CVI_SHOPTS}
#
# Start to insert kernel modules
#
insmod /mnt/system/ko/mars_sys.ko
insmod /mnt/system/ko/mars_base.ko
insmod /mnt/system/ko/mars_rtos_cmdqu.ko
insmod /mnt/system/ko/mars_fast_image.ko
insmod /mnt/system/ko/cvi_mipi_rx.ko
insmod /mnt/system/ko/snsr_i2c.ko
insmod /mnt/system/ko/mars_vi.ko
insmod /mnt/system/ko/mars_vpss.ko
insmod /mnt/system/ko/mars_dwa.ko
insmod /mnt/system/ko/mars_vo.ko
insmod /mnt/system/ko/mars_mipi_tx.ko
insmod /mnt/system/ko/mars_rgn.ko

#insmod /mnt/system/ko/mars_wdt.ko
insmod /mnt/system/ko/mars_clock_cooling.ko

insmod /mnt/system/ko/mars_tpu.ko
insmod /mnt/system/ko/mars_vcodec.ko
insmod /mnt/system/ko/mars_jpeg.ko
insmod /mnt/system/ko/cvi_vc_driver.ko MaxVencChnNum=9 MaxVdecChnNum=9
#insmod /mnt/system/ko/mars_rtc.ko
insmod /mnt/system/ko/mars_ive.ko

echo 3 > /proc/sys/vm/drop_caches
dmesg -n 4

#usb hub control
#/etc/uhubon.sh host

exit $?
