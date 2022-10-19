#
PWD := $(shell pwd)
#
TARGET = libface.so
TARGET_PATH := $(PWD)

SDIR := $(PWD)
SRCS := $(wildcard $(SDIR)/*.cpp)
SRCS += $(wildcard $(SDIR)/cvi_feature_matching/*.cpp)
INCS := -I$(SDIR)
OBJS := $(SRCS:%.cpp=%.o)

LDFLAGS := -shared -o

ifeq ($(SDK_VER), lib32)
TPU_SDK_INSTALL_PATH ?= $(SDK_INSTALL_PATH)/tpu/cvitek_tpu_sdk
else
TPU_SDK_INSTALL_PATH ?= $(SDK_INSTALL_PATH)/tpu_64/cvitek_tpu_sdk
endif

TPU_INC = $(TPU_SDK_INSTALL_PATH)/include
TPU_LIB =  -L$(TPU_SDK_INSTALL_PATH)/lib -lcvikernel -lcvimath
OPENCV_LIB = -L$(TPU_SDK_INSTALL_PATH)/opencv/lib -lopencv_core -lopencv_imgcodecs -lopencv_imgproc
OPENCV_INC = $(TPU_SDK_INSTALL_PATH)/opencv/include


#LIBS += $(TPU_LIB) $(OPENCV_LIB) 

INCS += -I$(SDK_PATH)/middleware/include/
INCS += -I$(SDK_PATH)/middleware/include/isp/cv183x
INCS += -I$(TPU_SDK_INSTALL_PATH)/include
INCS += -I$(TPU_SDK_INSTALL_PATH)/opencv/include
INCS += -I$(PWD)/../db/include
INCS += -I$(PWD)/../db/sqlite3/include
#INCS += -I$(PWD)/../app/3rdparty/tracer
#INCS += -I$(SDIR)/cviai/include
#INCS += -I$(AI_SDK_INC)
INCS += -I$(SDIR)/cvi_feature_matching

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) $(TARGET) $(OBJS) $(LIBS)

%.o: %.cpp
	$(CXX) -O3 -fPIC -c $< -o $@ $(INCS) $(CFLAGS)

.PHONY: clean all
clean:
	rm -rf $(OBJS) $(TARGET)
