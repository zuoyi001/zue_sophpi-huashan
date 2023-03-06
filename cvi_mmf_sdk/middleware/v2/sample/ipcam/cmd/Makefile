SHELL = /bin/bash

ifeq ($(PARAM_FILE), )
	PARAM_FILE := ../../../Makefile.param
	include $(PARAM_FILE)
endif

STATIC = 0
ifeq ($(STATIC), 1)
ELFFLAGS += -static
endif

APP_ROOT        := ..
APP_CMD_DIR     := $(APP_ROOT)/cmd
APP_COMMON_DIR  := $(APP_ROOT)/common
APP_INSTALL_DIR := $(APP_ROOT)/install

SEXT	:= c

LIBS_SO = -lpthread -lm

# ipcam source		S
# ===============================================================
# cmd
SDIR	+= $(APP_CMD_DIR)
INCS	+= -I$(APP_CMD_DIR)

# common 
SDIR	+= $(APP_COMMON_DIR)
INCS	+= -I$(APP_COMMON_DIR)

# SRCS ?= $(wildcard $(SDIR)/*.c)
SRCS ?= $(shell find $(SDIR) -type f -name "*.$(SEXT)")
OBJS = $(patsubst %.c, %.o, $(SRCS))
DEPS = $(SRCS:.c=.d)
# ipcam source		E
# ===============================================================

# End of source and header file 
# ===============================================================

CFLAGS += $(DEFS)
CFLAGS += -Wno-unused-parameter -D_GNU_SOURCE

# debug
# ===============================================================
# $(info "OBJS=$(OBJS)")
# ===============================================================

# TARGET cmdTest  : test ipcam command
# ===============================================================
TARGET = cmdTest
# ===============================================================

.PHONY : clean all install

all: $(TARGET)

$(OBJS): %.o : %.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(INCS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET) : $(OBJS)
	@$(CXX) -o $@ $(OBJS) $(LIBS_SO) $(ELFFLAGS) $(CFLAGS)
	@$(STRIP) $@
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)

clean:
	$(shell if [ -d $(APP_INSTALL_DIR) ]; then rm -rf $(APP_INSTALL_DIR); fi)
	@rm -f $(OBJS) $(DEPS) $(TARGET)

install:
	$(shell if [ ! -d $(APP_INSTALL_DIR) ]; then mkdir $(APP_INSTALL_DIR); fi)
	@cp -f $(TARGET) $(APP_INSTALL_DIR)

-include $(DEPS)
