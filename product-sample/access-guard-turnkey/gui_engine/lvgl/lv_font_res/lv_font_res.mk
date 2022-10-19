LV_FONT_RES ?= $(LVGL_DIR)/../lv_font_res
CSRCS += $(LV_FONT_RES)/stsong_18.c
CSRCS += $(LV_FONT_RES)/stsong_22.c
CSRCS += $(LV_FONT_RES)/stsong_26.c
CSRCS += $(LV_FONT_RES)/stsong_28.c
CSRCS += $(LV_FONT_RES)/stsong_32.c
CSRCS += $(LV_FONT_RES)/stsong_36.c

CFLAGS += "-I$(LV_FONT_RES)"
