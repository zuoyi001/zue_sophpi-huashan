SHELL = /bin/bash

ifeq ($(PARAM_FILE), )
	PARAM_FILE := ../../../Makefile.param
	include $(PARAM_FILE)
endif

# add sensor -D definition
include $(BUILD_PATH)/.config
include ../sensor.mk
DEFS += $(KBUILD_DEFINES)

GDB_DEBUG = 0
ifeq ($(GDB_DEBUG), 1)
CFLAGS += -g -O0
endif

STATIC = 0
ifeq ($(STATIC), 1)
ELFFLAGS += -static
endif

# App source and header file
# ===============================================================

APP_ROOT         := ..
APP_COMMON_DIR   := $(APP_ROOT)/common
APP_MERCURY_DIR  := $(APP_ROOT)/mercury
APP_MODULE_PATH  := $(APP_MERCURY_DIR)/modules
APP_SELFTEST_DIR := $(APP_MERCURY_DIR)/selftest
APP_RESOURCE_DIR := $(APP_ROOT)/resource
APP_INSTALL_DIR  := $(APP_ROOT)/install
APP_TOOLJSON_FILE := $(APP_ROOT)/../../modules/isp/common/toolJsonGenerator/pqtool_definition.json
IS_EXIST = $(shell if [ -f $(APP_TOOLJSON_FILE) ]; then echo "exist"; else echo "noexist"; fi)

SEXT	:= c

# ipcam source		S
# ===============================================================
# main
SDIR	+= $(APP_MERCURY_DIR)/main

# common
SDIR	+= $(APP_COMMON_DIR)
INCS	+= -I$(APP_COMMON_DIR)

# video 
SDIR	+= $(APP_MODULE_PATH)/video
INCS	+= -I$(APP_MODULE_PATH)/video

# osd 
SDIR	+= $(APP_MODULE_PATH)/osd
INCS	+= -I$(APP_MODULE_PATH)/osd

# audio 
AUDIO_ENABLE = 0
ifeq ($(AUDIO_ENABLE), 1)
SDIR	+= $(APP_MODULE_PATH)/audio
INCS	+= -I$(APP_MODULE_PATH)/audio
endif

# param parse
SDIR	+= $(APP_MODULE_PATH)/paramparse
INCS	+= -I$(APP_MODULE_PATH)/paramparse

# peripheral
SDIR	+= $(APP_MODULE_PATH)/peripheral/gpio
INCS	+= -I$(APP_MODULE_PATH)/peripheral/gpio

SDIR	+= $(APP_MODULE_PATH)/peripheral/adc
INCS	+= -I$(APP_MODULE_PATH)/peripheral/adc

SDIR	+= $(APP_MODULE_PATH)/peripheral/pwm
INCS	+= -I$(APP_MODULE_PATH)/peripheral/pwm

# AI
AI_ENABLE = 0
ifeq ($(AI_ENABLE), 1)
SDIR	+= $(APP_MODULE_PATH)/ai
INCS	+= -I$(APP_MODULE_PATH)/ai
endif

# RECORD
RECORD_ENBALE = 0
ifeq ($(RECORD_ENBALE), 1)
DEFS 	+= -DRECORD_SUPPORT
SDIR	+= $(APP_MODULE_PATH)/record
INCS	+= -I$(APP_MODULE_PATH)/record
endif

# self test
SDIR	+= $(APP_SELFTEST_DIR)
INCS	+= -I$(APP_SELFTEST_DIR)

# SRCS ?= $(wildcard $(SDIR)/*.c)
SRCS ?= $(shell find $(SDIR) -type f -name "*.$(SEXT)")
OBJS = $(patsubst %.c, %.o, $(SRCS))
DEPS = $(SRCS:.c=.d)
# ipcam source		E
# ===============================================================

# End of source and header file 
# ===============================================================

# Dependent MW library
# Video && ISP && Sensor
# ===============================================================
INCS += -I$(MW_INC) -I$(ISP_INC) -I$(KERNEL_INC)

ifeq ($(STATIC), 1)
LIBS_SO  = -L$(MW_LIB) -Wl,-Bstatic -Wl,--start-group -lcvi_bin -lcvi_bin_isp -lvpu -lvenc -lvdec -lcvi_jpeg -lcvi_vcodec
LIBS_SO += -lsys -lsns_full $(ISP_LIB) -lcvitracer -L$(MW_LIB)/3rd/ -lini -Wl,--end-group -Wl,-Bdynamic
else
LIBS_SO  = -lcvi_bin -lcvi_bin_isp -lvpu -lvenc -lvdec -lcvi_jpeg -lcvi_vcodec -lsys -lsns_full $(ISP_LIB) -lcvitracer
endif

LIBS_COMM = -lpthread -lm

# AUDIO
# ===============================================================
ifeq ($(AUDIO_ENABLE), 1)
MP3_ENABLE = 0

DEFS += -DAUDIO_SUPPORT

ifeq ($(STATIC), 1)
LIBS_SO  += -lcvi_audio -ltinyalsa -lcvi_vqe -lcvi_ssp -lcvi_RES1 -lcvi_VoiceEngine -ldnvqe 
else
LIBS_SO  += -lcvi_audio -ltinyalsa -lcvi_vqe -lcvi_ssp -lcvi_RES1 -lcvi_VoiceEngine -ldnvqe 

ifeq ($(CHIP_ARCH), CV182X)
LIBS_SO  += -lcvi_ssp2 -laacenc2 -laacdec2
else
LIBS_SO  += -lcli -laacenc2 -laacdec2 -laaccomm2 -laacsbrdec2 -laacsbrenc2
endif

ifeq ($(MP3_ENABLE), 1)
DEFS += -DMP3_SUPPORT
LIBS_SO  += -lcvi_mp3 -lmad
endif
endif

endif
# AI Module
# ===============================================================

# default AI LIBS from dailybuild
AI_LIB_FROM_DAILYBUILD = 1

ifeq ($(AI_ENABLE), 1)
DEFS += -DAI_SUPPORT

INCS += -I$(MW_INC)/cviai

ifeq ($(STATIC), 1)
# AI_SDK
AI_LIBS_A = $(MW_LIB)/libcviai_core-static.a
AI_LIBS_A += $(MW_LIB)/libcviai_service-static.a
# TPU
AI_LIBS_A += $(MW_LIB)/libcvikernel-static.a
AI_LIBS_A += $(MW_LIB)/libcviruntime-static.a
AI_LIBS_A += $(MW_LIB)/libcvimath-static.a
AI_LIBS_A += $(MW_LIB)/libcnpy.a 
AI_LIBS_A += $(MW_LIB)/libz.a
# IVE
AI_LIBS_A += $(MW_LIB)/libcvi_ive_tpu-static.a
# OPENCV
OPENCV_LIBS_A = $(MW_LIB)/libopencv_imgproc.a 
OPENCV_LIBS_A += $(MW_LIB)/libopencv_core.a 
OPENCV_LIBS_A += $(MW_LIB)/libtegra_hal.a

LIBS_A += -Wl,--start-group $(AI_LIBS_A) $(OPENCV_LIBS_A) -Wl,--end-group

else	# AI libs dynamic link

AISDK_LIBS = -lcviai_core -lcviai_service -lcviai_app
IVE_LIBS = -lcvi_ive_tpu
TPU_LIBS = -lcnpy  -lcvikernel  -lcvimath  -lcviruntime  -lz
# OPENCV_LIBS = -lopencv_core -lopencv_imgproc	# dynamic link not need opencv-libs
# ===============================================================

ifeq ($(AI_LIB_FROM_DAILYBUILD), 1)
## for daily build or release build
LIBS_SO += $(AISDK_LIBS) $(TPU_LIBS) $(IVE_LIBS)
else
## for internal build source code 
INCS += -I$(TOP_DIR)/cvitek_ai_sdk/include/cviai
INCS += -I$(TOP_DIR)/cvitek_ive_sdk/include
LIBS_SO += -L$(TOP_DIR)/cvitek_ai_sdk/lib $(AISDK_LIBS)
LIBS_SO += -L$(TOP_DIR)/cvitek_tpu_sdk/lib $(TPU_LIBS)
LIBS_SO += -L$(TOP_DIR)/cvitek_ive_sdk/lib $(IVE_LIBS)
endif

endif	# end --> ifeq ($(STATIC), 0)

endif	# end --> ifeq ($(AI_ENABLE), 1)
# ===============================================================

# Dependent 3rd library
# RTSP 
# ===============================================================
ifeq ($(SDK_VER), 32bit)
RTSP_LIB_DIR = $(APP_RESOURCE_DIR)/rtsp/glibc32
else ifeq ($(SDK_VER), 64bit)
RTSP_LIB_DIR = $(APP_RESOURCE_DIR)/rtsp/glibc64
else ifeq ($(SDK_VER), uclibc)
RTSP_LIB_DIR = $(APP_RESOURCE_DIR)/rtsp/uclibc
endif
INCS += -I$(RTSP_LIB_DIR)/include
RTSP_LIB_STATIC = 1
ifeq ($(RTSP_LIB_STATIC), 1)
LIBS_A += -L$(RTSP_LIB_DIR)/lib -Wl,-Bstatic -Wl,--start-group -lcvi_rtsp -lBasicUsageEnvironment -lgroupsock -lliveMedia -lUsageEnvironment -Wl,--end-group -Wl,-Bdynamic
else
LIBS_SO += -L$(RTSP_LIB_DIR)/lib -Wl,-Bdynamic -lcvi_rtsp
endif
# ===============================================================

# PQ Tools
# you need copy libcvi_ispd.so & libboost_system.so.1.68.0 to your board and add then to LD_LIBRARY_PATH
# you can get *.so from isp_tool_daemon_32bit.tar.gz or ask for FAE
# PQTOOL_SUPPORT
# ===============================================================
PQTOOL_SUPPORT ?= 1
ifeq ($(PQTOOL_SUPPORT), 1)
LIBS_SO += -ldl -lboost_system
DEFS += -DSUPPORT_ISP_PQTOOL
endif
# ===============================================================

# CLI debug
# ===============================================================
ifeq ($(CLI_DEBUG_SUPPORT), 1)
DEFS += -DCLI_DEBUG_SUPPORT
LIBS_SO += -lcli
endif
# ===============================================================

# MULTI_PROCESS_SUPPORT
# ===============================================================
ifeq ($(MULTI_PROCESS_SUPPORT), 1)
LIBS_SO += -lnanomsg
DEFS += -DRPC_MULTI_PROCESS
endif
# ===============================================================

# feature select
# ===============================================================
# fast boot 
# DEFS += -DFAST_BOOT_ENABLE
# ===============================================================

# Dependent 3rd library End

CFLAGS += $(DEFS)
CFLAGS += -Wno-unused-parameter -D_GNU_SOURCE

# dependencies LIBS_ALL list
# ===============================================================
LIBS_ALL = $(LIBS_A) $(LIBS_SO) $(LIBS_COMM)
# ===============================================================

# debug
# ===============================================================
# $(info "INCS=$(INCS)")
# $(warning "dbg print dependencies LIBS_ALL list = $(LIBS_ALL)")
# ===============================================================

# target 
# ===============================================================
TARGET = ipcam_mercury
# ===============================================================

.PHONY : clean all install

all: $(TARGET)

$(OBJS): %.o : %.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET) : $(OBJS)
	@$(CXX) -o $@ $(OBJS) $(LIBS_ALL) $(ELFFLAGS)
	@$(STRIP) $@
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)
ifeq ("$(IS_EXIST)", "exist")
	@cp -af $(APP_TOOLJSON_FILE) .
endif

clean:
	$(shell if [ -d $(APP_INSTALL_DIR) ]; then rm -rf $(APP_INSTALL_DIR); fi)
	@rm -f $(OBJS) $(DEPS) $(TARGET)

install:
	$(shell if [ ! -d $(APP_INSTALL_DIR) ]; then mkdir $(APP_INSTALL_DIR); fi)
	@cp -f $(TARGET) $(APP_INSTALL_DIR)

-include $(DEPS)
