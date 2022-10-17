CROSS_COMPILE ?= riscv64-unknown-linux-musl-
CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
AR = $(CROSS_COMPILE)ar

CUR_DIR := $(shell pwd)
SDIR = $(CUR_DIR)

TOP_DIR     := $(CUR_DIR)/../..
INSTALL_DIR ?= $(TOP_DIR)/install

SRCS = $(wildcard $(CUR_DIR)/*.c)
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

TARGET = sample_awtk

############################################
# Dependencies
############################################
UI_DIR    	:= $(TOP_DIR)/ui/cardv
UI_INC		:= -I$(UI_DIR)/include
UI_LIB		:= $(UI_DIR)/lib
INCS        += $(UI_INC)
LIBS        += -L$(UI_LIB) -lcviapp

TOUCHPADLIB := $(TOP_DIR)/touchpad/lib
LIBS        += -L$(TOUCHPADLIB) -lcvi_hal_touchpad

AWTK_DIR    := $(TOP_DIR)/awtkcode
AWTI_INC	:= -I$(AWTK_DIR)/awtk/src/
AWTI_INC	+= -I$(AWTK_DIR)/awtk/src/ext_widgets
AWTK_LIB	:= $(AWTK_DIR)/awtk-linux-fb/build/lib
INCS        += $(AWTI_INC)
LIBS        += -L$(AWTK_LIB) -lawtk
CONFIG_FLAG += -DUSE_GUI_AWTK

############################################
# End of Dependencies
############################################

DEPFLAGS     = -MMD
# CFLAGS      += -Wall -Wextra -Werror -pedantic-errors -std=c99 -D_GNU_SOURCE
CFLAGS      += -Wall -Wextra -std=c99 -D_GNU_SOURCE -march=rv64imafdcvxthead -mcmodel=medany -mabi=lp64d
CFLAGS      += -O3
CFLAGS      += $(DEPFLAGS)
CFLAGS      += $(CONFIG_FLAG)
# LDFLAGS     = -static -rdynamic
LDFLAGS     = -march=rv64imafdcvxthead -mcmodel=medany -mabi=lp64d

all : $(TARGET)

$(SDIR)/%.o: $(SDIR)/%.c
	$(CC) $(CFLAGS) $(INCS) -o $@ -c $<

$(TARGET) : $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

.PHONY : clean install
clean:
	rm -f $(OBJS) $(DEPS) $(TARGET)

install:
	mkdir -p $(INSTALL_DIR)/bin
	cp $(TARGET) $(INSTALL_DIR)/bin

-include $(DEPS)
