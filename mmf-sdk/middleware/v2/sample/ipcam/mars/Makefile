SHELL = /bin/bash

ifeq ($(PARAM_FILE), )
	PARAM_FILE := ../../../Makefile.param
	include $(PARAM_FILE)
endif

DEFS += -DSLICE_BUFF_MODE

# add sensor -D definition
include $(BUILD_PATH)/.config
include ../sensor.mk
DEFS += $(KBUILD_DEFINES)
DEFS += -D_MIDDLEWARE_V2_
GDB_DEBUG = 0
ifeq ($(GDB_DEBUG), 1)
CFLAGS += -g -O0
endif

STATIC = 1

# App source and header file 
# ===============================================================

APP_ROOT         := ..
APP_COMMON_DIR   := $(APP_ROOT)/common
APP_MARS_DIR     := $(APP_ROOT)/mars
APP_MODULE_PATH  := $(APP_MARS_DIR)/modules
APP_SELFTEST_DIR := $(APP_MARS_DIR)/selftest
APP_RESOURCE_DIR := $(APP_ROOT)/resource
APP_INSTALL_DIR  := $(APP_ROOT)/install

SEXT	:= c

# ipcam source		S
# ===============================================================
# main
SDIR	+= $(APP_MARS_DIR)/main

# common
SDIR	+= $(APP_COMMON_DIR)
INCS	= -I$(APP_COMMON_DIR)

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

SDIR	+= $(APP_MODULE_PATH)/peripheral/ircut
INCS	+= -I$(APP_MODULE_PATH)/peripheral/ircut

# AI
AI_ENABLE = 0
ifeq ($(AI_ENABLE), 1)
SDIR	+= $(APP_MODULE_PATH)/ai
INCS	+= -I$(APP_MODULE_PATH)/ai
INCS	+= -I$(APP_RESOURCE_DIR)/jpegturbo/cr182x/include
endif

# RECORD
RECORD_ENBALE = 0
ifeq ($(RECORD_ENBALE), 1)
DEFS 	+= -DRECORD_SUPPORT
SDIR	+= $(APP_MODULE_PATH)/record
INCS	+= -I$(APP_MODULE_PATH)/record
endif

NET_ENABLE = 0
ifeq ($(NET_ENABLE), 1)
DEFS += -DWEB_SOCKET
SDIR	+= $(APP_MODULE_PATH)/network
INCS	+= -I$(APP_MODULE_PATH)/network
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
INCS += -I$(MW_INC) -I$(ISP_INC) -I$(KERNEL_INC) -I$(MW_INC)/isp/mars/

LIBS = -L$(MW_LIB) -lcvi_bin -lcvi_bin_isp -lvpu -lvenc -lvdec -losdc -lsns_full $(ISP_LIB) -lsys 
LIBS += -L$(MW_LIB)/3rd
ifeq ($(SDK_VER), musl_riscv64)
LIBS += -L$(APP_RESOURCE_DIR)/jpegturbo/cr182x/uclibc_lib
else ifeq ($(SDK_VER), glibc_riscv64)
LIBS += -L$(APP_RESOURCE_DIR)/jpegturbo/cr182x/glibc_lib
else
$(error "SDK_VER = $(SDK_VER) not match??")
endif
LIBS += -pthread -lm

# AUDIO
# ===============================================================

ifeq ($(AUDIO_ENABLE), 1)
DEFS += -DAUDIO_SUPPORT
ifeq ($(STATIC), 1)
DEFS += -DCVIAUDIO_STATIC
endif
LIBS  += -lcvi_audio -ltinyalsa -lcvi_vqe -lcvi_ssp -lcvi_RES1 -lcvi_VoiceEngine -ldnvqe 
LIBS  += -laacdec2 -laacenc2 -laacsbrdec2 -laacsbrenc2 -laaccomm2 

MP3_ENABLE = 0
ifeq ($(MP3_ENABLE), 1)
DEFS += -DMP3_SUPPORT
LIBS  += -lcvi_mp3 -lmad
endif
endif

LIBS += -lcli

# AI Module
# ===============================================================
ifeq ($(AI_ENABLE), 1)
DEFS += -DAI_SUPPORT

INCS += -I$(MW_INC)/cviai

ifeq ($(STATIC), 1)		# AI libs static link
AISDK = -lcviai-static -lcviai_app-static
JPEG-TUBRO = -lturbojpeg
IVE = -lcvi_ive
TPU = -lcvikernel-static -lcviruntime-static -lcvimath-static -lcnpy -lz
OPENCV = -lopencv_core -lopencv_imgproc
LIBS += -Wl,--start-group $(AISDK) $(IVE) $(TPU) $(OPENCV) $(JPEG-TUBRO) -Wl,--end-group
else	# AI libs dynamic link
AISDK = -lcviai -lcviai_app
JPEG-TUBRO = -lturbojpeg
IVE = -lcvi_ive
TPU = -lcnpy  -lcvikernel  -lcvimath  -lcviruntime  -lz
# dynamic link needn't opencv-libs
# OPENCV = -lopencv_core -lopencv_imgproc
LIBS += $(AISDK) $(TPU) $(IVE) $(JPEG-TUBRO)
endif	# end --> ifeq ($(STATIC), 0)

endif	# end --> ifeq ($(AI_ENABLE), 1)
# ===============================================================

# Dependent 3rd library
# RTSP 
# ===============================================================
ifeq ($(SDK_VER), musl_riscv64)
RTSP_LIB_DIR = $(APP_RESOURCE_DIR)/rtsp/musl_riscv
else ifeq ($(SDK_VER), glibc_riscv64)
RTSP_LIB_DIR = $(APP_RESOURCE_DIR)/rtsp/glibc_riscv64
else
$(error "SDK_VER = $(SDK_VER) not match??")
endif
INCS += -I$(RTSP_LIB_DIR)/include
ifeq ($(STATIC), 1)
LIBS += -L$(RTSP_LIB_DIR)/lib -Wl,--start-group -lcvi_rtsp -lBasicUsageEnvironment -lgroupsock -lliveMedia -lUsageEnvironment -Wl,--end-group
else
LIBS += -L$(RTSP_LIB_DIR)/lib -lcvi_rtsp
endif
# ===============================================================

# PQ Tools
# you need copy libcvi_ispd.so & libboost_system.so.1.68.0 to your board and add then to LD_LIBRARY_PATH
# you can get *.so from isp_tool_daemon_32bit.tar.gz or ask for FAE
# PQTOOL_SUPPORT
# ===============================================================
PQTOOL_SUPPORT ?= 1
ifeq ($(PQTOOL_SUPPORT), 1)
ifeq ($(STATIC), 1)
LIBS += -Wl,-Bdynamic -ldl -Wl,-Bstatic -lcvi_ispd2 -ljson-c -lvpu
else
LIBS += -ldl -lcvi_ispd2 -ljson-c
endif
# LIBS += -lcvi_ispd2 -ljson-c
DEFS += -DSUPPORT_ISP_PQTOOL
endif
# ===============================================================

# MULTI_PROCESS_SUPPORT
# ===============================================================
ifeq ($(MULTI_PROCESS_SUPPORT), 1)
LIBS += -lnanomsg
DEFS += -DRPC_MULTI_PROCESS
endif
# ===============================================================

# feature select
# ===============================================================
# fast boot 
# DEFS += -DFAST_BOOT_ENABLE
# ===============================================================


# http and websocket
# ===============================================================
ifeq ($(NET_ENABLE), 1)

# thttpd
ifeq ($(SDK_VER), musl_riscv64)
THTTPD_LIB_DIR = $(APP_RESOURCE_DIR)/thttpd/libmusl_riscv64
else ifeq ($(SDK_VER), glibc_riscv64)
THTTPD_LIB_DIR = $(APP_RESOURCE_DIR)/thttpd/libglibc_riscv64
else
$(error "SDK_VER = $(SDK_VER) not match??")
endif
INCS += -I$(APP_RESOURCE_DIR)/thttpd/include
LIBS += -L$(THTTPD_LIB_DIR) -lthttpd

# websockets
ifeq ($(SDK_VER), musl_riscv64)
WEB_SOCKET_LIB_DIR = $(APP_RESOURCE_DIR)/libwebsockets/libmusl_riscv64
else ifeq ($(SDK_VER), glibc_riscv64)
WEB_SOCKET_LIB_DIR = $(APP_RESOURCE_DIR)/libwebsockets/libglibc_riscv64
else
$(error "SDK_VER = $(SDK_VER) not match??")
endif
INCS += -I$(APP_RESOURCE_DIR)/libwebsockets/include
INCS += -I$(APP_RESOURCE_DIR)/libwebsockets/include/libwebsockets
INCS += -I$(APP_RESOURCE_DIR)/libwebsockets/include/libwebsockets/protocols
INCS += -I$(APP_RESOURCE_DIR)/libwebsockets/include/libwebsockets/transports
LIBS += -L$(WEB_SOCKET_LIB_DIR) -lwebsockets

# openssl
OPENSSL_LIB_DIR = $(APP_RESOURCE_DIR)/openssl/lib64bit
INCS += -I$(APP_RESOURCE_DIR)/openssl/include/openssl
# LIBS += -Wl,-Bdynamic -L$(OPENSSL_LIB_DIR) -lcrypto -lssl  -Wl,-Bstatic

endif
# ===============================================================
# Dependent 3rd library End

CFLAGS += $(DEFS)
CFLAGS += -Wno-unused-parameter -D_GNU_SOURCE 

ifeq ($(SDK_VER), musl_riscv64)
ELFFLAGS += -march=rv64imafdcvxthead -mcmodel=medany -mabi=lp64d
else ifeq ($(SDK_VER), glibc_riscv64)
ELFFLAGS += -march=rv64imafdcvxthead -mcmodel=medany -mabi=lp64dv
else
$(error "SDK_VER = $(SDK_VER) not match??")
endif

ifeq ($(STATIC), 1)
ALL_LIBS = -Wl,-Bstatic $(LIBS) -Wl,-Bdynamic
else
ALL_LIBS = $(LIBS)
endif
# debug
# ===============================================================
# $(info "ALL_LIBS=$(ALL_LIBS)")
# ===============================================================

# target 
# ===============================================================
TARGET = ipcam_mars
# ===============================================================

.PHONY : clean all install prepare_mp3
all: $(TARGET)

$(OBJS): %.o : %.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET) : $(OBJS)
	@$(CXX) -o $@ $(OBJS) $(ALL_LIBS) $(ELFFLAGS)
	@$(STRIP) $@
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)

clean:
	$(shell if [ -d $(APP_INSTALL_DIR) ]; then rm -rf $(APP_INSTALL_DIR); fi)
	@rm -f $(OBJS) $(DEPS) $(TARGET)

install:
	$(shell if [ ! -d $(APP_INSTALL_DIR) ]; then mkdir $(APP_INSTALL_DIR); fi)
	@cp -f $(TARGET) $(APP_INSTALL_DIR)

prepare_ai:
	@echo "prepare for uclibc ai"
	@cp -f $(ROOT_DIR)/../../install/soc_$(PROJECT_FULLNAME)/tpu_$(SDK_VER)/cvitek_ai_sdk/lib/libcviai* $(MW_LIB)/3rd;
	@cp -f $(ROOT_DIR)/../../install/soc_$(PROJECT_FULLNAME)/tpu_$(SDK_VER)/cvitek_ai_sdk/lib/libcviai_app* $(MW_LIB)/3rd;
	@cp -f $(ROOT_DIR)/../../install/soc_$(PROJECT_FULLNAME)/tpu_$(SDK_VER)/cvitek_ai_sdk/lib/libopencv_core* $(MW_LIB)/3rd;
	@cp -f $(ROOT_DIR)/../../install/soc_$(PROJECT_FULLNAME)/tpu_$(SDK_VER)/cvitek_ai_sdk/lib/libopencv_imgproc* $(MW_LIB)/3rd;
	@cp -f $(ROOT_DIR)/../../install/soc_$(PROJECT_FULLNAME)/tpu_$(SDK_VER)/cvitek_tpu_sdk/lib/libcnpy.* $(MW_LIB)/3rd;
	@cp -f $(ROOT_DIR)/../../install/soc_$(PROJECT_FULLNAME)/tpu_$(SDK_VER)/cvitek_tpu_sdk/lib/libcvikernel* $(MW_LIB)/3rd;
	@cp -f $(ROOT_DIR)/../../install/soc_$(PROJECT_FULLNAME)/tpu_$(SDK_VER)/cvitek_tpu_sdk/lib/libcvimath* $(MW_LIB)/3rd;
	@cp -f $(ROOT_DIR)/../../install/soc_$(PROJECT_FULLNAME)/tpu_$(SDK_VER)/cvitek_tpu_sdk/lib/libcviruntime* $(MW_LIB)/3rd;
	@cp -f $(ROOT_DIR)/../../install/soc_$(PROJECT_FULLNAME)/tpu_$(SDK_VER)/cvitek_tpu_sdk/lib/libz.* $(MW_LIB)/3rd;
	@cp -rf $(ROOT_DIR)/../../cviai/include $(MW_INC)/cviai;
	
prepare_mp3:
	@echo "prepare for uclibc audio"
	@cp -f $(MW_PATH)/../v1/modules/audio/prebuilt/musl_riscv64/libmad/lib/* $(MW_LIB)/3rd; 
	@cp -f $(MW_PATH)/../v1/modules/audio/prebuilt/musl_riscv64/libmad/include/cvi_mp3_decode.h $(MW_INC)/; 

-include $(DEPS)
