include $(SRCTREE)/build/base.mk

TARGET_DIR	:= $(CURDIR)/lib
TARGET_A	:= $(TARGET_DIR)/libcvi_hal_flash.a
TARGET_SO	:= $(TARGET_DIR)/libcvi_hal_flash.so

INCS		+= -I$(CURDIR)/include
INCS		+= -I$(SRCTREE)/cpsl/mmf/include
INCS		+= -I$(SRCTREE)/cpsl/common/log/include
CFLAGS		+= -Wno-unused-result

SEXT		:= c
SDIR		:= $(CURDIR)/src
include $(SRCTREE)/build/common_lib.mk
