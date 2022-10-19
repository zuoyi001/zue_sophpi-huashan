LV_LIB_PNG_DIR_NAME ?= lv_lib_png
LV_LIBS ?= $(LVGL_DIR)/../lv_libs
CSRCS += $(wildcard $(LV_LIBS)/$(LV_LIB_PNG_DIR_NAME)/*.c)

