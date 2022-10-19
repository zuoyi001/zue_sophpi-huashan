include $(SRCTREE)/build/base.mk

screen-y += comm/src
screen-$(CONFIG_SNS1_PIXELPLUS_PR2020)              += pixelplus_pr2020

SEXT := c
SDIR := $(CURDIR)
ODIR := $(patsubst $(SRCTREE)/%,$(OBJTREE)/%,$(SDIR))
SRCS := $(foreach dir,$(screen-y),$(wildcard $(dir)/*.$(SEXT)))
OBJS := $(addprefix $(ODIR)/,$(SRCS:.c=.o))
DEPS := $(addprefix $(ODIR)/,$(SRCS:.c=.d))
# $(info SDIR = $(SDIR))
# $(info ODIR = $(ODIR))
# $(info SRCS = $(SRCS))
# $(info OBJS = $(OBJS))

TARGET_DIR	:= $(CURDIR)/lib
TARGET_A	:= $(TARGET_DIR)/libcvi_hal_ahd.a
TARGET_SO	:= $(TARGET_DIR)/libcvi_hal_ahd.so

LOG_DIR     := $(SRCTREE)/cpsl/common/log
INCS        += -I$(LOG_DIR)/include

HAL_DIR     := $(SRCTREE)/cpsl/hal
INCS        += -I$(HAL_DIR)/include

UTILS_DIR   := $(SRCTREE)/applications/common/utils
INCS        += -I$(UTILS_DIR)/include

INCS		+= -I$(CURDIR) -I$(CURDIR)/../include
INCS		+= -I$(CURDIR)/comm/include
INCS		+= -I$(SRCTREE)/cpsl/mmf/include

CFLAGS		+= $(INCS)
CFLAGS		+= -fPIC

.PHONY : clean all install
all : $(TARGET_A) $(TARGET_SO)

$(ODIR)/%.o : $(SDIR)/%.$(SEXT)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ -c $<

$(TARGET_DIR) :
	mkdir -p $@

$(TARGET_A) : $(OBJS) | $(TARGET_DIR)
	$(AR) $(ARFLAGS) $@ $^

$(TARGET_SO) : $(OBJS) | $(TARGET_DIR)
	$(CC) $(LDFLAGS_SO) -o $@ $^

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET_A) $(TARGET_SO)

$(INSTALL_PATH)/lib :
	mkdir -p $@

install: $(TARGET_A) $(TARGET_SO) | $(INSTALL_PATH)/lib
	cp $^ $(INSTALL_PATH)/lib

-include $(DEPS)
