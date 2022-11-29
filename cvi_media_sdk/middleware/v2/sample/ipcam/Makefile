SHELL = /bin/bash

TOPTARGETS := all clean install

# SUBDIRS := $(notdir $(basename $(shell find . -maxdepth 1 -type d)))
# $(info "CHIP_ARCH=$(CHIP_ARCH) CHIP_SERIES=$(CHIP_SERIES)")

ifneq ($(findstring $(CHIP_ARCH), CV182X CV183X), )
SUBDIRS := cmd mercury
else ifneq ($(findstring $(CHIP_ARCH), MARS PHOBOS), )
SUBDIRS := cmd mars
else
$(error UNKNOWN chip series - $(CHIP_ARCH))
endif

$(TOPTARGETS): $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)
