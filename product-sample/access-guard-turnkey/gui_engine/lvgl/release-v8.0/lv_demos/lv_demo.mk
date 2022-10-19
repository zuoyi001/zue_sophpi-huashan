LV_DEMOS_DIR_NAME ?= lv_demos

CSRCS += $(shell find -L $(LVGL_DIR)/$(LV_DEMOS_DIR_NAME) -name "*.c")