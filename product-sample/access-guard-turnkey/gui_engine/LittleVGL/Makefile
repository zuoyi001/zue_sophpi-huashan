-include $(TOP_DIR)/.config
#
# Makefile
#
LVGL_DIR_NAME ?= lvgl
LVGL_DIR ?= ${shell pwd}
INCS ?= -I$(LVGL_DIR)/ -I$(TOP_DIR)/include/
CFLAGS ?= -Wall -Wshadow -Wundef -Wmaybe-uninitialized -O3 -g0
CFLAGS += -std=gnu99 -D_GNU_SOURCE
LDFLAGS ?= -pthread -shared
LIB_LVGL = liblvgl.so

include $(LVGL_DIR)/$(LVGL_DIR_NAME)/lvgl.mk
include $(LVGL_DIR)/lv_drivers/lv_drivers.mk
#include $(LVGL_DIR)/lv_examples/lv_examples.mk
include $(LVGL_DIR)/lv_lib/lv_lib.mk
include $(LVGL_DIR)/lv_font_res/lv_font_res.mk

APP_ROOT_DIR ?= ${shell pwd}

#Collect the files to compile
MAINSRC = ./lib_main.c 

OBJEXT ?= .o

AOBJS = $(ASRCS:.S=$(OBJEXT))
COBJS = $(CSRCS:.c=$(OBJEXT))

MAINOBJ = $(MAINSRC:.c=$(OBJEXT))
APP_OBJ = $(APP_SRC:.c=$(OBJEXT))

SRCS = $(ASRCS) $(CSRCS) $(MAINSRC) $(APP_SRC)
OBJS = $(AOBJS) $(COBJS)

.PHONY: default all clean install

all: default

%.o: %.c
	@$(CC) -fPIC -pthread -shared $(CFLAGS) $(INCS) -c $< -o $@
	@echo "CC $<"

default: $(MAINOBJ) $(AOBJS) $(COBJS)
	$(CC) $(LDFLAGS) -o $(LIB_LVGL) $(MAINOBJ) $(AOBJS) $(COBJS) 
	$(STRIP) -s $(LIB_LVGL)

clean: 
	rm -f $(AOBJS) $(COBJS) $(MAINOBJ) $(LIB_LVGL)

install:
	cp -rpf liblvgl.so $(TOP_DIR)/customer/$(CUSTOMER)/install/lib
