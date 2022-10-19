include $(SRCTREE)/build/base.mk

TARGET_DIR	:= $(CURDIR)/lib
TARGET_A	:= $(TARGET_DIR)/libcvi_hfsm.a
TARGET_SO	:= $(TARGET_DIR)/libcvi_hfsm.so

############################################
# Dependencies
############################################
LOG_DIR		:= $(SRCTREE)/cpsl/common/log
INCS		+= -I$(LOG_DIR)/include

EVENTHUB_DIR:= $(SRCTREE)/cpsl/common/eventhub
INCS		+= -I$(EVENTHUB_DIR)/include

OSAL_DIR	:= $(SRCTREE)/cpsl/osal
INCS		+= -I$(OSAL_DIR)/include
# CFLAGS 		+= -g -O0
############################################
# End of Dependencies
############################################
INCS		+= -I$(CURDIR)/include

SEXT		:= c
SDIR		:= $(CURDIR)/src
include $(SRCTREE)/build/common_lib.mk
