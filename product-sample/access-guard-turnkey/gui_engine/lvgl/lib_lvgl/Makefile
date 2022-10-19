#
# Makefile
#
#CC := arm-linux-gnueabihf-gcc
LVGL_DIR_NAME := lvgl
LVGL_DIR := ${shell pwd}/../release-v8.0
INCS := -I$(LVGL_DIR) -I./ -I$(LVGL_DIR)/../lv_libs -I$(TOP_DIR)/include
CFLAGS := -O3 -g0 -I$(LVGL_DIR)/ -Wall -Wshadow -Wundef -Wmissing-prototypes -Wno-discarded-qualifiers -Wall -Wextra -Wno-unused-function -Wno-error=strict-prototypes -Wpointer-arith -fno-strict-aliasing -Wno-error=cpp -Wuninitialized -Wmaybe-uninitialized -Wno-unused-parameter -Wno-missing-field-initializers -Wtype-limits -Wsizeof-pointer-memaccess -Wno-format-nonliteral -Wno-cast-qual -Wunreachable-code -Wno-switch-default -Wreturn-type -Wmultichar -Wformat-security -Wno-ignored-qualifiers -Wno-error=pedantic -Wno-sign-compare -Wno-error=missing-prototypes -Wdouble-promotion -Wclobbered -Wdeprecated -Wempty-body -Wtype-limits -Wshift-negative-value -Wstack-usage=2048 -Wno-unused-value -Wno-unused-parameter -Wno-missing-field-initializers -Wuninitialized -Wmaybe-uninitialized -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wtype-limits -Wsizeof-pointer-memaccess -Wno-format-nonliteral -Wpointer-arith -Wno-cast-qual -Wmissing-prototypes -Wunreachable-code -Wno-switch-default -Wreturn-type -Wmultichar -Wno-discarded-qualifiers -Wformat-security -Wno-ignored-qualifiers -Wno-sign-compare
CFLAGS += -std=gnu99 -D_GNU_SOURCE -DLV_CONF_INCLUDE_SIMPLE -DLV_DEMO_CONF_INCLUDE_SIMPLE -fPIC
LDFLAGS := -lm -pthread -shared
LIB_LVGL = libgui_lvgl.so


#Collect the files to compile
MAINSRC = ./app_ui.c

include $(LVGL_DIR)/lvgl/lvgl.mk
include $(LVGL_DIR)/lv_drivers/lv_drivers.mk
include $(LVGL_DIR)/lv_demos/lv_demo.mk
include $(LVGL_DIR)/../lv_libs/lv_lib_png/lv_lib_png.mk
include $(LVGL_DIR)/../lv_font_res/lv_font_res.mk

#$(info wangliang=$(CSRCS))


OBJEXT ?= .o

AOBJS = $(ASRCS:.S=$(OBJEXT))
COBJS = $(CSRCS:.c=$(OBJEXT))

MAINOBJ = $(MAINSRC:.c=$(OBJEXT))

SRCS = $(ASRCS) $(CSRCS) $(MAINSRC)
OBJS = $(AOBJS) $(COBJS)

## MAINOBJ -> OBJFILES

all: default

%.o: %.c
	@$(CC)  $(CFLAGS) $(INCS) -c $< -o $@
	@echo "CC $<"
    
default: $(AOBJS) $(COBJS) $(MAINOBJ)
	$(CC) -o $(LIB_LVGL) $(MAINOBJ) $(AOBJS) $(COBJS) $(LDFLAGS)

clean: 
	rm -f $(LIB_LVGL) $(AOBJS) $(COBJS) $(MAINOBJ)

install:
	cp -rpf libgui_lvgl.so $(TOP_DIR)/customer/$(CUSTOMER)/install/lib

