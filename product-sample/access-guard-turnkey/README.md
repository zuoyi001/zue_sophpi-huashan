

1. 按照如下目录顺序摆放 SDK 与 sample_app source code。

	```
	sac_release_sdk (smart_access_control branch)
	├── build
	├── host-tools -> /home/test/host-tools
	├── install
	├── isp_tuning
	├── linux-linaro-stable
	├── middleware
	├── ramdisk
	└── u-boot
	access-guard-turnkey (megvii branch)
	├── ai_soc
	├── application
	├── build.sh
	├── customer
	├── function_lib
	├── gui_engine
	├── include
	├── install
	├── Makefile
	├── README.md
	└── tools
	```
	
2. 编译 SDK source

  ```
  cd sdk_source
  source buile/envseup_soc.sh
  menuconfig -> 选择 cv1835, wevb_0002a,...
  build_all
  编译成功之后， 可在 install 目录找到烧录 image。
  ```

3. 烧录 sdk image

```
可以使用 SD 卡烧录，USB 烧录， TFTP 烧录
```

4.  编译 sac_application.bin

  ```
  cd access-guard-turnkey
  make;make install
  将 access-guard-turnkey/install 目录下的内容 copy 到板子的 /mnt/data 目录
  重启后会依照 /mnt/data/auto.sh 内容自动执行 sac_application.bin 
  
  [root@cvitek]/mnt/data# ./auto_run.sh 
panel init ...
x is NULL
[SAC_APP_SetViConfigFromIni]-36: MMF Version:cv1835_v1.2.0-1528-g0884a29-32bit
waiting for connect...
[SAMPLE_COMM_VI_ParseIni]-2899: Parse /mnt/data/sensor_cfg.ini
[parse_handler]-2789: devNum =  2
[parse_handler]-2802: sensor =  SONY_IMX307_2L_MIPI_2M_30FPS_12BIT
[parse_handler]-2816: bus_id =  0
[parse_handler]-2822: mipi_dev =  0
[parse_handler]-2825: Lane_id =  1, 3, 2, -1, -1
[parse_handler]-2844: sensor2 =  SONY_IMX307_SLAVE_MIPI_2M_30FPS_12BIT
[parse_handler]-2859: bus_id =  3
[parse_handler]-2865: mipi_dev =  1
[parse_handler]-2868: Lane_id =  0, 4, 2, -1, -1
[parse_handler]-2877: mclk_en =  1
[parse_handler]-2880: mclk =  1
[SAC_APP_SetViConfigFromIni]-44: Parse complete
rpc_server_init
stSnsrMode.u16Width 1920 stSnsrMode.u16Height 1080 25.000000 wdrMode 0 pstSnsObj 0xf62eb48c
stSnsrMode.u16Width 1920 stSnsrMode.u16Height 1080 25.000000 wdrMode 0 pstSnsObj 0xf62eb8c4
[SAMPLE_COMM_VI_StartMIPI]-2047: sensor 0 stDevAttr.devno 0
[SAMPLE_COMM_VI_StartMIPI]-2047: sensor 1 stDevAttr.devno 1
Jan  1 08:01:08 cvitek sac_application.bin: [ISP-WRN] [W] CVI_ISP_Init::204 : ISP VERSION(T(2021-04-15), V(I137ff8, 5e37287))
Func awbInit line 2224,ver 4.23@4071037
0 R:1411 B:3109 CT:2850
1 R:1532 B:2508 CT:3900
2 R:2352 B:1608 CT:6500
cp 0 -496694
cp 1 1011735552
cp 2 -109756
cp 3 -36039668
cp 4 105251296
cp 5 -11844543
WB Quadratic:0
awb isWdrMode:0
Jan  1 08:01:08 cvitek sac_application.bin: [ISP-WRN] [W] setIspIqParamFromBin::146 : Can't find bin(/mnt/data/bin/cvi_sdr_bin), use default parameters
ViPipe:0,===IMX307 1080P 30fps 12bit LINE Init OK!===
[SAMPLE_COMM_ISP_Thread]-168: ISP Dev 0 running!
Func awbInit line 2224,ver 4.23@4071037
0 R:1411 B:3109 CT:2850
1 R:1532 B:2508 CT:3900
2 R:2352 B:1608 CT:6500
cp 0 -496694
cp 1 1011735552
cp 2 -109756
cp 3 -36039668
cp 4 105251296
cp 5 -11844543
WB Quadratic:0
awb isWdrMode:0
Jan  1 08:01:08 cvitek sac_application.bin: [ISP-WRN] [W] isp_buf_ctrl_algo_buf_init::118 : VIPipe (1) algo buffer already init
Jan  1 08:01:08 cvitek sac_application.bin: [ISP-WRN] [W] CVI_ISP_Init::204 : ISP VERSION(T(2021-04-15), V(I137ff8, 5e37287))
Jan  1 08:01:08 cvitek sac_application.bin: [ISP-WRN] [W] setIspIqParamFromBin::146 : Can't find bin(/mnt/data/bin/cvi_sdr_bin), use default parameters
ViPipe:1,===IMX307 1080P 30fps 12bit LINE Slave Init OK!===
[SAMPLE_COMM_ISP_Thread]-162: ISP Dev 1 return
Jan  1 08:01:08 cvitek sac_application.bin: [VPSS-ERR] cvi_vpss.c:234:setVpssParamFromBin(): Cant find bin(/mnt/data/bin/cvi_sdr_bin)
[SAMPLE_COMM_VO_StartChn]-284: u32Width:800, u32Height:1280, u32Square:1
The framebuffer device was opened successfully.
-- Default fb i[   68.765037] [0] 1294:disp_irq_handler():  disp bw failed at frame#1
nfo --
The ID=cvifb
The phy mem = 0xde00000, total size = 4096000(byte)
line length = 3200(byte)
xres = 800, yres = 1280, bits_per_pixel = 32
xresv = 800, yresv = 1280
vinfo.xoffset = 0, vinfo.yoffset = 0
vinfo.vmode is :0
finfo.ypanstep is :1
vinfo.red.offset=0x10
vinfo.red.length=0x8
vinfo.green.offset=0x8
vinfo.green.length=0x8
vinfo.blue.offset=0x0
vinfo.blue.length=0x8
vinfo.transp.offset=0x18
vinfo.transp.length=0x8
Expected screensize = 4096000(byte), using 1 frame
-- Updated fb info --
The ID=cvifb
The phy mem = 0xde00000, total size = 4096000(byte)
line length = 3200(byte)
xres = 800, yres = 1280, bits_per_pixel = 32
xresv = 800, yresv = 1280
vinfo.xoffset = 0, vinfo.yoffset = 0
vinfo.vmode is :0
finfo.ypanstep is :1
vinfo.red.offset=0x10
vinfo.red.length=0x8
vinfo.green.offset=0x8
vinfo.green.length=0x8
vinfo.blue.offset=0x0
vinfo.blue.length=0x8
vinfo.transp.offset=0x18
vinfo.transp.length=0x8
Expected screensize = 4096000(byte), using 1 frame
800x1280, 32bpp
The framebuffer device was mapped to memory successfully.
unable open evdev interface:: No such file or directory
wl mark 1, search_evdev 
wl mark 2, search_evdev 
wl mark3: (null)
wl mark3: (null)
wl mark3: (null)
wl mark3: (null)
wl mark3: (null)
to set indev /dev/input/event0---
wl mark 2, search_evdev 
rtsp://192.168.1.3:8554/live0
rtsp://192.168.1.3:8554/live1
start rtsp server
venc task1 start
venc task0 start
IME init ret 1
wdrLEOnly:1
wdrLEOnly:1
  ```

6. 旷视算法认证

  ```
  将 soft_license copy 到板子的 /mnt/data 目录，按如下步骤认证
  mkdir /mnt/data/auth
  export HASPUSER_PREFIX=/mnt/data/auth
  ./soft_auth_tool -f
  ./get_auth_file -f fp.c2v -c CBG_CV1835_Panel_Face_IR_Reco-xxx.cert -a xxxx
  ./soft_auth_tool -a
  注：get_auth_file 需要的 cert文件和串码可以从旷视获取。
  ```

7. 问题联系人

```
应用程序：
liang.wang@cvitek.com
tarzan.zhao@cvitek.com
macro.tan@cvitek.com
旷视算法：
liang.chen@cvitek.com
```

