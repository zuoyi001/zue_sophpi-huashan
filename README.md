# 华山派

![pic](https://github.com/sophgo/sophpi-huashan/blob/master/document/assets/1.jpg)

1.freertos编译
a) source build/cvisetup.sh
b) defconfig cv1812h_wevb_0007a_emmc
c) clean_uboot;build_uboot

2.freertos烧录
a) 将freertos/cvitek/install/bin/cvirtos.bin及fsbl/build/fip.bin拷贝至SD卡。
b) 将SD卡插到板子上，接上电源直接上电。
c) 等待串口打印显示升级完成后，在mars_c906#之后输入re重启。
d) 重启过程中按住enter键不放，进入uboot命令行界面。
e) 执行fatload mmc 1:1 0x80200000 cvirtos.bin && go 0x80200000即可。
