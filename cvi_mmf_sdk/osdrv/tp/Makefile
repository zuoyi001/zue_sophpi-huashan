#
# Makefile for the Linux tp network device drivers.
#
-include $(BUILD_PATH)/.config

.PHONY : all clean

all:
ifeq ($(CONFIG_TP_GT9XX),y)
	@if [ -d ts_gt9 ];then cd ts_gt9; make;fi
endif

clean:
	@if [ -d ts_gt9 ];then cd ts_gt9;  make clean;fi