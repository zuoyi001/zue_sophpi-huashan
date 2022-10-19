AG_UI_DIR=$(shell pwd)

INCLUDES += -I$(PWD)/resources/menu

include $(AG_UI_DIR)/resources/menu/view/view.mk
include $(AG_UI_DIR)/resources/menu/model/model.mk
include $(AG_UI_DIR)/resources/menu/core/core.mk