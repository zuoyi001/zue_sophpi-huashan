obj-m = tmel_drive.o
#linux_dir:=/home/suntianfeng/samba_share/git_pd/hisi3559_bsp/kernel/linux-4.9.y
linux_dir:=/home/data/CVITEK/cv183x_v1.3.0_source/linux-linaro-stable/build/cv1835_asic_wevb_0002a
PWD = $(shell pwd)
#EXTRA_CFLAGS += -Wno-error=date-time
#EXTRA_CFLAGS += -Wno-date-time
#EXTRA_CFLAGS += -Wdeclaration-after-statement
#EXTRA_CFLAGS += -fno-stack-protector
Q ?= @
S ?= -s

all:
	$(Q)make $(S) ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -C $(linux_dir) M=$(PWD)
clean:
	$(Q)make $(S) -C $(linux_dir) M=$(PWD) clean
