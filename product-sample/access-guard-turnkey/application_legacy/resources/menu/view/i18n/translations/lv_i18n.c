#include "./lv_i18n.h"


////////////////////////////////////////////////////////////////////////////////
// Define plural operands
// http://unicode.org/reports/tr35/tr35-numbers.html#Operands

// Integer version, simplified

#define UNUSED(x) (void)(x)

static inline uint32_t op_n(int32_t val) { return (uint32_t)(val < 0 ? -val : val); }
static inline uint32_t op_i(uint32_t val) { return val; }
// always zero, when decimal part not exists.
static inline uint32_t op_v(uint32_t val) { UNUSED(val); return 0;}
static inline uint32_t op_w(uint32_t val) { UNUSED(val); return 0; }
static inline uint32_t op_f(uint32_t val) { UNUSED(val); return 0; }
static inline uint32_t op_t(uint32_t val) { UNUSED(val); return 0; }

static lv_i18n_phrase_t en_us_singulars[] = {
    {"Welcome to CviTek", "Welcome to CviTek"},
    {"Device Activate", "Device Activate"},
    {"Mode Switch", "Mode Switch"},
    {"Device Setting", "Device Setting"},
    {"Device Information", "Device Information"},
    {"Face Lib", "Face Lib"},
    {"Lang Switch", "Lang Switch"},
    {"Add Face Info", "Add Face Info"},
    {"Gallery", "Gallery"},
    {"Capture", "Capture"},
    {"Batch Register", "Batch Register"},
    {"Cancel", "Cancel"},
    {"Search", "Search"},
    {"Access Control", "Access Control"},
    {"update tips", "Don't cut off the power！"},
    {NULL, NULL} // End mark
};



static uint8_t en_us_plural_fn(int32_t num)
{
    uint32_t n = op_n(num); UNUSED(n);
    uint32_t i = op_i(n); UNUSED(i);
    uint32_t v = op_v(n); UNUSED(v);

    if ((i == 1 && v == 0)) return LV_I18N_PLURAL_TYPE_ONE;
    return LV_I18N_PLURAL_TYPE_OTHER;
}

static const lv_i18n_lang_t en_us_lang = {
    .locale_name = "en-US",
    .singulars = en_us_singulars,

    .locale_plural_fn = en_us_plural_fn
};

static lv_i18n_phrase_t zh_cn_singulars[] = {
    {"Welcome to CviTek", "北京晶视智能科技有限公司"},
    {"Device Activate", "设备激活"},
    {"Mode Switch", "模式切换"},
    {"Device Setting", "设备设置"},
    {"Device Information", "设备信息"},
    {"Face Lib", "用户管理"},
    {"Lang Switch", "语言切换"},
    {"Add Face Info", "添加人脸信息"},
    {"Gallery", "相册"},
    {"Capture", "拍照"},
    {"Batch Register", "本地批量注册"},
    {"Cancel", "取消"},
    {"Search", "搜索"},
    {"Access Control", "门禁控制"},
    {"Authorization", "认证"},
    {"Auth Method", "认证方式"},
    {"Device Interface", "设备接口"},
    {"Electronic Control", "继电器控制"},
    {"Wiegand", "韦根"},
    {"Com", "串口"},
    {"Com Enable", "串口使能"},
    {"Baud Rate", "波特率"},
    {"Bits", "数据位"},
    {"Check Mode", "校验模式"},
    {"Stop bit", "停止位"},
    {"Protocol", "协议"},
    {"Others", "其它"},
    {"Signal", "门磁检测"},
    {"Open Delay(ms)", "开门延时"},
    {"Open/Close Setting", "门动作"},
    {"Alarmer", "报警器"},
    {"Open Period(ms)", "开门时长"},
    {"FR Gap Time(ms)", "识别间隔"},
    {"Profile & Notification", "个性化与通知"},
    {"Profile", "个性化"},
    {"Voice Notice", "语音播报"},
    {"Name Display", "姓名显示"},
    {"Company Name", "公司名称"},
    {"Please Input Employee's Name:", "请输入员工名字"},
    {"Please Input Employee's ID:", "请输入员工工号"},
    {"Commit", "提交"},
    {"Setting", "设置"},
    {"Device Manage", "设备管理"},
    {"Door Button", "门禁按钮"},
    {"Uart", "串口"},
    {"Upgrade", "升级"},
    {"Connect", "连接"},
    {"Start", "开始"},
    {"Export", "导出"},
    {"Reboot", "重启"},
    {"Shutdown", "关机"},
    {"Reset", "恢复出厂设置"},
    {"Delete Face", "删除人脸"},
    {"Confirm", "确认"},
    {"Edit Face Info", "编辑人脸信息"},
    {"Name", "名字"},
    {"Serial", "序列"},
    {"type", "类型"},
    {"ic_card", "IC卡"},
    {"Time zone", "时区"},
    {"Manual date", "手动设置日期"},
    {"Syn NTP", "同步网络时间"},
    {"Language", "语言"},
    {"Log export", "日志导出"},
    {"System", "系统"},
    {"Display brightness", "显示亮度"},
    {"Registered people/Max", "已注册人数/上限"},
    {"About", "关于"},
    {"Boot time", "启动时间"},
    {"Version", "版本"},
    {"Device name", "设备名称"},
    {"Device type", "设备类型"},
    {"Device serial", "设备序列号"},
    {"Release time", "发布时间"},
    {"Mac address", "硬件地址"},
    {"Hardware", "硬件"},
    {"Ip address", "网络地址"},
    {"Broadcast address", "广播地址"},
    {"Netmask", "子网掩码"},
    {"Voice sound", "音量"},
    {"Enter display lock time", "进入显示锁定时间"},
    {"Enter display close time", "进入显示关闭时间"},
    {"Password", "密码"},
    {"Storage", "存储"},
    {"Occupancy/Remaining", "已使用/剩余"},
    {"Account", "账号"},
    {"Camera", "摄像机"},
    {"Version", "版本号"},
    {"Sn", "序列号"},
		{"factory test", "工厂测试"},
		{"pass", "通过"},
		{"fail", "失败"},
		{"LED Test", "LED测试"},
		{"LCD Test", "LCD测试"},
		{"TP Test", "TP测试"},
		{"Camera Test", "摄像头测试"},
		{"Touch Screen", "点击屏幕"},
		{"IR Test", "红外灯测试"},
		{"Sound Test", "喇叭测试"},
		{"Relay Test", "继电器测试"},
		{"Touch blue button", "点击蓝色按钮"},
		{"NFC test", "NFC测试"},
		{"Swipe your card", "请刷卡"},
		{"MIC test", "MIC测试"},
		{"WIFI test", "WIFI测试"},
		{"RS485/wg test", "RS485/韦根 测试"},
		{"Radar test", "雷达测试"},
		{"person come", "有人进来"},
		{"person leave", "有人离开"},
		{"audio testing", "喇叭测试中，点击按钮暂停"},
		{"audio stop", "喇叭暂停测试，点击按钮开始测试"},
		{"USB test", "USB测试"},
		{"input udisk", "请插入U盘"},
		{"check udisk", "已检测到U盘"},
		{"udisk out", "U盘已拔出"},
		{"Key test", "按键测试"},
		{"doorkey testing", "开门开关检测......"},
		{"doorkey test pass", "开门开关检测通过"},
		{"doormag testing", "门磁开关检测......"},
		{"doormag test pass", "门磁开关检测通过"},
		{"rm testing", "防拆开关检测......"},
		{"rm test pass", "防拆开关检测通过"},
		{"Alarm testing", "报警开关检测......"},
		{"Alarm test pass", "报警开关检测通过"},
		{"Alarm test pass", "报警开关检测通过"},
		{"signal", "信号"},
		{"signal level1", "信号等级强"},
		{"signal level2", "信号等级中"},
		{"signal level3", "信号等级弱"},
		{"cardNo", "卡号"},
		{"Test mode", "测试模式"},
		{"About Device", "关于设备"},
		{"Record Manage", "记录管理"},
		{"Net Manage", "网络管理"},
		{"Ethernet Set", "以太网设置"},
		{"WIFI Set", "WIFI设置"},
		{"Web Set", "服务器设置"},
		{"Mode", "模式"},
		{"Ip", "Ip地址"},
		{"NetMask", "子网掩码"},
		{"hand mode", "手动设置IP"},
		{"auto mode", "自动获取IP"},
		{"Gateway", "默认网关"},
		{"Dns", "Dns服务器"},
		{"Connected", "连接成功"},
		{"Connecting", "正在连接......"},
		{"OPEN WEB", "WEB 已打开"},
		{"CLOSE WEB", "WEB 已关闭"},
		{"web ip", "服务器地址"},
		{"register id", "注册号"},
		{"InOut", "出入口"},
		{"In", "进口"},
		{"Out", "出口"},
		{"In and Out", "进和出"},
		{"write mac and sn", "烧写MAC/SN"},
		{"write mac and sn", "烧写MAC/SN"},
		{"now mac", "当前MAC"},
		{"now sn", "当前SN"},
		{"write mac", "烧写的MAC"},
		{"write sn", "烧写的SN"},
		{"update", "软件升级"},
		{"load finish", "软件下载成功"},
		{"load fail", "软件下载失败"},
		{"update tips", "升级过程中，请不要断电！"},
		{"CLOSE WLAN", "WIFI已关闭"},
		{"OPEN WLAN", "WIFI已开启"},
		{"wifi ip", "WIFI 地址"},
		{"slipping", "滑动按钮，调节亮度"},
		{"stop", "暂停"},
		{"start", "播放"},
		{"recording", "正在录音......"},
		{"record off", "录音结束"},
		{"playing", "正在播放录音......"},
		{"play off", "录音播放停止"},
		{"start record", "开启录音"},
		{"Relay Off", "继电器已关"},
		{"Relay On", "继电器已开"},
		{"start mic", "开启MIC"},
		{"close mic", "关闭MIC"},
		{"mic on", "MIC已经开启"},
		{"mic off", "MIC已经关闭"},
		{"eth test", "以太网测试"},
		{"sound volume(0~15)", "喇叭音量(0~15)"},
		{"mic volume(0~7)", "MIC 音量(0~7)"},
		{"Down User", "用户下载"},
		{"Downloading users", "正在下载用户，请不要操作设备!"},
		{"Lcd brightness", "Lcd 亮度"},
		{"Verify Pass", "识别成功"},
		{"Verify Fail", "识别失败"},
		{"flash", "更新热点"},
		{"flashing", "正在搜索WIFI"},
		{"Test Result", "测试结束"},
		{"Test End", "测试完成，即将重启设备!"},
    {NULL, NULL} // End mark
};



static uint8_t zh_cn_plural_fn(int32_t num)
{



    return LV_I18N_PLURAL_TYPE_OTHER;
}

static const lv_i18n_lang_t zh_cn_lang = {
    .locale_name = "zh-CN",
    .singulars = zh_cn_singulars,

    .locale_plural_fn = zh_cn_plural_fn
};

const lv_i18n_language_pack_t lv_i18n_language_pack[] = {
    &en_us_lang,
    &zh_cn_lang,
    NULL // End mark
};

////////////////////////////////////////////////////////////////////////////////


// Internal state
static const lv_i18n_language_pack_t * current_lang_pack;
static const lv_i18n_lang_t * current_lang;


/**
 * Reset internal state. For testing.
 */
void __lv_i18n_reset(void)
{
    current_lang_pack = NULL;
    current_lang = NULL;
}

/**
 * Set the languages for internationalization
 * @param langs pointer to the array of languages. (Last element has to be `NULL`)
 */
int lv_i18n_init(const lv_i18n_language_pack_t * langs)
{
    if(langs == NULL) return -1;
    if(langs[0] == NULL) return -1;

    current_lang_pack = langs;
    current_lang = langs[0];     /*Automatically select the first language*/
    return 0;
}

/**
 * Change the localization (language)
 * @param l_name name of the translation locale to use. E.g. "en-GB"
 */
int lv_i18n_set_locale(const char * l_name)
{
    if(current_lang_pack == NULL) return -1;

    uint16_t i;

    for(i = 0; current_lang_pack[i] != NULL; i++) {
        // Found -> finish
        if(strcmp(current_lang_pack[i]->locale_name, l_name) == 0) {
            current_lang = current_lang_pack[i];
            return 0;
        }
    }

    return -1;
}


static const char * __lv_i18n_get_text_core(lv_i18n_phrase_t * trans, const char * msg_id)
{
    uint16_t i;
    for(i = 0; trans[i].msg_id != NULL; i++) {
        if(strcmp(trans[i].msg_id, msg_id) == 0) {
            /*The msg_id has found. Check the translation*/
            if(trans[i].translation) return trans[i].translation;
        }
    }

    return NULL;
}


/**
 * Get the translation from a message ID
 * @param msg_id message ID
 * @return the translation of `msg_id` on the set local
 */
const char * lv_i18n_get_text(const char * msg_id)
{
    if(current_lang == NULL) return msg_id;

    const lv_i18n_lang_t * lang = current_lang;
    const void * txt;

    // Search in current locale
    if(lang->singulars != NULL) {
        txt = __lv_i18n_get_text_core(lang->singulars, msg_id);
        if (txt != NULL) return txt;
    }

    // Try to fallback
    if(lang == current_lang_pack[0]) return msg_id;
    lang = current_lang_pack[0];

    // Repeat search for default locale
    if(lang->singulars != NULL) {
        txt = __lv_i18n_get_text_core(lang->singulars, msg_id);
        if (txt != NULL) return txt;
    }

    return msg_id;
}

/**
 * Get the translation from a message ID and apply the language's plural rule to get correct form
 * @param msg_id message ID
 * @param num an integer to select the correct plural form
 * @return the translation of `msg_id` on the set local
 */
const char * lv_i18n_get_text_plural(const char * msg_id, int32_t num)
{
    if(current_lang == NULL) return msg_id;

    const lv_i18n_lang_t * lang = current_lang;
    const void * txt;
    lv_i18n_plural_type_t ptype;

    // Search in current locale
    if(lang->locale_plural_fn != NULL) {
        ptype = lang->locale_plural_fn(num);

        if(lang->plurals[ptype] != NULL) {
            txt = __lv_i18n_get_text_core(lang->plurals[ptype], msg_id);
            if (txt != NULL) return txt;
        }
    }

    // Try to fallback
    if(lang == current_lang_pack[0]) return msg_id;
    lang = current_lang_pack[0];

    // Repeat search for default locale
    if(lang->locale_plural_fn != NULL) {
        ptype = lang->locale_plural_fn(num);

        if(lang->plurals[ptype] != NULL) {
            txt = __lv_i18n_get_text_core(lang->plurals[ptype], msg_id);
            if (txt != NULL) return txt;
        }
    }

    return msg_id;
}

/**
 * Get the name of the currently used locale.
 * @return name of the currently used locale. E.g. "en-GB"
 */
const char * lv_i18n_get_current_locale(void)
{
    if(!current_lang) return NULL;
    return current_lang->locale_name;
}
