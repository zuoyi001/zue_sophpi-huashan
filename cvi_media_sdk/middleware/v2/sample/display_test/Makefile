SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../Makefile.param
	include $(PARAM_FILE)
endif
include ../sample.mk

SDIR = $(PWD)
SRCS = $(wildcard $(SDIR)/*.c) $(wildcard $(SDIR)/src/*.c)
INCS = -I$(MW_INC) -I$(ISP_INC) -I../common/ -I$(KERNEL_INC) -I./inc
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

PKG_CONFIG_PATH = $(MW_PATH)/pkgconfig
REQUIRES = cvi_common cvi_vdec cvi_misc
MW_LIBS = $(shell PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config --libs --define-variable=mw_dir=$(MW_PATH) $(REQUIRES))

TARGET_A = $(MW_LIB)/lib_lt9611.a
TARGET_SO = $(MW_LIB)/lib_lt9611.so

TARGET_EXE = lt9611_test

LIBS = $(MW_LIBS)
ifeq ($(MULTI_PROCESS_SUPPORT), 1)
DEFS += -DRPC_MULTI_PROCESS
LIBS += -lnanomsg
endif


EXTRA_CFLAGS = $(INCS) $(DEFS)
EXTRA_LDFLAGS = $(LIBS) -lpthread -lm -lini

.PHONY : clean all
all: $(TARGET_A) $(TARGET_SO) $(TARGET_EXE)

#$(COMMON_DIR)/%.o: $(COMMON_DIR)/%.c
#	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
#	@echo [$(notdir $(CC))] $(notdir $@)

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET_A): $(OBJS)
	@$(AR) $(ARFLAGS) $(TARGET_A) $(OBJS)
	@echo -e $(YELLOW)[LINK]$(END)[$(notdir $(AR))] $(notdir $(TARGET_A))

$(TARGET_SO): $(OBJS)
	@$(LD) $(LDFLAGS) $(EXTRA_LDFLAGS) -o $@ --start-group $(OBJS) --end-group
	@echo -e $(GREEN)[LINK]$(END)[$(notdir $(LD))] $(notdir $(TARGET_SO))

$(TARGET_EXE): $(OBJS)
	@$(CXX) -o $@ -Wl,--start-group $(OBJS)  $(MW_LIB)/libmipi_tx.a -Wl,--end-group $(ELFFLAGS) $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)

clean:
	@rm -f $(OBJS) $(DEPS) $(TARGET_A) $(TARGET_SO) $(TARGET_EXE)
# $(COMM_OBJ) $(COMM_DEPS)

-include $(DEPS)
