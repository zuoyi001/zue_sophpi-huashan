# ChangeLog

| Version | Platform | Notes                                               | Date       | Author |
| ------- | -------- | --------------------------------------------------- | ---------- | ------ |
| v1.1.133| Phobos/mars | 1. 修改默认加载指定的websockets库文件            | 30/11/2022 | xulong |
| v1.1.132| Phobos/mars | 1. 自动ir切换优化                                | 30/11/2022 | zl.wan |
| v1.1.131| Phobos   | 1. OSDC开关概率段错误                               | 29/11/2022 | leo |
| v1.1.130| Both     | 1. 默认打包 cv1810c 38板相关配置到 install          | 29/11/2022 | xulong |
| v1.1.129| Both     | 1. Makefile中的一些变量使用？=的方式                  | 29/11/2022 | macro |
| v1.1.128| Phobos   | 1. JPG线程与Venc线程互不影响                        | 24/11/2022 | leo |
| v1.1.127| Phobos   | 1. jpg线程优先级修改 2.修改子码流flow 3.关闭cmd线程 | 23/11/2022 | leo |
| v1.1.126| Both     | 1. 修改切分辨率时没有停所有venc对应的grp            | 22/11/2022 | xulong |
| v1.1.125| Both     | 1. 修改静态库依赖顺序                               | 17/11/2022 | xulong |
| v1.1.124| Both     | 1. 修复切换编码格式overflow问题                     | 16/11/2022 | xulong |
| v1.1.123| Both     | 1.清除网页端切换子码流参数的复位标志                   | 16/11/2022 | macro |
| v1.1.122| Mars     | 1.修复ota升级失败问题 2.恢复误删代码                   | 15/11/2022 | macro |
| v1.1.121| Phobos   | 1.fix web下发空字符串段错误 2.fix 子码流码率设置无效| 14/11/2022 | leo |
| v1.1.120| Mars     | 1.修复设置保存隐私区域参数，保存后会恢复上一次设置值的问题 | 14/11/2022 | macro |
| v1.1.119| Both     | 1.修复重复调用app_ipcam_Vpss_Create创建相同grp出现的问题 | 14/11/2022 | macro |
| v1.1.118| Phobos   | 1.防止片段过多web显示失败                           | 11/11/2022 | leo |
| v1.1.117| Phobos   | 1.根据帧率去设置getstream的超时时间                 | 10/11/2022 | leo |
| v1.1.116| Phobos/Mars   | 1. 增大ir切换间隔时间                          | 10/11/2022 | zl.wan |
| v1.1.115| Phobos/Mars   | 1. 静态链接libstdc++.so.6,libgcc_s.so.1,libc.so,libatomic.so.1   | 10/11/2022 | xulong |
| v1.1.114| Phobos/Mars   | 1. 修复 修复切size后 人型检测失效              | 10/11/2022 | xulong |
| v1.1.113| Phobos/Mars   | 1. 修复 Mars && Phobos Audio 兼容编译报错      | 10/11/2022 | xulong |
| v1.1.112| Phobos/Mars   | 1. 修复 Mars && Phobos 兼容编译报错            | 10/11/2022 | xulong |
| v1.1.111| Phobos   | 1.加大venc get stream的超时时间，防止帧率降低时返回busy| 09/11/2022 | macro |
| v1.1.110| Phobos   | 1.限制播放的音频文件大小 2.web osd信息显示不匹配    | 09/11/2022 | leo |
| v1.1.109| Phobos   | 1.aac对讲段错误 2.p帧大于512K丢弃 3.killapp venc阻塞| 09/11/2022 | leo |
| v1.1.108| Phobos   | 1. tmp solution for AI thread switch ,vpss can't get VB  | 09/11/2022 | hongjun |
| v1.1.107| Phobos   | 1. 修正isp 改名导致ipcam编译报错                    | 31/10/2022 | xulong |
| v1.1.106| Phobos   | 1. 调整osdc顺序并设置cv1800b_38板IR_cut引脚(前提引脚复用设置正确) | 29/10/2022 | zl.wan |
| v1.1.105| Phobos   | 1. 默认不打开AI和Web                                | 29/10/2022 | xulong |
| v1.1.104| Phobos   | 1. 优化切size 速度                                  | 28/10/2022 | xulong |
| v1.1.103| Phobos   | 1. 修复图像偏色                                     | 27/10/2022 | leo |
| v1.1.102| Phobos   | 1.编译错误 2.修复图像设置无效                       | 27/10/2022 | leo |
| v1.1.101| Phobos   | 1.增大osdc区域的显示优先级                          | 26/10/2022 | zl.wan |
| v1.1.100| Phobos   | 1.播放音频aac文件，概率段错误                       | 26/10/2022 | leo |
| v1.1.99 | Phobos   | 1.打开音频对讲开rtsp段错误 2.killapp 概率阻塞       | 26/10/2022 | leo |
| v1.1.98 | Mars     | 1.修正Mars osdc 区域web界面控制错误                 | 25/10/2022 | zl.wan |
| v1.1.97 | Phobos   | 1.修正Phobos 38board ini 参数错误                   | 25/10/2022 | xulong |
| v1.1.96 | Both     | 1.可以在 ini 里配置是否开启SBM                      | 24/10/2022 | xulong |
| v1.1.95 | Both     | 1.新增phobos vi vi_vpss offline mode config ini     | 24/10/2022 | xulong |
| v1.1.94 | Phobos   | 1.修复isp 获取AE统计值概率段错误                    | 21/10/2022 | leo |
| v1.1.93 | Phobos   | 1.增加Phobos AI models                              | 21/10/2022 | xulong |
| v1.1.92 | Phobos   | 1.增加cv1800b_evb config ini                        | 21/10/2022 | xulong |
| v1.1.91 | Phobos/Mars   | 1.优化LL cache depth to 5 slice                | 20/10/2022 | xulong |
| v1.1.90 | Phobos   | 1.新增OTA功能                                        | 19/10/2022 | macro  |
| v1.1.89 | Phobos   | 1.优化切size速度                                    | 18/10/2022 | xulong |
| v1.1.88 | Phobos   | 1.支持抓拍                                          | 18/10/2022 | leo    |
| v1.1.87 | Phobos   | 1.ao能够立马停止播放                                | 17/10/2022 | leo    |
| v1.1.86 | Phobos   | 1.修复关闭OSD切换分辨率overflow                     | 17/10/2022 | leo    |
| v1.1.85 | Phobos   | 1.无图像参数oom 2.添加web OSDC关闭、OSDC文本        | 17/10/2022 | leo    |
| v1.1.84 | Phobos/Mars  | 1. 修正 killall ipcam_mars 系统异常问题         | 17/10/2022 | xulong |
| v1.1.83 | Phobos/Mars  | 1.默认关闭区域框 2.修复mars osdc                | 14/10/2022 | zl.wan |
| v1.1.82 | Phobos   | 注册thermal thread的回调，用于设置fps                 | 14/10/2022 | macro  |
| v1.1.81 | Phobos   | 1.子码流切分辨率图像异常 2.卡录片段太多段错误       | 14/10/2022 | leo    |
| v1.1.80 | Phobos   | 1.添加录像回放功能 2.fix bug录像索引文件不修复      | 14/10/2022 | leo    |
| v1.1.79 | Phobos   | 1. 修改数据缓存机制，init 与 deinit 对应            | 14/10/2022 | xulong |
| v1.1.78 | Phobos   | 1.音频添加数据处理缓存机制                          | 13/10/2022 | leo    |
| v1.1.77 | Phobos/Mars| 1.add gpio config 2.调整ai参数 3.支持phobos evb ir cut | 13/10/2022 | zl.wan |
| v1.1.76 | Phobos   | 1. add web osd control 2.killapp audio segmentfault | 13/10/2022 | leo |
| v1.1.75 | Phobos   | 1. fix warning                                      | 12/10/2022 | leo |
| v1.1.74 | Phobos   | 1. add phobos sub osd 2.default open audio          | 11/10/2022 | leo |
| v1.1.74 | Phobos   | 1. phobos unsupport face                            | 10/10/2022 | leo |
| v1.1.73 | Phobos   | 1. add phobos sub osd                               | 10/10/2022 | leo |
| v1.1.72 | Phobos   | 1. fix warning                                      | 10/10/2022 | hongjun |
| v1.1.71 | Phobos   | 1. add phobos osd                                   | 08/10/2022 | leo |
| v1.1.70 | Mars     | 1. 修改区域入侵关闭再打开多出一条线 2.降低ai帧率    | 08/10/2022 | zl.wan |
| v1.1.69 | Phobos   | 1. fix build warning                                | 06/10/2022 | xulong |
| v1.1.68 | Phobos   | 1. 增加Phobos config ini                            | 06/10/2022 | xulong |
| v1.1.67 | Mars     | 1. 增加数据处理缓存机制                              | 04/10/2022 | xulong |
| v1.1.66 | Mars     | 1. suooprt sc1346 30/60fps                          | 30/09/2022 | zl.wan |
| v1.1.65 | Mars     | 1. 修复rtsp重连 osdc线程段错误;                      | 30/09/2022 | leo |
| v1.1.64 | Mars     | 1. 修复rtsp重连段错误; 2.音频界面文字修改             | 30/09/2022 | leo |
| v1.1.63 | Mars     | 1. 添加vui                                          | 29/09/2022 | leo |
| v1.1.62 | Both     | 1. 修复cmdTest内存泄露; 2.修复pd加载失败退出段错误    | 28/09/2022 | leo |
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


