
#ifndef __CVI_UPGRADE_H__
#define __CVI_UPGRADE_H__

#include "cvi_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** error code define */
#define CVI_UPGRADE_EINTR            1
#define CVI_UPGRADE_EPKG_INVALID     2
#define CVI_UPGRADE_EPKG_OVERSIZE    3          /* Package oversize */
#define CVI_UPGRADE_EIMAGE_OVERSIZE  4          /* Partition image oversize*/
#define CVI_UPGRADE_EINVAL           5
#define CVI_UPGRADE_EPKG_UNMATCH     6          /* Package un-match to partition*/

#define CVI_COMM_STR_LEN 64
#define CVI_UPGRADE_MAX_PART_CNT 10
#define CVI_COMM_PATH_MAX_LEN 128

#define CVI_UPGRADE_EVENT_PROGRESS 1

/** upgrade device information */
typedef struct cviUPGRADE_DEV_INFO_S {
	CVI_CHAR szSoftVersion[CVI_COMM_STR_LEN]; /* Software version */
	CVI_CHAR szModel[CVI_COMM_STR_LEN];       /* Product model */
} CVI_UPGRADE_DEV_INFO_S;

/** upgrade package head */
typedef struct cviUPGRADE_PKG_HEAD_S {
	CVI_U32  u32Magic;
	CVI_U32  u32Crc;         /* CRC number from HeadVer to end of image-data */
	CVI_U32  u32HeadVer;     /* Package head version: 0x00000001 */
	CVI_U32  u32PkgLen;      /* Package total length, including head/data */
	CVI_U32  reserved0;
	CVI_CHAR szPkgModel[CVI_COMM_STR_LEN]; /* Package model, eg. cv1835_asic_wevb_0002a */
	CVI_CHAR szPkgSoftVersion[CVI_COMM_STR_LEN];   /* Package version, eg. 1.0.0.0 */
	CVI_CHAR reserved1[1024];
	CVI_U32  reserved2;
	CVI_S32  s32PartitionCnt;
	CVI_U32  au32PartitionOffSet[CVI_UPGRADE_MAX_PART_CNT]; /* Partition offset in upgrade package */
} CVI_UPGRADE_PKG_HEAD_S;

/** upgrade event struct */
typedef struct cviUPGRADE_EVENT_S {
	CVI_U32  eventID;
	CVI_VOID *argv;
} CVI_UPGRADE_EVENT_S;

CVI_S32 CVI_UPGRADE_Init(void);

CVI_S32 CVI_UPGRADE_Deinit(void);

CVI_S32 CVI_UPGRADE_CheckPkg(const CVI_CHAR *pszPkgUrl, const CVI_UPGRADE_DEV_INFO_S *pstDevInfo, CVI_BOOL bCheckVer);

CVI_S32 CVI_UPGRADE_DoUpgrade(const CVI_CHAR *pszPkgUrl);

void CVI_UPGRADE_RegisterEvent(void (*eventCb)(CVI_UPGRADE_EVENT_S *));


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __CVI_UPGRADE_H__ */

