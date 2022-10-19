


SDIR = $(PWD)

SUBDIR = src test


SRCS = $(wildcard $(SDIR)/*.c $(INIH_PATH)/*.c)
ifeq ($(SDK_VER), lib32)
INCS = -I$(SDIR)/sqlite3_32bit/include
else
INCS = -I$(SDIR)/sqlite3/include
endif
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)

ifeq ($(SDK_VER), lib32)
LIB_PATH = $(SDIR)/sqlite3_32bit/lib
else
LIB_PATH = $(SDIR)/sqlite3/lib
endif

ifeq ($(SDK_VER), lib32)
SQLITE3_LIB_DIR = sqlite3_32bit
else
SQLITE3_LIB_DIR = sqlite3
endif

LIBS = -L$(LIB_PATH) -lsqlite3

CFLAGS = -lm


CFLAGS += -Werror -fPIC -pthread -g -D__FILENAME__=\"$(notdir $<)\"
CFLAGS += -std=gnu99 -D_GNU_SOURCE
DEPFLAGS = -MMD

all : test

$(SUBDIR):
	$(MAKE) -C $@ $(MAKECMDGOALS)

test: src

$(SDIR)/%.o: $(SDIR)/%.c
	$(CC) $(DEPFLAGS)  $(CFLAGS) $(INCS) $(LIBS) -o $@ -c $<
	$(STRIP) -s $@




.PHONY : clean $(SUBDIR)
clean: $(SUBDIR)
	rm -f $(OBJS) $(DEPS) $(TARGET)

install:
	cp -rpf $(SQLITE3_LIB_DIR)/lib/libsqlite3.so* $(TOP_DIR)/customer/$(CUSTOMER)/install/lib
	cp -rpf src/libcvi_hal_db.so $(TOP_DIR)/customer/$(CUSTOMER)/install/lib


-include $(DEPS)
