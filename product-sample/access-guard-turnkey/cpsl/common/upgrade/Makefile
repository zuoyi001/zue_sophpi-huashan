include $(SRCTREE)/build/base.mk

TARGET_DIR	:= $(CURDIR)/lib
TARGET_A	:= $(TARGET_DIR)/libcvi_upgrade.a
TARGET_SO	:= $(TARGET_DIR)/libcvi_upgrade.so

############################################
# Dependencies
############################################
MMF_DIR		:= $(SRCTREE)/cpsl/mmf
INCS		+= -I$(MMF_DIR)/include
INCS		+= -I$(MMF_DIR)/sample/common

FLASH_DIR       := $(SRCTREE)/cpsl/hal/flash
INCS            += -I$(FLASH_DIR)/include
LIBS            += -L$(FLASH_DIR)/lib -lcvi_hal_flash

LOG_DIR		:= $(SRCTREE)/cpsl/common/log
INCS		+= -I$(LOG_DIR)/include
LIBS		+= -L$(LOG_DIR)/lib -lcvi_log

OSAL_DIR	:= $(SRCTREE)/cpsl/osal
INCS		+= -I$(OSAL_DIR)/include
LIBS		+= -L$(OSAL_DIR)/lib -lcvi_osal
############################################
# End of Dependencies
############################################
INCS		+= -I$(CURDIR)/include

SEXT		:= c
SDIR		:= $(CURDIR)/src
include $(SRCTREE)/build/common_lib.mk