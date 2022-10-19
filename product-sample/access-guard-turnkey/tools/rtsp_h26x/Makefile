CC := arm-linux-gnueabihf-gcc
CXX := arm-linux-gnueabihf-g++
STRIP = arm-linux-gnueabihf-strip

SRC_C = rtsp_h26x.c
SRC_C += ../../application/resources/app_rtsp.c

OBJS := $(SRC_C:%.c=%.o)
OBJS += $(SRC_CPP:%.cpp=%.o)

TARGET = rtsp_h26x.bin

INCLUDES = -I../../../middleware/include/
INCLUDES += -I../../../middleware/sample/common
INCLUDES += -I../../../middleware/include/isp/cv183x
INCLUDES += -I../../application/resources
INCLUDES += -I../../function_lib/3rd_prebuilt/cvi_rtsp/include
CFLAGS = -g -O0 -fPIC
CFLAGS += -std=gnu99 -D_GNU_SOURCE

LIB_DIR = -L../../../middleware/lib -L../../../middleware/lib/3rd 
LIB_DIR += -L../../function_lib/3rd_prebuilt/cvi_rtsp/lib32

LIBS =  -lvpu -lnanomsg -lsys -lvenc -lcvi_jpeg -lcvi_vcodec -lcvitracer -lsample
LIBS += -lvdec -lcvi_rtsp
LIBS += -lawb -lae -laf -lisp -lsns_full -lini 
LIBS += -lm -lstdc++ -pthread

.PHONY: default clean install
all: default


default: $(OBJS)
	$(CC) -fPIC -o $(TARGET) $(OBJS) $(LIB_DIR) $(LIBS)
%.o: %.c $(DEPS)
	$(CC) -c $< -o $@ $(INCLUDES) $(CFLAGS)
clean:
	rm -rf $(OBJS) $(TARGET)