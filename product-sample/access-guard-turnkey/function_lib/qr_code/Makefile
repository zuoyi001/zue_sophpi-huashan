SHELL = /bin/bash
ifeq ($(PARAM_FILE), )
	PARAM_FILE:=../../Makefile.param
	include $(PARAM_FILE)
endif

################# select sensor type for your sample ###############################
SENSOR0_TYPE ?= SONY_IMX327_MIPI_2M_30FPS_12BIT
SENSOR1_TYPE ?= SONY_IMX327_MIPI_2M_30FPS_12BIT

SNS_LIB = -lsns_full

COMMON_DIR ?= $(PWD)/../common

CFLAGS += -DSENSOR0_TYPE=$(SENSOR0_TYPE)
CFLAGS += -DSENSOR1_TYPE=$(SENSOR1_TYPE)

ifeq ($(DEBUG), 1)
CFLAGS += -g -O0
endif
ifeq ($(STATIC), 1)
#LDFLAGS += -static
#ELFFLAGS += -static
else
LDFLAGS += -shared
endif

#########################################################################
COMM_SRC := $(wildcard $(COMMON_DIR)/*.c)
COMM_OBJ := $(COMM_SRC:%.c=%.o)
COMM_INC := $(COMMON_DIR)
COMM_DEPS = $(COMM_SRC:.c=.d)
########################################################################

SDIR = $(PWD)
SRCS = $(wildcard $(SDIR)/*.c) $(wildcard $(SDIR)/src/*.c)
INCS = -I$(MW_INC) -I$(ISP_INC) -I../common/ -I$(KERNEL_INC) -I./src -I./inc
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

TARGET = zbar_demo
LIBS = -lsample -lvpu -lisp -lvenc -lvdec -lcvi_jpeg -lcvi_vcodec -lawb -lae -laf -lsys $(SNS_LIB) -L./lib -lzbar 
ifeq ($(MULTI_PROCESS_SUPPORT), 1)
DEFS += -DRPC_MULTI_PROCESS
endif

EXTRA_CFLAGS = $(INCS) $(DEFS)
EXTRA_LDFLAGS = $(LIBS) -pthread -lm -L$(MW_PATH)/lib32/3rd -lini -lnanomsg -lcvitracer
.PHONY : clean all
all : $(TARGET)

$(COMMON_DIR)/%.o: $(COMMON_DIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(SDIR)/%.o: $(SDIR)/%.c
	@$(CC) $(DEPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
	@echo [$(notdir $(CC))] $(notdir $@)

$(TARGET) : $(COMM_OBJ) $(OBJS) $(MW_LIB)/libisp.a $(MW_LIB)/libvpu.a $(MW_LIB)/libsys.a
	@$(CXX) -o $@ -Wl,--start-group $(OBJS) $(COMM_OBJS) $(MW_LIB)/libsys.a -Wl,--end-group $(ELFFLAGS) $(EXTRA_LDFLAGS)
	@echo -e $(BLUE)[LINK]$(END)[$(notdir $(CXX))] $(notdir $@)

clean:
	@rm -f $(OBJS) $(DEPS) $(COMM_OBJ) $(COMM_DEPS) $(TARGET)

-include $(DEPS)
