include $(SRCTREE)/build/base.mk

TOPTARGETS := all clean install

# SUBDIRS := $(notdir $(basename $(shell find . -maxdepth 1 -type d)))
SUBDIRS := \
	component/isp/sensor/cv183x \
	sample/common

$(TOPTARGETS): $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)

$(INSTALL_PATH)/lib:
	mkdir -p $@

install: $(SUBDIRS) | $(INSTALL_PATH)/lib
	cp -a $(CURDIR)/lib/* $(INSTALL_PATH)/lib
