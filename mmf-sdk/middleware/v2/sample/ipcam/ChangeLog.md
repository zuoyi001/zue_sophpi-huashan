# ChangeLog

| Version | Platform | Notes                                               | Date       | Author |
| ------- | -------- | --------------------------------------------------- | ---------- | ------ |
| v1.1.68 | Phobos   | 1. 增加Phobos config ini                            | 06/10/2022 | xulong |
| v1.1.67 | Mars     | 1. 增加数据处理缓存机制                             | 04/10/2022 | xulong |
| v1.1.66 | Mars     | 1. suooprt sc1346 30/60fps                          | 30/09/2022 | zl.wan |
| v1.1.65 | Mars     | 1. 修复rtsp重连 osdc线程段错误;                     | 30/09/2022 | leo |
| v1.1.64 | Mars     | 1. 修复rtsp重连段错误; 2.音频界面文字修改           | 30/09/2022 | leo |
| v1.1.63 | Mars     | 1. 添加vui                                          | 29/09/2022 | leo |
| v1.1.62 | Both     | 1. 修复cmdTest内存泄露; 2.修复pd加载失败退出段错误  | 28/09/2022 | leo |
| v1.1.61 | Mars     | 1. 修复切size pd检测效果降低 2.修复md,pd阈值范围 3.修改in部分参数| 28/09/2022 | zl.wan |
| v1.1.60 | Mars     | 1. 修复RTSP不能开多个预览窗口问题                   | 28/09/2022 | xulong |
| v1.1.59 | Mars     | 1. 修复音量小，播放不立刻停止; 2.修复切video段错误  | 28/09/2022 | leo |
| v1.1.58 | Mars     | 1. 修正OSD字串越界问题；2.修改OSDC buff size        | 27/09/2022 | xulong |
| v1.1.57 | Mars     | 1. 修复区域入侵web/图像显示效果;2 .修复区域入侵bug  | 27/09/2022 | zl.wan |
| v1.1.56 | Mars     | 1. 修改 venc attr时，不做rtsp destroy，除了 vcodec  | 27/09/2022 | xulong |
| v1.1.55 | Mars     | 1. 切换分辨率到720P，人形侦测不起作用，OSD也消失只留下未在走的时间戳显示 | 26/09/2022 | xulong |
| v1.1.54 | Mars     | 1. 修正webir手动切换/自动切换功能;2.修正不开ai websend ai fps 编译错误 | 23/09/2022 | zl.wan|
| v1.1.53 | Mars     | 1. 新增rtsp传输音频帧                               | 26/09/2022 | leo |
| v1.1.52 | Mars     | 1. 修正在web上设置OSD再切size异常等问题             | 23/09/2022 | xulong |
| v1.1.51 | Mars     | 1. 修正在web上的音频问题                            | 23/09/2022 | leo |
| v1.1.50 | Mars     | 1. 增加web上roi功能                                 | 23/09/2022 | leo |
| v1.1.49 | Mars     | 1. 修正 在web上修改osd不生效问题                    | 23/09/2022 | xulong |
| v1.1.48 | Mars     | 1. 增加web上画隐私区域功能                          | 23/09/2022 | xulong |
| v1.1.47 | Mars     | 1. 新增web AI帧率显示；2. 新增一个处理各种小任务的线程 | 22/09/2022 | xulong |
| v1.1.46 | Both     | 1. aac编码开启aec，sound mode必须是mono             | 21/09/2022 | leo    |
| v1.1.45 | Mars     | 1. 修改web 设置 venc 属性的一些错误                 | 21/09/2022 | xulong |
| v1.1.44 | Mars     | 1. 增加web ai page                                  | 21/09/2022 | zl.wan |
| v1.1.43 | Mars     | 1. 增加拍照；2. 修改OSD 一些错误                    | 20/09/2022 | xulong |
| v1.1.42 | Mars     | 1. 增加Web 卡录 功能                                | 20/09/2022 | leo    |
| v1.1.41 | Both     | 1. 增加Web audio功能                                | 20/09/2022 | leo    |
| v1.1.40 | Mars     | 1. 1.优化web相关API                                 | 18/09/2022 | xulong |
| v1.1.39 | Mars     | 1. 1.增加web相关功能；包括preview、image、osd 等 get/set | 12/09/2022 | xulong |
| v1.1.38 | Mars     | 1. OSDC压缩移到小核后，更新上层应用流程             | 05/09/2022 | xulong |
| v1.1.37 | Both     | 1. 支持smart p                                      | 01/09/2022 | leo    |
| v1.1.36 | Mars     | 1. mars静态编译支持pq 2.修复fd退出错误              | 31/08/2022 | zl.wan |
| v1.1.35 | Both     | 1. mercury编译报错 2.默认打开3dnr 3.vui设置与fps匹配| 30/08/2022 | leo    |
| v1.1.34 | Mars     | 1. 增加控制AI画框的命令                             | 28/08/2022 | xulong |
| v1.1.33 | Both     | 1. 修复ai leak泄露；2.mars支持sc2335;3.增加libturbo glibc so | 26/08/2022 | zl.wan |
| v1.1.32 | Both     | 1. 优化切video size速度,从6秒左右到1秒左右          | 26/08/2022 | xulong |
| v1.1.31 | Both     | 1. 优化切video size速度                             | 23/08/2022 | xulong |
| v1.1.30 | Mars     | 1. 修改glibc编译libdl.a报错                         | 23/08/2022 | xulong |
| v1.1.29 | Both     | 1. 添加osd DEBUG显示                                | 22/08/2022 | Leo    |
| v1.1.28 | Both     | 1. 添加cover颜色变换 2.抓拍默认支持FR，解决fr段错误 | 22/08/2022 | Leo    |
| v1.1.27 | Mars     | 1. 优化切video size速度；2. 修改 Ctrl+c 退出流程    | 21/08/2022 | xulong |
| v1.1.26 | Mars     | 1. 添加子码流OSD/cover 2.支持画线                   | 19/08/2022 | Leo    |
| v1.1.25 | Mecury   | 1. 增加attach venc,避免切换venc size ion申请失败    | 19/08/2022 | Leo    |
| v1.1.24 | Both     | 1. 优化ipcam size大小                               | 18/08/2022 | xulong |
| v1.1.23 | Both     | 1. 增加AI帧率与耗时获取接口                         | 18/08/2022 | zl.wan |
| v1.1.22 | Both     | 1. 修改 lpthread 库为 pthread                       | 18/08/2022 | xulong |
| v1.1.21 | Both     | 1. 增加libjpegrurbo静态库                           | 17/08/2022 | zl.wan |
| v1.1.20 | Mars     | 1. 增加SBM切size flow                               | 17/08/2022 | xulong |
| v1.1.19 | Mars     | 1.support face capture 2.add libjpegrurbo           | 15/08/2022 | zl.wan |
| v1.1.18 | Mars     | aac support static                                  | 15/08/2022 | Leo    |
| v1.1.17 | Mars     | add new sensor K06                                  | 11/08/2022 | zl.wan |
| v1.1.16 | Mecury   | 1. 优化人脸识别功能 2.增加宠物侦测功能              | 10/08/2022 | zl.wan |
| v1.1.15 | Mars     | 1. 删除冗余代码                                     | 09/08/2022 | xulong |
| v1.1.14 | Mars     | 1. fixed VDI timeout error                          | 09/08/2022 | xulong |
| v1.1.13 | Mars     | 1. 默认disable AI；2. 修改空指针报错                | 09/08/2022 | xulong |
| v1.1.12 | Mars     | 增加Mars video pipeline 说明                        | 08/08/2022 | xulong |
| v1.1.11 | Mars     | 增加 Mars 人型侦测 小模型                           | 08/08/2022 | xulong |
| v1.1.10 | Mars     | 拿掉venc remap                                      | 04/08/2022 | xulong |
| v1.1.9  | Mars     | 增加venc remap节省ION使用，此为workaround，预计8/15正式修正  | 03/08/2022 | xulong |
| v1.1.8  | Mars     | 1. 增加 CR1820 SBM config ini                       | 03/08/2022 | xulong |
| v1.1.7  | Mars     | 1. 增加人脸识别功能；2.替换fd新模型;3.上传mars modle文件| 02/08/2022 | zl.wan |
| v1.1.6  | Mars     | 1. 增加SBM ini; 2. 同步公版RC参数                   | 29/07/2022 | xulong |
| v1.1.5  | Mars     | 1. 修正跑Framebuff Mode 100%空指针问题<br>2. 简化非AI配置文件参数| 28/07/2022 | xulong    |
| v1.1.4  | cv182x   | FD支持新模型                                        | 28/07/2022 | Leo    |
| v1.1.3  | Mars     | 1. 修改子码流size为720*576<br>2. 修改主子码流bitrate为1K<br>3. 添加配置画框ini | 28/07/2022 | xulong    |
| v1.1.2  | Mars     | AI支持静态编译                                      | 26/07/2022 | Leo    |
| v1.1.1  | Both     | 1: 增加音频重采样;2: 音频支持静态编译;3.MP3优化     | 25/07/2022 | Leo    |
| v1.1.0  | Mars     | 1: 增加RGN画框；2: 增加Mars Slice Buff Mode         | 22/07/2022 | xulong |
| v1.0.19 | Both     | 增加Mp3 play 开关                                   | 19/07/2022 | zl.wan |
| v1.0.18 | Mars     | 增加glibc_riscv64版本的rtsp lib                     | 19/07/2022 | xulong |
| v1.0.17 | Both     | 增加PD Capture控制开关                              | 18/07/2022 | zl.wan |
| v1.0.16 | Mars     | 优化Makefile；修改 ini 有些成员名字错误             | 16/07/2022 | xulong |
| v1.0.15 | Mars     | 修改编译AI相关的Makefile；更新AI相关的静态库名称    | 15/07/2022 | xulong |
| v1.0.14 | Both     | 修改变量类型未定义引起的编译报错问题                | 14/07/2022 | xulong |
| v1.0.13 | Mars     | 修复CVI_ISP_SetStatisticsConfig调用报错             | 13/07/2022 | Leo    |
| v1.0.12 | Both     | mars支持MP3，修改MP3通用代码                        | 12/07/2022 | zl.wan |
| v1.0.11 | Both     | 上传mars pipeline图片，mars支持roi,整理通用代码     | 11/07/2022 | Leo    |
| v1.0.10 | Mars     | 优化Mars Makefile，并修改 ipcam_mars 默认为静态编译 | 11/07/2022 | xulong |
| v1.0.9  | Mars     | 修改Mars video pipeline ，dev1 output 3CHNs         | 11/07/2022 | xulong |
| v1.0.8  | Both     | 支持SD卡录像                                        | 11/07/2022 | Leo    |
| v1.0.7  | Mars     | 修正cmdTest不能在Mars平台上运行                     | 08/07/2022 | xulong |
| v1.0.6  | Both     | 优化face ae代码，整理AI代码                         | 05/07/2022 | Leo    |
| v1.0.5  | Mars     | 修改Mars app_ipcam_Ispd_Load所加载的so name         | 05/07/2022 | zl.wan |
| v1.0.4  | Both     | 修改设置PQ BIN API                                  | 05/07/2022 | xulong |
| v1.0.3  | Both     | 拿掉在audio source code中使用的 AUDIO_SUPPORT 宏    | 05/07/2022 | xulong |
| v1.0.2  | Both     | 修改ipcam退出流程                                   | 05/07/2022 | xulong |
| v1.0.1  | Both     | 修改Makefile获取platform方式                        | 05/07/2022 | xulong |
| v1.0.0  | Both     | 将Mercury和Mars应用分开                             | 04/07/2022 | xulong |
