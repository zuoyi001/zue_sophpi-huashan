##
menu "Main app config"

config DUAL_SENSOR_SUPPORT
    bool "dual sensor (rgb+ir) support"
	default y
	help
        dual sensor (rgb+ir) support.

config SENSOR0_IS_RGB
    depends on DUAL_SENSOR_SUPPORT
    bool "dual sensor, sensor0 is rgb"
	default y
	help
        dual sensor, sensor0 is rgb, n is ir.

config ENABLE_ISPD
	bool "Enable ispd thread for isp tuning."
	default y
	help
		ENABLE_ISPD define.

choice
    prompt "Input device config"
    default INPUT_TP
config INPUT_TP
    bool "TouchPad"
config INPUT_KB
    bool "KeyBoard"
endchoice

config BUILD_APP
    bool "build app with lvgl-v8.0"
    depends on ENABLE_GUI_LVGL
    default n
    help
        build app legacy with lvgl-v8.0.

config BUILD_APP_LEGACY
    bool "build app legacy with lvgl-v7.0"
    depends on ENABLE_GUI_LVGL
    depends on !BUILD_APP
    default y
    help
        build app legacy with lvgl-v7.0.

config BUILD_APP_SAMPLE
    bool "build app sample."
    depends on ENABLE_GUI_AWTK
    default y
    help
        build app sample.

config BUILD_APP_FRAMEWORK
    bool "build app framework."
    depends on ENABLE_GUI_AWTK
    depends on !BUILD_APP_SAMPLE
    default y
    help
        build app framework.

choice
    prompt "VI rotation config"
    default VI_ROTATION_NONE
config VI_ROTATION_NONE
    bool "0 degree"
config VI_ROTATION_90
    bool "90 degree"
config VI_ROTATION_180
    bool "180 degree"
config VI_ROTATION_270
    bool "270 degree"
endchoice

choice
    prompt "VO rotation config"
    default VO_ROTATION_NONE
config VO_ROTATION_NONE
    bool "0 degree"
config VO_ROTATION_90
    bool "90 degree"
config VO_ROTATION_180
    bool "180 degree"
config VO_ROTATION_270
    bool "270 degree"
endchoice

config VPSS_VIDEO_FLIP_SUPPORT
    bool "vpss video flip support"
	default n
	help
        vpss video flip support.

config VPSS_VIDEO_MIRROR_SUPPORT
    bool "vpss video mirror support"
	default n
	help
        vpss video mirror support.

config RGB_VIDEO_RTSP_SUPPORT
    bool "RGB Video rtsp support"
	default n
	help
        RGB Video rtsp support.

choice
  	prompt "RGB Video rtsp stream source"
    depends on RGB_VIDEO_RTSP_SUPPORT
    default RGB_VIDEO_RTSP_VENC_BIND_VI
config RGB_VIDEO_RTSP_VENC_BIND_VI
    bool "RGB, vi vind venc"
config RGB_VIDEO_RTSP_VENC_BIND_VPSS
    bool "RGB, vpss bind venc"
config RGB_VIDEO_RTSP_VENC_BIND_DISABLE
    bool "RGB, venc bind disable"
endchoice

config IR_VIDEO_RTSP_SUPPORT
    bool "IR Video rtsp support"
	default n
	help
        IR Video rtsp support.
choice
  	prompt "IR Video rtsp stream source"
    depends on IR_VIDEO_RTSP_SUPPORT
    default IR_VIDEO_RTSP_VENC_BIND_VI
config IR_VIDEO_RTSP_VENC_BIND_VI
    bool "IR, vi vind venc"
config IR_VIDEO_RTSP_VENC_BIND_VPSS
    bool "IR, vpss bind venc"
config IR_VIDEO_RTSP_VENC_BIND_DISABLE
    bool "IR, venc bind disable"
endchoice
##
config ISP_FACE_AE_SUPPORT
    bool "isp face ae support"
    default n
	help
        isp face ae support.

config WEBSERVER_SUPPORT
    bool "webserver support"
	depends on !VIDEO_RTSP_SUPPORT
    default n
	help
        webserver support.

config FILE_STORAGE_SUPPORT
    bool "file storage support"
    default n
    help
        file storage support.

config FILE_PLAYER_SUPPORT
    bool "File player support"
    default n
	help
        File player support.

config FILE_PLAYER_SUPPORT
    bool "File player support"
    default n
	help
        File player support.


config ETHERNET_SUPPORT
    bool "Ethernet connect support"
    default n
	help
        Ethernet connect support.

config WIFI_SUPPORT
    bool "Wifi connect support"
    default n
	help
        Wifi connect support.

config BLUETOOTH_SUPPORT
    bool "Bluetooth connect support"
    default n
	help
        Bluetooth connect support.

config AUDIO_PLAYER_SUPPORT
    bool "Audio player support"
    default n
	help
        Audio player support.

config MP3_SUPPORT
    bool "Mp3 audio play support"
    depends on AUDIO_PLAYER_SUPPORT
    default n
	help
        Mp3 audio play support.

config FACTORY_TEST_SUPPORT
    bool "Factory test support"
    default y
	help
        Factory test support.

endmenu
