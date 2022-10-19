CSRCS += lodepng.c
CSRCS += lv_lodepng.c

DEPPATH += --dep-path $(LVGL_DIR)/lv_lib/lv_lib_lodepng
VPATH += :$(LVGL_DIR)/lv_lib/lv_lib_lodepng

CFLAGS += "-I$(LVGL_DIR)/lv_lib/lv_lib_lodepng"
