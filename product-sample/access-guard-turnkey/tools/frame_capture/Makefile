CC := arm-linux-gnueabihf-gcc
CXX := arm-linux-gnueabihf-g++
STRIP = arm-linux-gnueabihf-strip

SRC_C = frame_vi_vpss_dump.c

OBJS := $(SRC_C:%.c=%.o)
OBJS += $(SRC_CPP:%.cpp=%.o)

TARGET = frame_vi_vpss_dump.bin

INCLUDES = -I../../../middleware/include/
CFLAGS = -g -O0 -fPIC
CFLAGS += -std=gnu99 -D_GNU_SOURCE

LIB_DIR = -L../../../middleware/lib -L../../../middleware/lib/3rd 
LIBS =  -lvpu -lm -lstdc++ -lnanomsg -lsys -lvenc -lcvi_jpeg -lcvi_vcodec -lvdec -lcvitracer

.PHONY: default clean install
all: default


default: $(OBJS)
	$(CC) -fPIC -o $(TARGET) $(OBJS) $(LIB_DIR) $(LIBS)
%.o: %.c $(DEPS)
	$(CC) -c $< -o $@ $(INCLUDES) $(CFLAGS)
clean:
	rm -rf $(OBJS) $(TARGET)