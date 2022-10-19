-include $(TOP_DIR)/.config

BUILD_DIRS += sys ethernet config
#ifeq ($(CONFIG_ALGORITHM_VENDOR_MEGVII), y)
#BUILD_DIRS += facelib_megvii
#endif

ifeq ($(CONFIG_NFC_SUPPORT), y)
#BUILD_DIRS += nfc
endif

BUILD_DIRS += 3rd_prebuilt

.PHONY: all clean install

all:
	@for dir in $(BUILD_DIRS) ; do \
		if test -d $$dir ; then \
			echo "$$dir: $(MAKE) -j$(NPROC) $@" ; \
			if ($(MAKE) -C $$dir -j$(NPROC) $@) ; then true; else exit 1; fi; \
		fi \
	done;

clean:
	@for dir in $(BUILD_DIRS) ; do \
		if test -d $$dir ; then \
			echo "$$dir: $(MAKE) $@" ; \
			if ($(MAKE) -C $$dir $@) ; then true; else exit 1; fi; \
		fi \
	done;

install:
	@for dir in $(BUILD_DIRS) ; do \
		if test -d $$dir ; then \
			echo "$$dir: $(MAKE) $@" ; \
			if ($(MAKE) -C $$dir $@) ; then true; else exit 1; fi; \
		fi \
	done;
