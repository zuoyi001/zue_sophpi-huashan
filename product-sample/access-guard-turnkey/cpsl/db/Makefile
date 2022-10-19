-include $(TOP_DIR)/.config

PWD := $(shell pwd)
SDIR := $(PWD)
TARGET_A :=  libcvi_hal_db.a
TARGET_SO :=  libcvi_hal_db.so
TARGET_PATH := $(PWD)

SRCS = $(wildcard $(SDIR)/src/*.c)
INCS = -I$(SDIR)/src 
INCS += -I$(SDIR)/include
INCS +=  -I$(TOP_DIR)/cpsl/db/sqlite3/include/
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

ifeq ($(CONFIG_STATIC_LINK), y)
ELFFLAGS += -Wl,-Bstatic
else
ELFFLAGS += -Wl,-Bdynamic
endif

ifeq ($(SDK_VER), lib32)
LIB_PATH = -L$(TOP_DIR)/cpsl/db/sqlite3/lib32
endif
ifeq ($(SDK_VER), lib64)
LIB_PATH = -L$(TOP_DIR)/cpsl/db/sqlite3/lib64
endif
ifeq ($(SDK_VER), uclibc)
LIB_PATH = -L$(TOP_DIR)/cpsl/db/sqlite3/uclibc
endif
ifeq ($(SDK_VER), musl_riscv64)
LIB_PATH = -L$(TOP_DIR)/cpsl/db/sqlite3/musl_riscv64
endif
LIBS = $(ELFFLAGS) -lsqlite3 $(ELFFLAGS)

CFLAGS += -Werror -fPIC -pthread -g -D__FILENAME__=\"$(notdir $<)\"
CFLAGS += -std=gnu99 -D_GNU_SOURCE
ARFLAGS = rcs
DEPFLAGS = -MMD
LDFLAGS = -shared -o

all : $(TARGET_A) $(TARGET_SO)
 
$(SDIR)/src/%.o: $(SDIR)/src/%.c
	$(CC) $(DEPFLAGS)  $(CFLAGS) $(INCS) $(LIB_PATH) $(LIBS) -o $@ -c $<
 
$(TARGET_A) : $(OBJS) 
	$(AR) $(ARFLAGS) $(TARGET_A) $(OBJS)
 
$(TARGET_SO) : $(OBJS) 
	$(CC) $(LDFLAGS) $(TARGET_PATH)/$@ $(OBJS) 
	$(STRIP) -s $(TARGET_PATH)/$@

.PHONY : clean install
clean:
	rm -f $(OBJS) $(DEPS) $(TARGET_A) $(TARGET_SO)

install:
	cp -rpf libcvi_hal_db.so $(TOP_DIR)/customer/$(CUSTOMER)/install/lib
	cp -rpf sqlite3/$(SDK_VER)/*.so* $(TOP_DIR)/customer/$(CUSTOMER)/install/lib

-include $(DEPS)