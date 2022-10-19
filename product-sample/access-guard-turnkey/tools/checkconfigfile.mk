ifeq ($(TOP_DIR)/.config,$(wildcard $(TOP_DIR)/.config))
$(info  $(TOP_DIR)/.config)
else
$(error "please do 'make menuconfig' first!!")
endif

.PHONY: all

default: all

all:
	@echo "run checkconfigfile.mk"
