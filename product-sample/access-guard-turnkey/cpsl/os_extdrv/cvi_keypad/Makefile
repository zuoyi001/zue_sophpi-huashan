PWD = $(shell pwd)

TOP_DIR := $(PWD)/../..
SDK_PATH :=$(TOP_DIR)/..

#cvi_keypad-objs += cvi_keypad.o
obj-m = cvi_keypad.o
KRNELDIR = $(shell readlink -f $(SDK_PATH)/linux-linaro-stable/build/cv1835_wevb_0002a)
#CROSS_COMPILE =arm-linux-
#CC :=  aarch64-linux-gnu-gcc
#LD := aarch64-linux-gnu-ld
all:
	make -C $(KRNELDIR) M=$(PWD) modules ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- 
#	make -C $(KRNELDIR) M=$(PWD) modules ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-
        
.PHONY :clean
clean:
	rm -rf *.o *ko *.order *.symvers .*.cmd *.mod.c .tmp_versions 