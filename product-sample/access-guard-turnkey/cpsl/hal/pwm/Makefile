#
-include $(TOP_DIR)/.config
#
TARGET_A := libcvi_hal_pwm.a
TARGET_SO := libcvi_hal_pwm.so
TARGET_PATH := $(shell pwd)
#
SRCS_C +=  src/cvi_hal_pwm.c
#
INCS = -I./include
INCS += -I$(TOP_DIR)/include
#
OBJS = $(SRCS_C:.c=.o)
DEPS = $(SRCS_C:.c=.d)
#
CFLAGS += -Werror -fPIC -pthread -g -D__FILENAME__=\"$(notdir $<)\"
CFLAGS += -std=gnu99 -Wno-unused-result -D_GNU_SOURCE
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
