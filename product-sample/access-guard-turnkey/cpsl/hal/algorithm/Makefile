-include $(TOP_DIR)/.config
##
PWD := $(shell pwd)
SDIR := $(PWD)
##
ifeq ($(CONFIG_ALGORITHM_VENDOR_MEGVII), y)
TARGET = libcvi_hal_face_megvii.so
SRCS := $(SDIR)/src/cvi_hal_face_megvii.cpp
SRCS += $(SDIR)/src/facepass_wrapper.cpp
#
MEG_VERSION = 1.2.1
FACEPASS_SDK_INSTALL_PATH ?=  $(PWD)/src/Meg_Lg_CV1835_FacePass_V$(MEG_VERSION)
ifeq ($(CONFIG_MW_SDK_VER_LIB32), y)
FACEPASS_SDK_LIB_PATH = $(FACEPASS_SDK_INSTALL_PATH)/lib/arm32
OPENCV_LIB_PATH += $(FACEPASS_SDK_INSTALL_PATH)/demo/thirdparty/image/share/arm32
else
FACEPASS_SDK_LIB_PATH = $(FACEPASS_SDK_INSTALL_PATH)/lib/arm64
OPENCV_LIB_PATH = $(FACEPASS_SDK_INSTALL_PATH)/demo/thirdparty/image/share/arm64
endif
FACEPASS_SDK_INC = $(FACEPASS_SDK_INSTALL_PATH)/include/
OPENCV_INC = $(FACEPASS_SDK_INSTALL_PATH)/demo/thirdparty/image/include
FACE_SDK_LIB += -L$(FACEPASS_SDK_LIB_PATH) -lfacesdk_cv1835
OPENCV_LIB += -L$(OPENCV_LIB_PATH) -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lopencv_calib3d -lopencv_features2d -lopencv_flann
INCS += -I$(FACEPASS_SDK_INC)
endif

ifeq ($(CONFIG_ALGORITHM_VENDOR_AISDK), y)
TARGET := libcvi_hal_face_aisdk.so
SRCS := $(SDIR)/src/cvi_hal_face_aisdk.cpp
ifeq ($(CONFIG_MW_SDK_VER_LIB32), y)
TPU_PREBUILT_PATH = $(TOP_DIR)/prepare/cvi_aisdk/$(shell echo $(CONFIG_CVI_SOC) | sed 's/\"//g')/32bit
endif
ifeq ($(CONFIG_MW_SDK_VER_LIB64), y)
TPU_PREBUILT_PATH = $(TOP_DIR)/prepare/cvi_aisdk/$(shell echo $(CONFIG_CVI_SOC) | sed 's/\"//g')/64bit
endif
ifeq ($(CONFIG_MW_SDK_VER_UCLIBC), y)
TPU_PREBUILT_PATH = $(TOP_DIR)/prepare/cvi_aisdk/$(shell echo $(CONFIG_CVI_SOC) | sed 's/\"//g')/uclibc
endif
ifeq ($(CONFIG_MW_SDK_VER_MUSL), y)
TPU_PREBUILT_PATH = $(TOP_DIR)/prepare/cvi_aisdk/$(shell echo $(CONFIG_CVI_SOC) | sed 's/\"//g')/musl_riscv64
endif
CVIMODEL_PREBUILT_PATH = $(TOP_DIR)/prepare/ai_models_output/mars
CVIMODEL_LIST = retinaface_mnet0.25_608_342.cvimodel
CVIMODEL_LIST += fqnet-v5_shufflenetv2-softmax.cvimodel
CVIMODEL_LIST += cviface-v5-s.cvimodel
CVIMODEL_LIST += liveness-rgb-ir.cvimodel

OPENCV_INC = $(TPU_PREBUILT_PATH)/cvitek_tpu_sdk/opencv/include
OPENCV_LIB = -L$(TPU_PREBUILT_PATH)/cvitek_tpu_sdk/opencv/lib -lopencv_core -lopencv_imgcodecs
FACE_SDK_LIB = -L$(TPU_PREBUILT_PATH)/cvitek_ai_sdk/lib
INCS += -I$(TPU_PREBUILT_PATH)/cvitek_ai_sdk/include/cviai
INCS += -I$(TPU_PREBUILT_PATH)/cvitek_ai_sdk/include/cviai/evaluation
INCS += -I$(TPU_PREBUILT_PATH)/cvitek_ai_sdk/include/cviai/core
INCS += -I$(TPU_PREBUILT_PATH)/cvitek_ai_sdk/include/cviai/core/face
INCS += -I$(TPU_PREBUILT_PATH)/cvitek_ai_sdk/include/cviai/service
LIBS += -lcviai -lcviai_app -lcviai_evaluation
LIBS += -Wl,--rpath-link $(TPU_PREBUILT_PATH)/cvitek_tpu_sdk/lib
LIBS += -Wl,--rpath-link $(TPU_PREBUILT_PATH)/cvitek_ai_sdk/lib
LIBS += -L$(SDK_PATH)/middleware/v2/lib -lcvi_ive
endif
#
TARGET_PATH := $(PWD)
##

##
INCS += -I$(SDIR)/include
INCS += -I$(SDIR)/src
INCS += -I$(TOP_DIR)/cpsl/db/include
INCS += -I$(KERNEL_INC)
OBJS := $(SRCS:%.cpp=%.o)
##


SQLITE3_LIB_PATH += $(TOP_DIR)/cpsl/db/sqlite3/$(SDK_VER)
LIBS += $(OPENCV_LIB) $(FACE_SDK_LIB) -ldl
LIBS += -L$(SQLITE3_LIB_PATH) -lsqlite3
LIBS += -L$(TOP_DIR)/cpsl/db
##
INCS += -I$(OPENCV_INC)
ifeq ($(CONFIG_MW_SDK_VER_MUSL), y)
INCS += -I$(SDK_PATH)/middleware/v2/include
else
INCS += -I$(SDK_PATH)/middleware/include
endif
INCS += -I$(TOP_DIR)/cpsl/db/include
INCS += -I$(TOP_DIR)/cpsl/db/sqlite3/include
INCS += -I$(TOP_DIR)/cpsl/hal/algorithm/include
##
LDFLAGS := -shared -fPIC -o
CXXFLAGS += -fPIC -Wall -Werror -O3 -std=gnu++11 -D_GNU_SOURCE -D_MIDDLEWARE_V2_ 
CXXFLAGS += -Wno-class-memaccess -Wno-format-overflow -Wno-stringop-truncation -Wno-stringop-overflow
##
.PHONY: all clean install
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX)  $(LDFLAGS) $(TARGET) $(OBJS) $(LIBS)

%.o: %.cpp
	$(CXX) -c $< -o $@ $(INCS) $(CXXFLAGS)

clean:
	rm -rf $(OBJS) $(TARGET)

install:
	cp -rpf $(TARGET) $(TOP_DIR)/customer/$(CUSTOMER)/install/lib
ifeq ($(CONFIG_ALGORITHM_VENDOR_MEGVII), y)
	cp -rpf $(FACEPASS_SDK_LIB_PATH)/*.so* $(TOP_DIR)/customer/$(CUSTOMER)/install/lib
	cp -rpf $(OPENCV_LIB_PATH)/*.so* $(TOP_DIR)/customer/$(CUSTOMER)/install/lib
	cp -rpf $(FACEPASS_SDK_INSTALL_PATH)/model/* $(TOP_DIR)/customer/$(CUSTOMER)/install/cvimodel
endif
ifeq ($(CONFIG_ALGORITHM_VENDOR_AISDK), y)
	cp -rpf $(TPU_PREBUILT_PATH)/cvitek_ai_sdk/lib/*.so* $(TOP_DIR)/customer/$(CUSTOMER)/install/lib
	cp -rpf $(TPU_PREBUILT_PATH)/cvitek_tpu_sdk/lib/*.so* $(TOP_DIR)/customer/$(CUSTOMER)/install/lib
	cp -rpf $(SDK_PATH)/middleware/v2/lib/libcvi_ive.so $(TOP_DIR)/customer/$(CUSTOMER)/install/lib
	@for model in $(CVIMODEL_LIST) ; do \
		if test -f $(CVIMODEL_PREBUILT_PATH)/$$model ; then \
			echo "$$model: install" ; \
			cp -rpf $(CVIMODEL_PREBUILT_PATH)/$$model $(TOP_DIR)/customer/$(CUSTOMER)/install/cvimodel/; \
		else \
			echo "$$model: not exist, please check!!" ; \
		fi \
	done;
endif
