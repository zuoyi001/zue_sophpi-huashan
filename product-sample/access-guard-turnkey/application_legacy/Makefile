-include $(TOP_DIR)/.config
##
PWD := $(shell pwd)
##
TARGET := sac_application.bin
##
SRCS_C := $(PWD)/sac_main_app.c
SRCS_C += $(PWD)/resources/app_rtsp.c
SRCS_C += $(PWD)/resources/config_data/config_data.c
SRCS_C += $(PWD)/resources/common/msgq.c
SRCS_C += $(PWD)/resources/common/threadpool.c
SRCS_C += $(PWD)/resources/utils/app_utils.c
SRCS_C += $(TOP_DIR)/customer/$(CUSTOMER)/$(CUSTOMER)_gui_ext.c
SRCS_C += $(PWD)/resources/app_ai.c
#SRCS_C += $(PWD)/resources/audio/alsacommon.c
#SRCS_C += $(PWD)/resources/audio/audio_debug.c
#SRCS_C += $(PWD)/resources/audio/cvi_audio_main.c
#SRCS_C += $(PWD)/resources/audio/cvi_audio_utils.c
#SRCS_C += $(PWD)/resources/audio/cyclebuffer.c
SRCS_C += $(PWD)/resources/app_init.c
SRCS_C += $(PWD)/resources/public.c
SRCS_C += $(PWD)/resources/app_video.c
ifeq ($(CONFIG_ALGORITHM_VENDOR_AISDK), y)
SRCS_C += $(TOP_DIR)/customer/$(CUSTOMER)/$(CUSTOMER)_vpss_config_aisdk.c
else
SRCS_C += $(TOP_DIR)/customer/$(CUSTOMER)/$(CUSTOMER)_vpss_config.c
endif
SRCS_C += $(PWD)/resources/factory/factory.c
SRCS_C += $(PWD)/resources/peripheral/app_peripheral_comp.c
SRCS_C += $(PWD)/resources/peripheral/app_peripheral.c
SRCS_C += $(PWD)/resources/ping.c
SRCS_C += $(PWD)/resources/app_comp.c
SRCS_C += $(PWD)/resources/menu/app_menu.c
SRCS_C += $(PWD)/resources/menu/core/core.c
SRCS_C += $(PWD)/resources/menu/model/draw_helper.c
SRCS_C += $(PWD)/resources/menu/model/peri_helper.c
SRCS_C += $(PWD)/resources/menu/view/common/dialog.c
SRCS_C += $(PWD)/resources/menu/view/common/update_dialog.c
SRCS_C += $(PWD)/resources/menu/view/common/write_mac_sn.c
SRCS_C += $(PWD)/resources/menu/view/common/input_dialog.c
SRCS_C += $(PWD)/resources/menu/view/common/down_user_dialog.c
SRCS_C += $(PWD)/resources/menu/view/common/util.c
SRCS_C += $(PWD)/resources/menu/view/common/write_mac_sn_dialog.c
SRCS_C += $(PWD)/resources/menu/view/common/select_dialog.c
SRCS_C += $(PWD)/resources/menu/view/common/keyboard.c
SRCS_C += $(PWD)/resources/menu/view/common/face_rect.c
SRCS_C += $(PWD)/resources/menu/view/launcher/launcher.c
SRCS_C += $(PWD)/resources/menu/view/i18n/translations/lv_i18n.c
SRCS_C += $(PWD)/resources/menu/view/menu/menu_net/menu_net_eth.c
SRCS_C += $(PWD)/resources/menu/view/menu/menu_net/wifi_input_pwd_dialog.c
SRCS_C += $(PWD)/resources/menu/view/menu/menu_net/menu_net.c
SRCS_C += $(PWD)/resources/menu/view/menu/menu_net/menu_net_wifi.c
SRCS_C += $(PWD)/resources/menu/view/menu/menu_net/menu_net_web.c
SRCS_C += $(PWD)/resources/menu/view/menu/menu_device/menu_access_control.c
SRCS_C += $(PWD)/resources/menu/view/menu/menu_device/menu_notification.c
SRCS_C += $(PWD)/resources/menu/view/menu/menu_device/menu_device_activate.c
SRCS_C += $(PWD)/resources/menu/view/menu/menu_device/menu_device_info.c
SRCS_C += $(PWD)/resources/menu/view/menu/menu_device/menu_device_manage.c
SRCS_C += $(PWD)/resources/menu/view/menu/menu_setting.c
SRCS_C += $(PWD)/resources/menu/view/menu/menu.c
SRCS_C += $(PWD)/resources/menu/view/menu/menu_face/face_delete_dialog.c
SRCS_C += $(PWD)/resources/menu/view/menu/menu_face/menu_face_demo.c
SRCS_C += $(PWD)/resources/menu/view/menu/menu_face/face_register_dialog.c
SRCS_C += $(PWD)/resources/menu/view/menu/menu_face/menu_face_lib.c
SRCS_C += $(PWD)/resources/menu/view/menu/menu_face/face_edit_dialog.c
SRCS_C += $(PWD)/resources/menu/view/menu/menu_face/face_add_dialog.c
SRCS_C += $(PWD)/resources/menu/view/menu/menu_face/face_capture_page.c
SRCS_C += $(PWD)/resources/menu/view/menu/menu_page.c
SRCS_C += $(PWD)/resources/menu/view/menu/menu_record/menu_record_manage.c
SRCS_C += $(PWD)/resources/menu/view/ime/kb/ime_kb.c
SRCS_C += $(PWD)/resources/menu/view/testmode/menu_camera_test.c
SRCS_C += $(PWD)/resources/menu/view/testmode/menu_ir_led_test.c
SRCS_C += $(PWD)/resources/menu/view/testmode/menu_test_result.c
SRCS_C += $(PWD)/resources/menu/view/testmode/menu_tp_test.c
SRCS_C += $(PWD)/resources/menu/view/testmode/menu_mic_test.c
SRCS_C += $(PWD)/resources/menu/view/testmode/menu_factory_mode.c
SRCS_C += $(PWD)/resources/menu/view/testmode/menu_test_mode.c
SRCS_C += $(PWD)/resources/menu/view/testmode/menu_wifi_test.c
SRCS_C += $(PWD)/resources/menu/view/testmode/menu_audio_test.c
SRCS_C += $(PWD)/resources/menu/view/testmode/menu_lcd_test.c
SRCS_C += $(PWD)/resources/menu/view/testmode/menu_nfc_test.c
SRCS_C += $(PWD)/resources/menu/view/testmode/menu_about_test.c
SRCS_C += $(PWD)/resources/menu/view/testmode/menu_eth_test.c
SRCS_C += $(PWD)/resources/menu/view/testmode/menu_key_test.c
SRCS_C += $(PWD)/resources/menu/view/testmode/menu_radar_test.c
SRCS_C += $(PWD)/resources/menu/view/testmode/menu_usb_test.c
SRCS_C += $(PWD)/resources/menu/view/testmode/menu_led_test.c
SRCS_C += $(PWD)/resources/menu/view/testmode/menu_relay_test.c
SRCS_C += $(PWD)/resources/menu/view/testmode/menu_rs485_wg_test.c
##
SRCS_CPP += $(PWD)/resources/menu/view/ime/pinyin/utf.cpp
SRCS_CPP += $(PWD)/resources/menu/view/ime/pinyin/ime.cpp
##
OBJS := $(SRCS_C:%.c=%.o)
OBJS += $(SRCS_CPP:%.cpp=%.o)
## ?
DEFS = -DSUPPORT_LVGL
# DEFS += -DCLI_CMD_DEBUG
CFLAGS += $(DEFS)
##
INCLUDES += -I$(PWD)/resources/utils
INCLUDES += -I$(PWD)/resources/video
INCLUDES += -I$(PWD)/resources/
INCLUDES += -I$(PWD)/resources/peripheral
INCLUDES += -I$(PWD)/resources/factory
INCLUDES += -I$(PWD)/resources/config_data
INCLUDES += -I$(PWD)/resources/common
INCLUDES += -I$(PWD)/resources/menu
INCLUDES += -I$(PWD)/resources/menu/model
INCLUDES += -I$(PWD)/../function_lib/3rd_prebuilt/cvi_rtsp/include
INCLUDES += -I$(PWD)/../function_lib/config/include
#INCLUDES += -I$(TOP_DIR)/include/function_lib/peripheral
INCLUDES += -I$(PWD)/../function_lib/nfc/include
INCLUDES += -I$(PWD)/../function_lib/ethernet
INCLUDES += -I$(PWD)/../function_lib/sys
INCLUDES += -I$(PWD)/../include/function_lib
INCLUDES += -I$(PWD)/../include/
INCLUDES += -I$(PWD)/../include/customer
INCLUDES += -I$(TOP_DIR)/cpsl/hal/algorithm/include
INCLUDES += -I$(TOP_DIR)/cpsl/db/include
INCLUDES += -I$(TOP_DIR)/cpsl/hal/gpio/include
INCLUDES += -I$(TOP_DIR)/cpsl/hal/uart/include
INCLUDES += -I$(TOP_DIR)/cpsl/hal/wiegand/include
INCLUDES += -I$(TOP_DIR)/cpsl/hal/watchdog/include
INCLUDES += -I$(TOP_DIR)/cpsl/hal/pwm/include


ifeq ($(CONFIG_STATIC_LINK), y)
ELFFLAGS += -Wl,-Bstatic
else
ELFFLAGS += -Wl,-Bdynamic
endif

##
LIB_DIR += -L../function_lib/3rd_prebuilt/cvi_rtsp/$(SDK_VER)
LIB_DIR += -L../function_lib/3rd_prebuilt/googlepinyin/$(SDK_VER)
LIB_DIR += -L../function_lib/3rd_prebuilt/others/$(SDK_VER)
LIB_DIR += -L../gui_engine/LittleVGL
#LIB_DIR += -L../function_lib/db
LIB_DIR += -L../cpsl/db
LIB_DIR += -L../function_lib/facelib_megvii
LIB_DIR += -L../function_lib/config
#LIB_DIR += -L../function_lib/peripheral
LIB_DIR += -L../function_lib/nfc
LIB_DIR += -L../function_lib/ethernet
LIB_DIR += -L../function_lib/sys

LIB_DIR += -L$(SDK_PATH)/middleware/v2/lib
LIB_DIR += -L$(SDK_PATH)/middleware/v2/lib/3rd
##
include $(PWD)/resources/menu/menu.mk
include $(PWD)/resources/common/common.mk
##
LIBS += -lsns_full
LIBS += -lsample -lvpu -lisp -lcvi_bin -lcvi_bin_isp -lvenc -lvdec -lawb -lae -laf -lm -lsys -pthread -lisp_algo
#LIBS += -lnanomsg
ifeq ($(CONFIG_CVI_SOC_CV182X), y)
LIBS += -lisp_algo
endif
#LIBS += -lcvi_audio -lcvi_vqe -lcvi_VoiceEngine -lcvi_RES1 -lcvi_ssp -ltinyalsa
LIBS += -lcvi_rtsp
LIBS += -lmipi_tx
LIBS += -lini
LIBS += -llvgl -lstdc++ -ldl
# LIBS += -lcvitracer

ELFFLAGS += -march=rv64imafdcvxthead -mcmodel=medany -mabi=lp64d
#function lib
LIBS += $(ELFFLAGS) -lsample_sys -lnet -Wl,-Bdynamic #-lcli
LIBS += -lgooglepinyin

ifeq ($(CONFIG_NFC_SUPPORT), y)
#LIBS += -lnfc
endif

LIBS += -lcvi_hal_db $(ELFFLAGS) -lcviconfig -Wl,-Bdynamic
#LIBS += -lface $(AI_SDK_LIB) $(IVE_SDK_LIB) $(TPU_LIB) $(OPENCV_LIB)
#LIBS += -lface $(IVE_SDK_LIB) $(OPENCV_LIB)
#LIBS += -lface $(TPU_LIB) $(OPENCV_LIB) -Wl,-Bdynamic


INCLUDES += -I$(PWD)/../gui_engine/LittleVGL
#INCLUDES += -I$(PWD)/src/platform
INCLUDES += -I$(SDK_PATH)/middleware/v2/include
INCLUDES += -I$(SDK_PATH)/middleware/v2/include/isp/mars
INCLUDES += -I$(SDK_PATH)/middleware/v2/sample/common
INCLUDES += -I$(SDK_PATH)/middleware/v2/3rdparty/inih
INCLUDES += -I$(KERNEL_INC)
INCLUDES += -I$(PWD)/src/gui/
INCLUDES += -I$(PWD)/3rdparty/tracer
#

LIBS += -L$(TOP_DIR)/cpsl/hal/gpio -lcvi_hal_gpio
LIBS += -L$(TOP_DIR)/cpsl/hal/uart -lcvi_hal_uart
LIBS += -L$(TOP_DIR)/cpsl/hal/wiegand -lcvi_hal_wiegand
LIBS += -L$(TOP_DIR)/cpsl/hal/watchdog -lcvi_hal_watchdog
LIBS += -L$(TOP_DIR)/cpsl/hal/pwm -lcvi_hal_pwm

ifeq ($(CONFIG_ALGORITHM_VENDOR_MEGVII), y)
ifeq ($(MW_SDK_VER), 32bit)
LIB_DIR += -Wl,--rpath-link $(TOP_DIR)/cpsl/hal/algorithm/src/Meg_Lg_CV1835_FacePass_V1.2.1/lib/arm32
LIB_DIR += -Wl,--rpath-link $(TOP_DIR)/cpsl/hal/algorithm/src/Meg_Lg_CV1835_FacePass_V1.2.1/demo/thirdparty/image/share/arm32
LIB_DIR += -Wl,--rpath-link $(TOP_DIR)/cpsl/hal/algorithm/src/Meg_Lg_CV1835_FacePass_V1.2.1/demo/thirdparty/sqlite3/lib/arm32
else
LIB_DIR += -Wl,--rpath-link $(TOP_DIR)/cpsl/hal/algorithm/src/Meg_Lg_CV1835_FacePass_V1.2.1/lib/arm64
LIB_DIR += -Wl,--rpath-link $(TOP_DIR)/cpsl/hal/algorithm/src/Meg_Lg_CV1835_FacePass_V1.2.1/demo/thirdparty/image/share/arm64
LIB_DIR += -Wl,--rpath-link $(TOP_DIR)/cpsl/hal/algorithm/src/Meg_Lg_CV1835_FacePass_V1.2.1/demo/thirdparty/sqlite3/lib/arm64
endif
LIBS += -lcvi_hal_face_megvii
endif
ifeq ($(CONFIG_ALGORITHM_VENDOR_AISDK), y)
ifeq ($(CONFIG_MW_SDK_VER_LIB32), y)
TPU_PREBUILT_PATH = $(TOP_DIR)/prepare/cvi_aisdk/$(shell echo $(CONFIG_CVI_SOC) | sed 's/\"//g')/32bit
endif
ifeq ($(CONFIG_MW_SDK_VER_LIB64), y)
TPU_PREBUILT_PATH = $(TOP_DIR)/prepare/cvi_aisdk/$(shell echo $(CONFIG_CVI_SOC) | sed 's/\"//g')/64bit
endif
ifeq ($(CONFIG_MW_SDK_VER_UCLIBC), y)
TPU_PREBUILT_PATH = $(TOP_DIR)/prepare/cvi_aisdk/$(shell echo $(CONFIG_CVI_SOC) | sed 's/\"//g')/uclibc
endif
ifeq ($(CONFIG_MW_SDK_VER_MUSL), y)
TPU_PREBUILT_PATH = $(TOP_DIR)/prepare/cvi_aisdk/$(shell echo $(CONFIG_CVI_SOC) | sed 's/\"//g')/musl_riscv64
endif

LIBS += -Wl,--rpath-link $(TPU_PREBUILT_PATH)/cvitek_tpu_sdk/opencv/lib
LIBS += -Wl,--rpath-link $(TPU_PREBUILT_PATH)/cvitek_ai_sdk/lib
LIBS += -Wl,--rpath-link $(TPU_PREBUILT_PATH)/cvitek_tpu_sdk/lib
#LIBS += -Wl,--rpath-link $(TPU_PREBUILT_PATH)/cvitek_ive_sdk/lib
LIBS += -Wl,--rpath-link $(SDK_PATH)/middleware/v2/lib/

LIBS += -lcvi_hal_face_aisdk
endif
LIB_DIR += -L$(TOP_DIR)/cpsl/hal/algorithm
#
CFLAGS += -fPIC -pthread $(DEFS) -D__FILENAME__=\"$(notdir $<)\"
CFLAGS += -Wall -Werror -O3 -Wno-stringop-truncation -Wno-stringop-overflow -Wno-format-truncation -Wno-format-overflow
CFLAGS += -std=gnu99 -D_GNU_SOURCE


CXXFLAGS += -fPIC -pthread $(DEFS)
CXXFLAGS += -Wall -Werror -O3
CXXFLAGS += -std=gnu++11 -D_GNU_SOURCE
#LDFLAGS += -shared

.PHONY: all clean install

default: all

all: $(OBJS)
	$(CC) -fPIC -o $(TARGET) $(LDFLAGS) $(OBJS) $(LIB_DIR) $(LIBS) $(PKG_LIB) $(RPATH_DIR) $(OPENCV_LIB)
#	$(STRIP) -s $(TARGET)

%.o: %.c $(DEPS)
	$(CC) -c $< -o $@ $(INCLUDES) $(CFLAGS)

%.o: %.cpp $(DEPS)
	$(CXX) -c $< -o $@ $(INCLUDES) $(CXXFLAGS) -fpermissive

clean:
	rm -rf $(OBJS) $(TARGET)

install:
	cp -rpf sac_application.bin $(TOP_DIR)/customer/$(CUSTOMER)/install
	cp -rpf resources/menu/res  $(TOP_DIR)/customer/$(CUSTOMER)/install
#	cp -rpf $(GCC_SYSROOT_PATH)/libc/lib/libanl* $(TOP_DIR)/customer/$(CUSTOMER)/install/lib
#	cp -rpf $(GCC_SYSROOT_PATH)/libc/lib/libnsl* $(TOP_DIR)/customer/$(CUSTOMER)/install/lib
	
	
