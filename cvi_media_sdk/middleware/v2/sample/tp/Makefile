SHELL = /bin/bash

ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../Makefile.param
	include $(PARAM_FILE)
endif

CUR_DIR     := $(shell pwd)
TOP_DIR     := $(CUR_DIR)/../../..

SRC_DIR     := $(CUR_DIR)
INC_DIR     := $(CUR_DIR)

INSTALL_DIR := $(TOP_DIR)/install

DBG_DIR     := $(TOP_DIR)/src/util/dbg

SRCS         = $(wildcard $(SRC_DIR)/*.c)
OBJS         = $(SRCS:.c=.o)
DEPS         = $(SRCS:.c=.d)
INCS         = -I$(INC_DIR) -I$(DBG_DIR)
LIBS		 =

TARGET_A     = libtp.a
TARGET_SO    = libtp.so
TARGET       =  tp
LDFLAGS		+=	-shared

CFLAGS      +=	-Werror
CFLAGS      +=	-ffunction-sections -fdata-sections

CFLAGS		+= $(INCS)

.PHONY : clean all install
all : $(TARGET_A) $(TARGET_SO) $(TARGET)

%.o : %.c
	$(CC) $(CFLAGS) -o $@ -c $<

$(TARGET): $(OBJS)
	@$(CXX) -o $@ $(OBJS) $(ELFFLAGS) $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CC))] $(notdir $@)


$(TARGET_A) : $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

# to do test
# $(AR) $(ARFLAGS) $@ $^	# $(OBJS) instead of $< ???

$(TARGET_SO) : $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

# for backup mark
# $(LD) $(LDFLAGS) -o $@ --start-group $(OBJS) $(LIBS) --end-group	# for loop dependent

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET_A) $(TARGET_SO) $(TARGET)

install:
	mkdir -p $(INSTALL_DIR)
	mkdir -p $(INSTALL_DIR)/lib
	mkdir -p $(INSTALL_DIR)/include
	cp $(TARGET_A) $(INSTALL_DIR)/lib
	cp $(TARGET_SO) $(INSTALL_DIR)/lib
	cp $(INC_DIR)/*.h $(INSTALL_DIR)/include

-include $(DEPS)
