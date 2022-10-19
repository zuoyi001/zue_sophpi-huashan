-include $(TOP_DIR)/.config

PWD := $(shell pwd)
SDIR := $(PWD)
TARGET_A := libnfc.a
TARGET_SO := libnfc.so
TARGET_PATH := $(PWD)

SRCS = $(wildcard $(SDIR)/src/*.c)
INCS = -I$(SDIR)/include -I$(TOP_DIR)/include
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

CFLAGS += -Werror -fPIC -pthread -g -D__FILENAME__=\"$(notdir $<)\"
CFLAGS += -std=gnu99 -D_GNU_SOURCE
ARFLAGS = rcs
DEPFLAGS = -MMD
LDFLAGS = -shared -o

.PHONY : clean all install

all : $(TARGET_A) $(TARGET_SO)
 
$(SDIR)/%.o: $(SDIR)/%.c
	$(CC) $(DEPFLAGS)  $(CFLAGS) $(INCS) -o $@ -c $<
 
$(TARGET_A) : $(OBJS) 
	$(AR) $(ARFLAGS) $(TARGET_A) $(OBJS)
  
$(TARGET_SO) : $(OBJS) 
	$(CC) $(LDFLAGS) $(TARGET_PATH)/$@ $(OBJS) 
	$(STRIP) -s $(TARGET_PATH)/$@

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET_A) $(TARGET_SO)

install:
ifneq ($(CONFIG_STATIC_LINK), y)
	cp -rpf libnfc.so $(TOP_DIR)/customer/$(CUSTOMER)/install/lib
endif

-include $(DEPS) 
