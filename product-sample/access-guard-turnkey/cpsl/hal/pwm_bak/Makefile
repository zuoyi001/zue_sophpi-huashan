include $(SRCTREE)/build/base.mk

TARGET_DIR	:= $(CURDIR)/lib
TARGET_A	:= $(TARGET_DIR)/libcvi_hal_pwm.a
TARGET_SO	:= $(TARGET_DIR)/libcvi_hal_pwm.so

LOG_DIR     := $(SRCTREE)/cpsl/common/log
INCS        += -I$(LOG_DIR)/include

HAL_DIR     := $(SRCTREE)/cpsl/hal
INCS        += -I$(HAL_DIR)/include

UTILS_DIR   := $(SRCTREE)/applications/common/utils
INCS        += -I$(UTILS_DIR)/include

INCS		+= -I$(CURDIR)/include
INCS		+= -I$(CURDIR)/../include
INCS		+= -I$(SRCTREE)/cpsl/common/sysutils/include
CFLAGS		+= -Wno-unused-result

SEXT		:= c
SDIR		:= $(CURDIR)/src
include $(SRCTREE)/build/common_lib.mk
