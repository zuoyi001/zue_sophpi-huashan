-include $(TOP_DIR)/.config

PWD := $(shell pwd)
SDIR := $(PWD)
TARGET_A := libcviconfig.a
TARGET_SO := libcviconfig.so
TARGET_PATH := $(PWD)

SRCS = $(wildcard $(SDIR)/src/*.cpp)
INCS = -I$(PWD)/include -I$(PWD)/json_include
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

LIB_PATH = $(SDIR)


CXXFLAGS += -Werror -fPIC -pthread -g -D__FILENAME__=\"$(notdir $<)\"
ARFLAGS = rcs
DEPFLAGS = -MMD
LDFLAGS = -shared -o

.PHONY: all clean

all: $(TARGET_A) $(TARGET_SO)

$(SDIR)/src/%.o: $(SDIR)/src/%.cpp
	$(CXX) $(DEPFLAGS)  $(CXXFLAGS) $(INCS) -o $@ -c $<
 
$(TARGET_A) : $(OBJS) 
	$(AR) $(ARFLAGS) $(TARGET_A) $(OBJS)

$(TARGET_SO) : $(OBJS) 
	$(CXX) $(LDFLAGS) $(TARGET_PATH)/$@ $(OBJS) 
	$(STRIP) -s $(TARGET_PATH)/$@

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET_A) $(TARGET_SO)

install:
ifneq ($(CONFIG_STATIC_LINK), y)
	cp -rpf libcviconfig.so $(TOP_DIR)/customer/$(CUSTOMER)/install/lib
endif