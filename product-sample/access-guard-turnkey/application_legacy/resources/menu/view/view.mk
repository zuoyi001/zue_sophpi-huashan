
include $(AG_UI_DIR)/resources/menu/view/menu/menu.mk
include $(AG_UI_DIR)/resources/menu/view/i18n/i18n.mk
include $(AG_UI_DIR)/resources/menu/view/ime/ime.mk

INCLUDES += -I$(AG_UI_DIR)/resources/menu/view/common 
INCLUDES += -I$(AG_UI_DIR)/resources/menu/view/launcher 
INCLUDES += -I$(AG_UI_DIR)/resources/menu/view/menu
INCLUDES += -I$(AG_UI_DIR)/resources/menu/view/testmode