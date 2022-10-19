include $(SRCTREE)/build/base.mk

screen-y += comm/src
screen-y += mipidsi/src
screen-$(CONFIG_SCREEN_HX8394)              += hx8394_mipi
screen-$(CONFIG_SCREEN_OTA5182)             += ota5182
screen-$(CONFIG_SCREEN_ST7796S)             += st7796s
screen-$(CONFIG_SCREEN_ST7789)              += st7789
screen-$(CONFIG_SCREEN_ST7796S_RGB16BIT)    += st7796s_rgb16bit
screen-$(CONFIG_SCREEN_OTA7290B)            += ota7290b
screen-$(CONFIG_SCREEN_ILI8961)             += ili8961
screen-$(CONFIG_SCREEN_OTA7290B_MIPI11INCH) += ota7290b_mipi11inch
screen-$(CONFIG_SCREEN_ICNL9707)            += icnl9707

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
TARGET_A	:= $(TARGET_DIR)/libcvi_hal_screen.a
TARGET_SO	:= $(TARGET_DIR)/libcvi_hal_screen.so



INCS		+= -I$(CURDIR)/../gpio/include
LIBS		+= -L$(CURDIR)/../gpio/lib -lcvi_hal_gpio
INCS		+= -I$(CURDIR)/../pwm/include
LIBS		+= -L$(CURDIR)/../pwm/lib -lcvi_hal_pwm
INCS        += -I$(CURDIR) -I$(CURDIR)/../include
INCS		+= -I$(CURDIR)/comm/include
INCS		+= -I$(CURDIR)/mipidsi/include
INCS		+= -I$(SRCTREE)/cpsl/mmf/include

CFLAGS      += $(INCS)
CFLAGS      += -fPIC

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
