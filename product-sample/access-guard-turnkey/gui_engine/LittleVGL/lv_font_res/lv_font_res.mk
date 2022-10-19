#CSRCS += stsong_16.c
CSRCS += stsong_18.c
CSRCS += stsong_22.c
#CSRCS += stsong_24.c
CSRCS += stsong_26.c
CSRCS += stsong_28.c
#CSRCS += stsong_30.c
CSRCS += stsong_32.c
#CSRCS += stsong_34.c
CSRCS += stsong_36.c

DEPPATH += --dep-path $(LVGL_DIR)/lv_font_res
VPATH += :$(LVGL_DIR)/lv_font_res

CFLAGS += "-I$(LVGL_DIR)/lv_font_res"
