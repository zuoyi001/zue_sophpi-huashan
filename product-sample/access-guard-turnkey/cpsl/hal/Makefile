-include $(TOP_DIR)/.config

BUILD_DIRS = gpio uart watchdog wiegand pwm touchpad algorithm

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