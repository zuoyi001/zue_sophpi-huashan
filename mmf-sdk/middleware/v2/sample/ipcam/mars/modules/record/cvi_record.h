#ifndef __CVI_RECORD_H__
#define __CVI_RECORD_H__

#ifdef __cplusplus
    #define PSS_API extern "C"
#else
    #define PSS_API
#endif
#define API_CALL

#include "time.h"
#include "cvi_type.h"


#ifndef HANDLE
typedef void* HANDLE;
#endif

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef INOUT
#define INOUT
#endif

/* level setting */
#define RECORD_LEVEL_ERROR                 0 /*min print info*/
#define RECORD_LEVEL_WARN                  1
#define RECORD_LEVEL_INFO                  2
#define RECORD_LEVEL_DEBUG                 3 /*max print info*/
#define RECORD_LEVEL_TRACE                 (RECORD_LEVEL_DEBUG+1) /*for DBG_PRINT use*/

#define RECORD_LEVEL            RECORD_LEVEL_DEBUG  //use level

/*系统错误码定义*/
#define ERR_NO                    0x00000000
#define ERR_HANDLE_ALLOC_ERROR    0xc0000100
#define ERR_BUF_CHECK_ERROR       0xc0000101    //检验失败
#define ERR_OPEN_FILE_ERROR       0xc0000102
#define ERR_INVALID_HANDLE        0xc0000103
#define ERR_INVALID_ARGUMENT      0xc0000105
#define ERR_INVALID_FILENAME      0xc0000108
#define ERR_SYS_NOT_INIT          0xc000010D

//起始码
#define CVI_RECORD_VERSION                   (0)                  //录像版本号
#define CVI_RECORD_FILE_STARTCODE            (0X4F3E2D1C)         //文件起始码
#define CVI_RECORD_SEGMENT_STARTCODE         (0X1C2D3E4F)         //数据片段起始码

//写数据标记暂不使用
#define CVI_RECORD_WRITE_INVALID         (0x00000000)            //无效数据
#define CVI_RECORD_WRITE_WRITTING        (0X1234CDEF)            //写开始标志用于区分改片段是否被改写未完成
#define CVI_RECORD_WRITE_WRITTOK         (0XCDEF1234)            //写完成标志用于区分是否写完成

#define CVI_RECORD_PRE_RECORD_TIMES       (5)        //默认预录15秒
#define CVI_RECORD_MAX_RECORD_READER      (3)        //最大创建3个读回放接口
#define CVI_RECORD_MAX_SKIP_NO            (60)       //帧跳跃最大间隔,超过需要等待索引帧

#define CVI_RECORD_EVENT_ALL_ALARM (0)    //所有报警

//枚举定义
typedef enum {
    CVI_RECORD_EVENT_ALL_ALARM_MASK = 0xFFFFFFFF,    //所有报警
    CVI_RECORD_EVEN_NULL_MASK = 0,                  //人形报警录像1<<0
    CVI_RECORD_EVEN_HUMEN_ALARM_MASK = 1,           //人形报警录像1<<0
    CVI_RECORD_EVEN_FACE_ALARM_MASK = 2,            //人脸报警录像1<<1
    CVI_RECORD_EVEN_MODE_MAX
}CVI_RECORD_EVENT_MASK_E;

//录像事件如果0为无录像对应位为1时有效
#define CVI_RECORD_EVEN_CHECK_MASK(iEven, iMask)   (iEven&(1<<iMask))     //检测是否有对应人形报警事件
#define CVI_RECORD_EVEN_SET_MASK(iEven, iMask)     (iEven|(1<<iMask))     //设置对应报警事件
#define CVI_RECORD_EVEN_CLEAR_MASK(iEven, iMask)   (iEven&(~(1<<iMask)))     //设置对应报警事件
#define CVI_RECORD_EVEN_NONE                       (1<<CVI_RECORD_EVEN_NULL_MASK)     //无事件录像

//枚举定义注意使用时反向写入
typedef enum {
    CVI_RECORD_FRAME_HEADER_INFO_KEY1 = 0,   //关键索引帧1
    CVI_RECORD_FRAME_HEADER_INFO_KEY0 = 1,   //关键索引帧0
    CVI_RECORD_FRAME_HEADER_INFO_HEAD = 2,   //头head
    CVI_RECORD_FRAME_HEADER_INFO_MAX
}CVI_RECORD_FRAME_HEADER_INFO_E;

#define CVI_RECORD_WRITE_INT                                   (10 * 1000)   //每次写文件延迟间隔不能太大
#define CVI_RECORD_FORMAT_PRE_CREATE_INT                       (100 * 1000)  //创建文件延迟间隔
#define CVI_RECORD_FORMAT_PRE_WRITE_INT                        (500 * 1000)  //格式化写文件延迟间隔

#define  CVI_RECORD_FRAME_DATA_INFO_WRITE_MAX_COUNT     (200) //每次缓存数据索引帧最大数 大概10秒左右写一次索引
#define  CVI_RECORD_FRAME_DATA_INFO_WRITE_MAX_SIZE      (CVI_RECORD_FRAME_DATA_INFO_WRITE_MAX_COUNT*sizeof(CVI_RECORD_SEGMENT_INFO))
#define  CVI_RECORD_FRAME_DATA_INFO_WRITE_INDEX(index)  (CVI_RECORD_FRAME_DATA_INFO_WRITE_MAX_COUNT - index - 1) //反向写入

#define  CVI_RECORD_FRAME_DATA_INFO_READ_MAX_COUNT     (200) //每次缓存数据索引帧最大数 大概10秒左右写一次索引
#define  CVI_RECORD_FRAME_DATA_INFO_READ_MAX_SIZE      (CVI_RECORD_FRAME_DATA_INFO_READ_MAX_COUNT*sizeof(CVI_RECORD_SEGMENT_INFO))

#define  CVI_RECORD_FRAME_DATA_INFO_READ_MAX_REPAIR_SIZE      (2000*sizeof(CVI_RECORD_SEGMENT_INFO))  //修复索引时buf



#define CVI_RECORD_KEYINDEX_PER_NUMS         (16)        //short 类型，对齐32位不能修改.
#define CVI_RECORD_KEYINDEX_NUM              (2)         //数据片段起始码,每个片段最大2个KEYINDEX，所以最大32个索引帧
#define CVI_RECORD_KEYINDEX_MAX_NUM          (CVI_RECORD_KEYINDEX_NUM*CVI_RECORD_KEYINDEX_PER_NUMS)         //关键帧最大个数
#define CVI_RECORD_MAX_PER_SEGMENT_TIME      (60)        //写每片段最长时间防止断电丢录像多
#define CVI_RECORD_MAX_PER_SKIP_TIME         (2)         //写录像时间帧间隔大于这个则跳帧

#define CVI_RECORD_FORMAT_FLAG      "CvitekFormat.cvitek"         //录像格式化标记
#define CVI_RECORD_INDEX_FILE       "CvitekStart%02d.cvitek"      //索引文件
#define CVI_RECORD_INDEX_FILE_BAK   "CvitekStart%02d_bak.cvitek"  //索引备份文件
#define CVI_RECORD_AV_FILE          "CvitekAV%04d.cvitek"         //录像文件

#define CVI_RECORD_ALL_FILE         "*.cvitek"              //格式化失败时删除的文件



#define CVI_RECORD_CHAR_TO_MB               (1024*1024)    //字符转MB
#define CVI_RECORD_INDEX_FILE_COUNT         (2)            //索引文件有备份所以有2份
#define CVI_RECORD_INDEX_FILE_MAX_SIZE      (32 * CVI_RECORD_CHAR_TO_MB) //索引文件大小

//1280+64*avfiles+64*CVI_RECORD_AV_FILE_MAX_SEGMENT*avfiles <= CVI_RECORD_INDEX_FILE_MAX_SIZE
//avfiles <= 511个文件
#define CVI_RECORD_INDEX_MAX_AVFILES        (511)           //每个索引文件最大支持多少个视频文件
#define CVI_RECORD_AV_FILE_MAX_SEGMENT      (1024)          //每个文件最大片段数
#define CVI_RECORD_AV_FILE_MAX_SIZE         (256 * CVI_RECORD_CHAR_TO_MB) //256M每个文件
#define CVI_RECORD_REMAIN_FILE_MIN_SIZE     (CVI_RECORD_AV_FILE_MAX_SIZE) //剩余最少256M空间

#define CVI_RECORD_MAX_PATH_NAME       (128)   //最大路径名长度

#define PKG_FRAME_MAX_LEN              (1 * 1024 * 1024) //最大帧 每次写IO最大BUF，避免太大为输入异常
#define PKG_READ_DEFAULT_FRAME_LEN     (300 * 1024)      //默认申请读buf大小超过这个范围会重新创建
#define PKG_WRITE_BUFFER_MAX_LEN       (512 * 1024)      //缓存写BUF大小
#define PKG_WRITE_BUFFER_TMP_MAX_LEN   (64 * 1024)       //临时使用写数据缓冲

#define PKG_HEADER_INDEX_MAX_LEN     (sizeof(CVI_RECORD_FILE_INDEX_HEADER))
#define PKG_RECORD_INDEX_MAX_LEN     (CVI_RECORD_INDEX_MAX_AVFILES*sizeof(CVI_RECORD_FILE_INDEX_RECORD))
#define PKG_SEGMENT_INDEX_MAX_LEN    (CVI_RECORD_AV_FILE_MAX_SEGMENT*sizeof(CVI_RECORD_SEGMENT_INDEX_RECORD))
#define PKG_FILE_INDEX_MAX_LEN       (PKG_HEADER_INDEX_MAX_LEN + PKG_RECORD_INDEX_MAX_LEN + PKG_SEGMENT_INDEX_MAX_LEN)


//枚举定义
typedef enum {
    CVI_RECORD_WRITE_MODE_ALL = 0, //全
    CVI_RECORD_WRITE_MODE_EVENT,   //事件
    CVI_RECORD_WRITE_MODE_NONE,    //不录像
    CVI_RECORD_WRITE_MODE_MAX
}CVI_RECORD_WRITE_MODE_E;

//枚举定义
typedef enum {
    CVI_RECORD_RUN_MODE_NORMAL = 0,    //正常状态
    CVI_RECORD_RUN_MODE_STOP,          //停止状态
    CVI_RECORD_RUN_MODE_MAX
}CVI_RECORD_RUN_MODE_E;

typedef enum
{
    CVI_STORAGE_STATUS_ERROR = 0,     //异常状态
    CVI_STORAGE_STATUS_NORMAL,        //正常状态
    CVI_STORAGE_STATUS_UNINIT,        //未初始化
    CVI_STORAGE_STATUS_FORMAT,        //格式化中
    CVI_STORAGE_STATUS_STOP,          //停止状态
    CVI_STORAGE_STATUS_MAX
}CVI_STORAGE_STATUS_E;


//索引状态
typedef enum
{
    CVI_RECORD_STATUS_NORMAL   = 1,      //状态正常写入完成
    CVI_RECORD_STATUS_LOCK     = 2,      //状态锁定写入中
    CVI_RECORD_STATUS_BAD_FILE = 0xE5,   //无效坏道文件类型
    CVI_RECORD_STATUS_INIT     = 0xFE,   //初始化状态
}CVI_RECORD_STATUS_E;

//附件信息帧类型
typedef enum
{
	CVI_RECORD_INFO_TYPE_HEADER    = 1,     //segment 头信息类型
    CVI_RECORD_INFO_TYPE_KEY_INDEX = 2,     //segment 头索引信息类型
    CVI_RECORD_INFO_TYPE_FRAME     = 3,     //帧信息
    CVI_RECORD_INFO_TYPE_INVALID   = 0xFD,  //无效信息类型
    CVI_RECORD_INFO_TYPE_INIT      = 0xFE,  //初始化状态
}CVI_RECORD_INFO_TYPE_E;


/* 录像文件索引头 ,总字节1280*/
typedef struct
{
    CVI_U32                        iFileStartCode;                     //索引文件起始码CVI_RECORD_FILE_STARTCODE
    CVI_U64                        iModifyTimes;                       //对应文件修改次数
    CVI_S32                        iVersion;                           //当前版本号
    CVI_S32                        iAVFiles;                           //对应文件数
    CVI_S32                        iCurrFileRecNo;                     //当前写入文件
    CVI_CHAR                       iRes1[1252];                        //保留字节
    CVI_S32                        iCrcSum;                            //前面字节相加校验和
}__attribute__((packed)) CVI_RECORD_FILE_INDEX_HEADER, *PCVI_RECORD_FILE_INDEX_HEADER;


/*对应文件的录像索引数据 每个录像文件Rec00XXX.cvitek都有1个对应索引数据 ,总字节64*/
typedef struct
{
    CVI_CHAR                       iStatus;                            //状态路上CVI_RECORD_STATUS_E
    CVI_CHAR                       iRes1[1];                           //保留字段 
    CVI_U16                        iSegRecNums;                        //文件包含的有效总片段数
    time_t                         tBeginTime;                         //录像起始时间
    time_t                         tEndTime;                           //录像结束时间
    CVI_CHAR                       iRes2[40];                          //保留字段
    CVI_S32                        iCrcSum;                            //前面字节相加校验和
}__attribute__((packed)) CVI_RECORD_FILE_INDEX_RECORD, *PCVI_RECORD_FILE_INDEX_RECORD;


/**************************************************************************************************
文件录像片段索引头总长度64

对应每个录像文件文件都有最大256个片段，对应片段的索引数据

1 剩余空间不足当前帧时划分片段。
2 每种录像类型划分片段。
3 时间不连贯划分片段（时间修改等原因）。
4 修改分辨率，编码参数时划分片段。
5 索引帧大于最大数（32）时划分片段。
****************************************************************************************************/
typedef struct
{
    CVI_S32                        iEvenType;                          //片段录像类型普通，人形告警CVI_RECORD_EVEN_NONE
    CVI_CHAR                       iStatus;                            //状态CVI_RECORD_STATUS_E
    CVI_CHAR                       iRes1[3];                           //保留字节
    time_t                         tBeginTime;                         //起始时间
    time_t                         tEndTime;                           //结束时间
    CVI_U32                        iFrameStartOffset;                  //片段开始偏移
    CVI_U32                        iFrameEndOffset;                    //片段开始偏移 
    CVI_U32                        iInfoCount;                         //片段附件个数 = 帧数 + 头信息数(1 + 2)
    CVI_U32                        iInfoEndOffset;                     //片段信息开始偏移的结束偏移
    CVI_U32                        iSegmentNo;                         //当前写segment次数累加，用于查找上个I帧片段使用
    CVI_CHAR                       iRes2[16];                          //保留数据，索引长度固定
    CVI_S32                        iCrcSum;                            //校验和
}__attribute__((packed)) CVI_RECORD_SEGMENT_INDEX_RECORD, *PCVI_RECORD_SEGMENT_INDEX_RECORD;


/* 录像片段帧信息 32 字节*/
typedef struct
{
    CVI_S32                 iEvenType;                          //片段录像类型普通，人形告警CVI_RECORD_EVEN_NONE
    CVI_CHAR                iStatus;                            //状态CVI_RECORD_STATUS_E
    CVI_CHAR                iRes1[3];                           //保留字节
    CVI_U16                 iInfoExNums;                        //额外Info信息帧数量
    CVI_U16                 iIndexFrameNums;                    //额外索引帧数量
    CVI_U32                 iSegmentNo;                         //片段帧数
    CVI_CHAR                iRes2[16];                          //保留字节
}__attribute__((packed)) CVI_RECORD_HEADER_INFO, *PCVI_RECORD_HEADER_INFO;

/* 关键帧索引信息 32 字节*/
typedef struct
{
    CVI_U16      iIndexFrame[CVI_RECORD_KEYINDEX_PER_NUMS];      // 关键帧索引对齐32个字节所以是16，0标识无关键帧，从1开始
}__attribute__((packed)) CVI_RECORD_INDEX_INFO, *PCVI_RECORD_INDEX_INFO;

/* 录像数据帧 信息 32 字节*/
typedef struct
{
    CVI_CHAR                iFrameType;                         //帧类型，264 265 视频，音频，图片等 CVI_RECORD_FRAME_TYPE_E
    CVI_CHAR                iSubType;                           //流类型，I/P/SP;G711U/A;JPEG/PNG等CVI_RECORD_H265_TYPE_E
    CVI_CHAR                iRes1[2];                           //保留字节
    CVI_U32                 iFrameNo;                           //帧序号
    CVI_U32                 iStartOffset;                       //文件帧偏移量
    CVI_U32                 iFrameLen;                          //文件真结束偏移量
    CVI_U64                 iFrame_absTime;                     //绝对时标
    time_t                  iShowTime;                          //帧时间
}__attribute__((packed)) CVI_RECORD_FRAME_INFO, *PCVI_RECORD_FRAME_INFO;


/* 录像片段附件信息 44字节*/
typedef struct
{
    CVI_U32                 iSegmentStartCode;                  //起始码CVI_RECORD_SEGMENT_STARTCODE
    CVI_CHAR                iInfoType;                          //片段数据信息 CVI_RECORD_INFO_TYPE_E
    CVI_CHAR                iRes[3];                            //保留字节
    union
    {
        CVI_RECORD_HEADER_INFO   iHeaderInfo;                    //索引头
        CVI_RECORD_INDEX_INFO    iIndexInfo;                     //索引信息
        CVI_RECORD_FRAME_INFO    iFrameInfo;                     //数据帧
    };
    CVI_S32                 iCrcSum;                            //校验和
}__attribute__((packed)) CVI_RECORD_SEGMENT_INFO, *PCVI_RECORD_SEGMENT_INFO;

//录像帧类型存储库只有char存储8位
typedef enum
{
    CVI_RECORD_FRAME_TYPE_START   = 0,
	CVI_RECORD_FRAME_TYPE_H264    = 1,     //264
    CVI_RECORD_FRAME_TYPE_H265    = 2,     //265
    CVI_RECORD_FRAME_TYPE_G711U   = 3,     //音频G711U
    CVI_RECORD_FRAME_TYPE_MAX,
}CVI_RECORD_FRAME_TYPE_E;

//录像视频帧类型存储库只有char存储8位
typedef enum
{
	CVI_RECORD_H264_TYPE_P_FRAME    = 0,     //P帧
    CVI_RECORD_H264_TYPE_I_FRAME    = 1,     //I帧
    CVI_RECORD_H264_TYPE_MAX,
}CVI_RECORD_H264_TYPE_E;

//录像视频帧类型存储库只有char存储8位
typedef enum
{
	CVI_RECORD_H265_TYPE_P_FRAME    = 0,     //P帧
    CVI_RECORD_H265_TYPE_I_FRAME    = 1,     //I帧
    CVI_RECORD_H265_TYPE_SP_FRAME   = 2,    //SP帧
    CVI_RECORD_H265_TYPE_MAX,
}CVI_RECORD_H265_TYPE_E;

/* 录像数据回调 信息 */
typedef struct
{
    CVI_CHAR                iFrameType;                         //帧类型，264 265 视频，音频，图片等 CVI_RECORD_FRAME_TYPE_E
    CVI_CHAR                iSubType;                           //流类型，I/P/SP;等, CVI_RECORD_H264_TYPE_E, CVI_RECORD_H265_TYPE_E
    CVI_U32                 iFrameNo;                           //帧序号
    CVI_U32                 iFrameLen;                          //帧长度
    CVI_U64                 iFrame_absTime;                     //绝对时标
    time_t                  iShowTime;                          //帧时间
    CVI_U8*                 pBuf;                               //帧数据
}T_FRAME_INFO, *PT_FRAME_INFO;


/*录像片断信息结构体*/
typedef struct _PSS_REGSEG
{
    time_t             tBeginTime; //起始时间点
    time_t             tEndTime;   //结束时间点
    CVI_S32            iEvenType;   //该时间段的录像类型
    struct _PSS_REGSEG *ptNext;
}CVI_RECORD_RECORD_TS_S, *PCVI_RECORD_RECORD_TS_S;


/*****************************************************************************
 函 数 名  : CVI_RECORD_Init
 功能描述  : 初始化存储库
 输入参数  : CVI_CHAR *pPkgPathName 存储的路径
 输出参数  : 0
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_RECORD_Init(IN CVI_CHAR *pPkgPathName);

/*****************************************************************************
 函 数 名  : CVI_RECORD_uninit
 功能描述  : 去初始化存储库
 输入参数  : 
 输出参数  : 0
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_RECORD_Uninit();

/*****************************************************************************
 函 数 名  : CVI_RECORD_format
 功能描述  : 格式化存储路径
 输入参数  : 0
 输出参数  : 0
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_RECORD_Format(CVI_U32 iFileSize);

/*****************************************************************************
 函 数 名  : CVI_RECORD_resume
 功能描述  :设置存储库状态开始
 输入参数  : 
 输出参数  : 
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_RECORD_Resume();

/*****************************************************************************
 函 数 名  : CVI_RECORD_pause
 功能描述  :设置存储库状态停止
 输入参数  : 
 输出参数  : 0
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_RECORD_Pause();

/*****************************************************************************
 函 数 名  : CVI_RECORD_dataInput
 功能描述  :输入录像数据
 输入参数  : 
 PT_FRAME_INFO  pstFrameInfo   帧数据参数
 输出参数  : 0
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_RECORD_DataInput(PT_FRAME_INFO  pstFrameInfo);


/*****************************************************************************
 函 数 名  : CVI_RECORD_get_status
 功能描述  :获取存储库状态
 输入参数  : 
 输出参数  : 
 OUT CVI_STORAGE_STATUS_E* iPKGStatus 获取状态 OUT CVI_S32 iPrecent格式化百分比
 OUT CVI_U32 *iTotalSize, OUT CVI_U32 *iUseSize 总容量及剩余容量单位MB
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_RECORD_GetStatus(OUT CVI_STORAGE_STATUS_E* iPKGStatus, OUT CVI_S32* iPrecent, OUT CVI_U32 *iTotalSize, OUT CVI_U32 *iRemainSize);

/*****************************************************************************
 函 数 名  : CVI_RECORD_set_write_mode
 功能描述  : 存储库写模式
 输入参数  : IN CVI_RECORD_WRITE_MODE_E iWriteMode 不录像，事件录像，全天录像
 输出参数  : 0
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_RECORD_SetWriteMode(IN CVI_RECORD_WRITE_MODE_E iWriteMode);

/*****************************************************************************
 函 数 名  : CVI_RECORD_set_pre_record_time
 功能描述  :设置预录时间
 输入参数  : IN CVI_U32 iPreTimes 预录时间
 输出参数  : 0
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_RECORD_SetPreRecordTime(IN CVI_U32 iPreTimes);

/*****************************************************************************
 函 数 名  : CVI_RECORD_start_event
 功能描述  :触发事件
 输入参数  : IN CVI_S32 iEvenType 触发的事件类型
 输出参数  : 0
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_RECORD_StartEvent(IN CVI_RECORD_EVENT_MASK_E iEvenMask);

/*****************************************************************************
 函 数 名  : CVI_RECORD_stop_event
 功能描述  :结束事件
 输入参数  : IN CVI_S32 iEvenType 触发的事件类型
 输出参数  : 0
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_RECORD_StopEvent(IN CVI_RECORD_EVENT_MASK_E iEvenType);


/*****************************************************************************
 函 数 名  : CVI_RECORD_pb_query_by_month
 功能描述  :查询这个月是否有录像
 输入参数  : IN CVI_U32 query_year, IN CVI_U32 query_month 年月
 输出参数  : OUT CVI_U32 *pMonthDays 32位，每位代码对应天是否有录像
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_REPLAY_QueryByMonth(IN CVI_U32 query_year, IN CVI_U32 query_month, OUT CVI_U32 *pMonthDays);

/*****************************************************************************
 函 数 名  : CVI_RECORD_pb_query_by_day
 功能描述  :查询当天录像
 输入参数  :
 IN CVI_S32 year, IN CVI_S32 month, IN CVI_S32 day 年月日
 iN bTimeRec 是否查询定时录像
 iEvenType 事件录像
 输出参数  : OUT CVI_RECORD_RECORD_TS_S **pTsList返回录像片段
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_REPLAY_QueryByDay(IN CVI_U32 query_year, IN CVI_U32 query_month, IN CVI_U32 query_day, IN CVI_U32 iEvenType, OUT CVI_RECORD_RECORD_TS_S **pRecSegHead, OUT CVI_S32 *pRecSegCount);

/*****************************************************************************
 函 数 名  : CVI_RECORD_pb_query_free_ts_arr
 功能描述  :释放获取的录像片段
 输入参数  : IN CVI_RECORD_RECORD_TS_S *pTsList 录像片段
 输出参数  : 
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_REPLAY_QueryFreeTsArr(IN CVI_RECORD_RECORD_TS_S *pTsList);

//枚举定义
typedef enum {
    CVI_REPLAY_CB_NONE = 0,     //全
    CVI_REPLAY_CB_FINISH,       //事件
    CVI_REPLAY_CB_ERROR,        //不录像
    CVI_REPLAY_CB_MAX
}CVI_REPLAY_CB_EVENT_E;

//函数类定义
/*****************************************************************************
HANDLE hDataPopper     句柄
PT_FRAME_INFO iFrameInfo  回调数据
unsigned long EventID    事件
*****************************************************************************/
typedef CVI_S32 (*CVI_REPLAY_DATAPOP_CALLBACK)(IN HANDLE hDataPopper, IN PT_FRAME_INFO pFrameInfo, CVI_REPLAY_CB_EVENT_E EventID);

/*****************************************************************************
 函 数 名  : CVI_RECORD_pb_create
 功能描述  :创建回放句柄
 输入参数  : 
 IN time_t tStartSeekTime  开始时间
 IN time_t tEndSeekTime   结束时间
 IN CVI_S32 iEvenType             事件
 IN T_REPLAY_DATAPOP_CALLBACK cbReplayCallback   数据回调
 输出参数  : 
 返 回 值  : HANDLE 句柄 NULL失败
*****************************************************************************/
PSS_API HANDLE API_CALL CVI_REPLAY_Create(IN time_t tStartSeekTime, IN time_t tEndSeekTime, IN CVI_U32 iEvenType, IN CVI_REPLAY_DATAPOP_CALLBACK cbReplayCallback);


/*****************************************************************************
 函 数 名  : CVI_RECORD_pb_create
 功能描述  :创建回放句柄
 输入参数  : 
 IN HANDLE hDataPopper                    句柄
 IN CVI_U32 PopKeyInterval           I间隔0全部弹出
 输出参数  : 
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_REPLAY_DataSetKeyFrame(IN HANDLE hDataPopper, IN CVI_U32 PopKeyInterval);

/*****************************************************************************
 函 数 名  : CVI_RECORD_pb_start
 功能描述  :开始回放
 输入参数  : 
 IN HANDLE hDataPopper                    句柄
 输出参数  : 
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_REPLAY_Start(IN HANDLE hDataPopper);

/*****************************************************************************
 函 数 名  : CVI_RECORD_pb_stop
 功能描述  :停止回放
 输入参数  : 
 IN HANDLE hDataPopper                    句柄
 输出参数  : 
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_REPLAY_Stop(IN HANDLE hDataPopper);

/*****************************************************************************
 函 数 名  : CVI_REPLAY_Release
 功能描述  :销毁回放
 输入参数  : 
 IN HANDLE hDataPopper                    句柄
 输出参数  : 
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_REPLAY_Release(IN HANDLE hDataPopper);


/*****************************************************************************
 函 数 名  : CVI_RECORD_pb_seek
 功能描述  :销毁回放
 输入参数  : 
 IN HANDLE hDataPopper                    句柄
 IN  time_t SeekTime                         弹出数据时间
 输出参数  : 
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_REPLAY_Seek(IN HANDLE hDataPopper,IN  time_t SeekTime);

/*****************************************************************************
 函 数 名  : CVI_RECORD_show_file_info
 功能描述  :显示信息log
 输入参数  : 
 输出参数  : 0
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_RECORD_ShowRecordInfo(CVI_U32 iCurrFileRecNo);

/*****************************************************************************
 函 数 名  : CVI_REPLAY_Resume
 功能描述  :继续回放
 输入参数  : 
 IN HANDLE hDataPopper                    句柄
 输出参数  : 
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_REPLAY_Resume(IN HANDLE hDataPopper);

/*****************************************************************************
 函 数 名  : CVI_REPLAY_Pause
 功能描述  :暂停回放
 输入参数  : 
 IN HANDLE hDataPopper                    句柄
 输出参数  : 
 返 回 值  : 0
*****************************************************************************/
PSS_API CVI_S32 API_CALL CVI_REPLAY_Pause(IN HANDLE hDataPopper);

PSS_API CVI_S32 API_CALL cvi_system(IN const CVI_CHAR *pszCmd);

#endif

