CROSS_COMPILE ?= riscv64-unknown-linux-musl-
CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
AR = $(CROSS_COMPILE)ar

CUR_DIR     := $(shell pwd)
TOP_DIR     := $(CUR_DIR)/../..
INSTALL_DIR ?= $(TOP_DIR)/install

SRC_DIR     := $(CUR_DIR)/src
AWTKPORT_DIR := $(CUR_DIR)/../awtk-port
INCS     	:= -I$(CUR_DIR)/include

AWTK_DIR    := $(TOP_DIR)/awtkcode
INCS 	    += -I$(AWTK_DIR)/awtk/src
INCS        += -I$(AWTK_DIR)/awtk/src/ext_widgets
INCS 	    += -I$(AWTKPORT_DIR)

TOUCHPAD_DIR:= $(TOP_DIR)/touchpad
INCS        += -I$(TOUCHPAD_DIR)/include

LIB_DIR     := $(CUR_DIR)/lib
OBJ_DIR     := $(CUR_DIR)/obj

RESRAWDIR   := customer

SRCS         = $(wildcard $(SRC_DIR)/*.c)
AWTKPORT     = $(wildcard $(AWTKPORT_DIR)/*.c)
OBJS 	     = $(AWTKPORT:$(AWTKPORT_DIR)/%.c=$(OBJ_DIR)/%.o)
OBJS        += $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEPS 	     = $(AWTKPORT:$(AWTKPORT_DIR)/%.c=$(OBJ_DIR)/%.d)
DEPS        += $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.d)
CONFIG_FLAG += -DHAS_STDIO

TARGET       = $(LIB_DIR)/libcviapp.a
TARGET_SO    = $(LIB_DIR)/libcviapp.so

############################################
# End of Dependencies
############################################

LDFLAGS_SO   = -shared
DEPFLAGS     = -MMD
CFLAGS      += $(DEPFLAGS)  -march=rv64imafdcvxthead -mcmodel=medany -mabi=lp64d
CFLAGS      += -fPIC
CFLAGS 	    += -Wall -Os -std=gnu99
CFLAGS      += $(CONFIG_FLAG)

.PHONY : clean all install
all : $(TARGET) $(TARGET_SO)

$(OBJ_DIR)/%.o: $(AWTKPORT_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCS) -o $@ -c $<

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCS) -o $@ -c $<

$(TARGET) : $(OBJS)
	mkdir -p $(LIB_DIR)
	$(AR) $(ARFLAGS) -o $@ $^

$(TARGET_SO) : $(OBJS)
	mkdir -p $(LIB_DIR)
	$(CC) $(LDFLAGS_SO) -o $@ $^

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET) $(TARGET_SO)

install:
	mkdir -p $(INSTALL_DIR)/lib
	mkdir -p $(INSTALL_DIR)/include
	mkdir -p $(INSTALL_DIR)/bin
	cd $(CUR_DIR)/cardvres/;./xml2bin.sh $(RESRAWDIR);cd -
	cp $(TARGET) $(INSTALL_DIR)/lib
	cp $(TARGET_SO) $(INSTALL_DIR)/lib
	cp $(CUR_DIR)/include/* $(INSTALL_DIR)/include
	cp $(CUR_DIR)/res $(INSTALL_DIR)/bin/ -a

-include $(DEPS)
