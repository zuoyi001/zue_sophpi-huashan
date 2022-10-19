-include $(TOP_DIR)/.config

PWD := $(shell pwd)
SDIR := $(PWD)
TARGET_A := libnet.a
TARGET_SO := libnet.so
TARGET_PATH := $(PWD)

SRCS = $(wildcard $(SDIR)/*.c)
INCS = -I$(SDIR)
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

LIB_PATH = $(SDIR)

CFLAGS += -Werror -fPIC -pthread -g -D__FILENAME__=\"$(notdir $<)\"
CFLAGS += -std=gnu99 -D_GNU_SOURCE
ARFLAGS = rcs
DEPFLAGS = -MMD
LDFLAGS = -shared -o

all : $(TARGET_A) $(TARGET_SO)
 
$(SDIR)/%.o: $(SDIR)/%.c
	$(CC) $(DEPFLAGS)  $(CFLAGS) $(INCS) -o $@ -c $<
 
$(TARGET_A) : $(OBJS) 
	$(AR) $(ARFLAGS) $(TARGET_A) $(OBJS)
 
$(TARGET) : $(OBJS) 
	$(CC) -shared -o $(TARGET_PATH)/$@ $(OBJS) 
$(TARGET_SO) : $(OBJS) 
	$(CC) $(LDFLAGS) $(TARGET_PATH)/$@ $(OBJS) 
	$(STRIP) -s $(TARGET_PATH)/$@

.PHONY : clean install
clean:
	rm -f $(OBJS) $(DEPS) $(TARGET_A) $(TARGET_SO)
install:
ifneq ($(CONFIG_STATIC_LINK), y)
	cp -rpf libnet.so $(TOP_DIR)/customer/$(CUSTOMER)/install/lib
endif

-include $(DEPS)
