#
-include $(TOP_DIR)/.config
#
TARGET_A := libawtk-port.a
TARGET_SO := libawtk-port.so
TARGET_PATH := $(shell pwd)
#
SRCS_C =  input_dispatcher.c
SRCS_C += input_thread.c
SRCS_C += lcd_linux_drm.c
SRCS_C += lcd_linux_fb.c
SRCS_C += lcd_mem_others.c
SRCS_C += main_loop_linux.c
SRCS_C += mouse_thread.c
SRCS_C += tslib_thread.c
#
INCS = -I$./ -I./include
INCS += -I$(TOP_DIR)/cpsl/hal/touchpad/include
INCS += -I../awtk/src
INCS += -I../awtk/src/ext_widgets
#
OBJS = $(SRCS_C:.c=.o)
DEPS = $(SRCS_C:.c=.d)
#
CFLAGS += -Werror -fPIC -pthread -g -D__FILENAME__=\"$(notdir $<)\"
CFLAGS += -std=gnu99 -D_GNU_SOURCE
ARFLAGS = rcs
DEPFLAGS = -MMD
LDFLAGS = -shared -o
#
.PHONY : all clean install
all : $(TARGET_A) $(TARGET_SO)
#
%.o: %.c
	$(CC) $(DEPFLAGS)  $(CFLAGS) $(INCS) -o $@ -c $<
#
$(TARGET_A) : $(OBJS) 
	$(AR) $(ARFLAGS) $(TARGET_PATH)/$@ $(OBJS)
#
$(TARGET_SO) : $(OBJS) 
	$(CC) $(LDFLAGS) $(TARGET_PATH)/$@ $(OBJS) 
	$(STRIP) -s $(TARGET_PATH)/$@
#
clean:
	rm -f $(OBJS) $(DEPS) $(TARGET_A) $(TARGET_SO)
#
install:
ifneq ($(CONFIG_STATIC_LINK), y)
	cp -rpf $(TARGET_SO) $(TOP_DIR)/customer/$(CUSTOMER)/install/lib
endif
#
-include $(DEPS)
#
