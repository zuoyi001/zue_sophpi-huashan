-include $(TOP_DIR)/.config

PWD := $(shell pwd)
SDIR := $(PWD)


ifeq ($(SDK_VER), lib32)
export CROSS_COMPILE = $(SDK_PATH)/host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
else
export CROSS_COMPILE = $(SDK_PATH)/host-tools/gcc/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-
endif


BUILD_DIRS = awtk-port

.PHONY : all clean install

all : 
	$(PWD)/build_awtk.sh
#
	@for dir in $(BUILD_DIRS) ; do \
		if test -d $$dir ; then \
			echo "$$dir: $(MAKE) -j$(NPROC) $@" ; \
			if ($(MAKE) -C $$dir -j$(NPROC) $@) ; then true; else exit 1; fi; \
		fi \
	done;


clean:
	rm -rf awtk-linux-fb/build
#
	@for dir in $(BUILD_DIRS) ; do \
		if test -d $$dir ; then \
			echo "$$dir: $(MAKE) $@" ; \
			if ($(MAKE) -C $$dir $@) ; then true; else exit 1; fi; \
		fi \
	done;

install:
ifneq ($(CONFIG_STATIC_LINK), y)
	cp -rpf awtk-linux-fb/build/bin/libawtk.so $(TOP_DIR)/customer/$(CUSTOMER)/install/lib
endif
#
	@for dir in $(BUILD_DIRS) ; do \
		if test -d $$dir ; then \
			echo "$$dir: $(MAKE) $@" ; \
			if ($(MAKE) -C $$dir $@) ; then true; else exit 1; fi; \
		fi \
	done;