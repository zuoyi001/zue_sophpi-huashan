-include $(TOP_DIR)/.config

.PHONY: all clean install 

all:
	@echo "3rd_prebuilt"

clean:
	@echo "3rd_prebuilt"

install:
ifeq ($(CONFIG_VIDEO_RTSP_SUPPORT), y)
	cp -rpf cvi_rtsp/$(SDK_VER)/*.so* $(TOP_DIR)/customer/$(CUSTOMER)/install/lib
endif
	cp -rpf googlepinyin/$(SDK_VER)/*.so* $(TOP_DIR)/customer/$(CUSTOMER)/install/lib
