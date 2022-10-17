TOPTARGETS := all clean install
.PHONY: $(TOPTARGETS)

# SUBDIRS := $(wildcard */.)
SUBDIRS := \
	cardv\
	sample

$(TOPTARGETS): $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)

