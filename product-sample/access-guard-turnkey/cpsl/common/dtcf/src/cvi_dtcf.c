#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>

#include "cvi_dtcf.h"
#include "cvi_log.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

// default directory name
#define CVI_EMR_DIR          "EMR"
#define CVI_EMR_S_DIR        "EMR_s"
#define CVI_MOVIES_DIR       "Movie"
#define CVI_MOVIES_S_DIR     "Movie_s"
#define CVI_PARK_DIR         "Park"
#define CVI_PARK_S_DIR       "Park_s"
#define CVI_EMR_B_DIR        "EMR_b"
#define CVI_EMR_B_S_DIR      "EMR_b_s"
#define CVI_MOVIES_B_DIR     "Movie_b"
#define CVI_MOVIES_B_S_DIR   "Movie_b_s"
#define CVI_PARK_B_DIR       "Park_b"
#define CVI_PARK_B_S_DIR     "Park_b_s"
#define CVI_PHOTO_DIR        "Photo"
#define CVI_PHOTO_B_DIR      "Photo_b"

#define CVI_DTCF_DIR_MODE    (0760)

#define CVI_FILE_SUFFIX_LEN_MAX       (8)

#define CVI_DTCF_TIME_STR_LEN         (20)
#define CVI_FILE_NAME_LEN_MIN         (4) //e.g. .mp4


#define CVI_DTCF_SCAN_FILE_AMOUNT_MAX (10 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2 * 2) //10240

#define CHECK_DTCF_NULL_PTR(ptr)\
    do{\
        if(NULL == ptr)\
        {\
            CVI_LOGE("DTCF NULL pointer\n" );\
            return CVI_DTCF_ERR_NULL_PTR;\
        }\
    }while(0)

#define CHECK_DTCF_STATUS_CREATED(status)\
    do{\
        if(DTCF_STATUS_CREATED != status && DTCF_STATUS_SCANED != status )\
        {\
            CVI_LOGE("status is not DTCF_STATUS_CREATED|DTCF_STATUS_SCANED, current status:%d\n", s_enDtcfStatu);\
            return CVI_DTCF_ERR_STATUS_ERROR;\
        }\
    }while(0)

#define CHECK_DTCF_STATUS_SCANED(status)\
    do{\
        if(DTCF_STATUS_SCANED != status)\
        {\
            CVI_LOGE("status is not DTCF_STATUS_SCANED, current status:%d\n", s_enDtcfStatu);\
            return CVI_DTCF_ERR_STATUS_ERROR;\
        }\
    }while(0)

#define CHECK_DTCF_ENDIR_RANGE(enDir)\
    do{\
        if(enDir >= DTCF_DIR_BUTT)\
        {\
            CVI_LOGE("DTCF enDir:%d out of Range\n", enDir);\
            return CVI_DTCF_ERR_EINVAL_PAEAMETER;\
        }\
    }while(0)

#define CHECK_DTCF_FILETYPE_RANGE(fileType)\
    do{\
        if(fileType >= CVI_DTCF_FILE_TYPE_BUTT)\
        {\
            CVI_LOGE("DTCF fileType out of Range\n" );\
            return CVI_DTCF_ERR_EINVAL_PAEAMETER;\
        }\
    }while(0)

#define CHECK_DTCF_FILE_PATH_LENGTH(length)\
    do{\
        if(length == 0)\
        {\
            CVI_LOGE("DTCF file path length is 0\n" );\
            return CVI_DTCF_ERR_EINVAL_PAEAMETER;\
        }\
    }while(0)

#define DTCF_THREAD_MUTEX_LOCK(pLock) \
    do {\
        int32_t s32LockRet = pthread_mutex_lock(&pLock);\
        if (0 != s32LockRet)         \
        {                                               \
            CVI_LOGE("[%s][%d]DTCF mutex lock failed %d!\n", __func__, __LINE__, s32LockRet);  \
        }                                               \
    } while (0)

#define DTCF_THREAD_MUTEX_UNLOCK(pLock) \
    do {\
        int32_t s32unLockRet = pthread_mutex_unlock(&pLock);\
        if (0 != s32unLockRet)         \
        {                                               \
            CVI_LOGE("[%s][%d]DTCF mutex unlock failed ret %d!\n", __func__, __LINE__, s32unLockRet);  \
        }                                               \
    } while (0)

#define FREE_SCANDIR_LIST(list, cnt) \
    do {\
        int32_t i = 0;\
        while (i < cnt)\
        {  \
            free(list[i++]);\
        }   \
        free(list);\
    } while(0)

#define CHECK_DTCF_PATH_IS_DIR(path) \
    do {\
        struct stat s_buf = {0};\
        if (0 != stat(path, &s_buf)) { \
            CVI_LOGE("[%s][%d] stat sys errno:%d  %s\n", __func__, __LINE__, errno, strerror(errno));\
            return CVI_DTCF_ERR_SYSTEM_ERROR;\
        }\
        if (!S_ISDIR(s_buf.st_mode)){\
            CVI_LOGE("[%s][%d] %s is not a directory\n", __func__, __LINE__, path);\
            return CVI_DTCF_PATH_IS_NOT_DIR_ERROR;\
        }\
    } while(0)

#define IS_DIGIT_CHAR(c, min, max)\
    do{\
        if(max < c || c < min)\
        {\
            return false;\
        }\
    }while(0)

#define IS_SAME_CHAR(c, tag)\
    do{\
        if(tag != c)\
        {\
            return false;\
        }\
    }while(0)

#define MW_UNUSED(x) (void)(x)

typedef enum cviDTCF_STATUS_E
{
    DTCF_STATUS_CREATED = 0,
    DTCF_STATUS_SCANED,
    DTCF_STATUS_DESTROY,
    DTCF_STATUS_BUTT
} CVI_DTCF_STATUS_E;

typedef struct cviDTCF_TEMP_DIR_S
{
    char azTmpDir[CVI_FILE_PATH_LEN_MAX];
    uint32_t u32DirNameLen;
    CVI_DTCF_DIR_E enDir;
    CVI_DTCF_FILE_TYPE_E enType;
    char azTimeStr[CVI_DTCF_TIME_STR_LEN + 1];
} CVI_DTCF_TEMP_DIR_S;

typedef struct cviDTCF_DIRENT_S
{
    struct dirent *pdirent;
    CVI_DTCF_DIR_E enDir;
} CVI_DTCF_DIRENT_S;

typedef struct cviDTCF_SCAN_FILE_LIST_S
{
    CVI_DTCF_DIRENT_S *pstDirentList;
    uint32_t u32DirentlistCount;
    uint32_t u32listCount;
    CVI_DTCF_DIR_E enScanDirs[DTCF_DIR_BUTT];
    uint32_t u32ScanDirCount;
} CVI_DTCF_SCAN_FILE_LIST_S;

static CVI_DTCF_SCAN_FILE_LIST_S s_stDtcfScanFileList = {NULL, 0, 0, {0}, 0};

static char s_azRootDir[CVI_FILE_PATH_LEN_MAX] = {0};
static CVI_DTCF_STATUS_E s_enDtcfStatu = DTCF_STATUS_BUTT;
static pthread_mutex_t s_pthDtcfInterface_lock = PTHREAD_MUTEX_INITIALIZER;

static char g_azDirNames[DTCF_DIR_BUTT][CVI_DIR_LEN_MAX] =
{
    CVI_EMR_DIR,
    CVI_EMR_S_DIR,
    CVI_MOVIES_DIR,
    CVI_MOVIES_S_DIR,
    CVI_PARK_DIR,
    CVI_PARK_S_DIR,
    CVI_EMR_B_DIR,
    CVI_EMR_B_S_DIR,
    CVI_MOVIES_B_DIR,
    CVI_MOVIES_B_S_DIR,
    CVI_PARK_B_DIR,
    CVI_PARK_B_S_DIR,
    CVI_PHOTO_DIR,
    CVI_PHOTO_B_DIR
};

static char g_szFileTypeSuffix[CVI_DTCF_FILE_TYPE_BUTT][CVI_FILE_SUFFIX_LEN_MAX] =
{
    "MP4",
    "JPG",
    "TS",
    "MOV"
};

static char g_szFileNameSuffix[DTCF_DIR_BUTT][CVI_FILE_SUFFIX_LEN_MAX] =
{
    "",
    "_s",
    "",
    "_s",
    "",
    "_s",
    "_b",
    "_b_s",
    "_b",
    "_b_s",
    "_b",
    "_b_s",
    "",
    "_b"
};

static void dtcf_FreeScanNameList()
{
    while (s_stDtcfScanFileList.u32DirentlistCount--)
    {
        if(NULL != s_stDtcfScanFileList.pstDirentList[s_stDtcfScanFileList.u32DirentlistCount].pdirent)
        {
            free(s_stDtcfScanFileList.pstDirentList[s_stDtcfScanFileList.u32DirentlistCount].pdirent);
            s_stDtcfScanFileList.pstDirentList[s_stDtcfScanFileList.u32DirentlistCount].pdirent = NULL;
        }
    }
    free(s_stDtcfScanFileList.pstDirentList);
    s_stDtcfScanFileList.pstDirentList = NULL;
    s_stDtcfScanFileList.u32DirentlistCount = 0;
    s_stDtcfScanFileList.u32ScanDirCount = 0;
    s_stDtcfScanFileList.u32listCount = 0;
    memset(&s_stDtcfScanFileList.enScanDirs, 0x0, sizeof(CVI_DTCF_DIR_E) * DTCF_DIR_BUTT);
}

static bool DTCF_FileNameRuleCheckFileTypeSuffix(const char *pFileTypeSuffix)
{
    uint32_t suffexLen,i;
    for (i = 0; i < CVI_DTCF_FILE_TYPE_BUTT; i++)
    {
        suffexLen = strlen(g_szFileTypeSuffix[i]);
        if (0 == strncmp(g_szFileTypeSuffix[i], pFileTypeSuffix, suffexLen))
        {
            return true;
        }
    }
    return false;
}

static int32_t DTCF_GetFileSuffix(const char *pszFileName, char *pszSuffix)
{
    const char *nameSuffix = strrchr(pszFileName, '.');
    if (NULL == nameSuffix) {
     //   CVI_LOGE("input filename don't have . suffix \n");
        return false;
    }

    if (strnlen(nameSuffix, CVI_FILE_NAME_LEN_MIN + 1) > CVI_FILE_NAME_LEN_MIN ||
        strnlen(nameSuffix, CVI_FILE_NAME_LEN_MIN) < 2) {
    //    CVI_LOGD("input filename .suffix:%s  length too long, max len:%d \n", nameSuffix, CVI_FILE_NAME_LEN_MIN);
        return false;
    }

    snprintf(pszSuffix, CVI_FILE_NAME_LEN_MIN, "%s", nameSuffix + 1);
    return true;
}

/**
 * .MP4
 * .JPEG
 * .TS
 */
static bool DTCF_FileNameRuleCheck(const char *pazPath)
{
	char Suffix[CVI_FILE_NAME_LEN_MIN] = {0};
	DTCF_GetFileSuffix(pazPath, Suffix);

	return DTCF_FileNameRuleCheckFileTypeSuffix(Suffix);
}

static int32_t DTCF_CheckRootDir(const char* pszRootDir)
{
    int32_t s32Ret = 0;
    uint32_t i = 0;
    uint32_t u32Len = 0;
    char aszPath[CVI_FILE_PATH_LEN_MAX] = {0};

    u32Len = strlen(pszRootDir);
    if((u32Len >= CVI_FILE_PATH_LEN_MAX) || (u32Len == 0)) {
        CVI_LOGE("RootDir:%s too long\n", pszRootDir);
        return CVI_DTCF_UNSUPPORT_PATH;
    }

    s32Ret = snprintf(aszPath, CVI_FILE_PATH_LEN_MAX - 1, "%s", pszRootDir);
    if (s32Ret < 0) {
        CVI_LOGE("RootDir:%s is invalid path\n", pszRootDir);
        return CVI_DTCF_UNSUPPORT_PATH;
    }

    for (i = 0; i < u32Len; i++) {
        if (i == 0 && aszPath[i] != '/') {
            CVI_LOGE("RootDir:%s is invalid path\n", pszRootDir);
            return CVI_DTCF_UNSUPPORT_PATH;
        }

        if (aszPath[i] == '/') {
            continue;
        }

        if (!((aszPath[i] >= 'a' && aszPath[i] <= 'z')||
            (aszPath[i] >= 'A' && aszPath[i] <= 'Z')||
            (aszPath[i] >= '0' && aszPath[i] <= '9')||
            (aszPath[i] == '_') || (aszPath[i] == '.'))) {
            CVI_LOGE("RootDir:%s is invalid path\n", pszRootDir);
            return CVI_DTCF_UNSUPPORT_PATH;
        }
    }

    return 0;
}

static int32_t selector(const struct dirent *dir)
{
    CHECK_DTCF_NULL_PTR(dir);
    //uint32_t len = strlen(dir->d_name);
    if(true == DTCF_FileNameRuleCheck(dir->d_name))
        return 1;

    //CVI_LOGE( "dir->d_name:%s\n", dir->d_name);
    return 0;
}

static int32_t sortBydname( const void *a , const void *b)
{
    CVI_DTCF_DIRENT_S *aa = (CVI_DTCF_DIRENT_S *)a;
    CVI_DTCF_DIRENT_S *bb = (CVI_DTCF_DIRENT_S *)b;
    return alphasort((void *)&bb->pdirent, (void *)&aa->pdirent);
}

static int32_t DTCF_checkFilePath(const char *pazSrcFilePath, CVI_DTCF_TEMP_DIR_S *pstSrcTmpDir)
{
    int32_t index = 0;
    uint32_t u32FilePathLen = 0;
    char aszFileName[CVI_FILE_PATH_LEN_MAX] = {0};
    CVI_DTCF_DIR_E i;
    CVI_DTCF_FILE_TYPE_E j;

    u32FilePathLen = strlen(pazSrcFilePath);
    if (u32FilePathLen >= CVI_FILE_PATH_LEN_MAX)
    {
        CVI_LOGE("pazSrcFilePath:%s too long\n", pazSrcFilePath);
        return CVI_DTCF_UNSUPPORT_PATH;
    }

    if(strncmp(s_azRootDir, pazSrcFilePath, strlen(s_azRootDir)) != 0)
    {
        CVI_LOGE("s_azRootDir:%s pazSrcFilePath:%s\n", s_azRootDir, pazSrcFilePath);
        return CVI_DTCF_UNSUPPORT_PATH;
    }
    index += strlen(s_azRootDir);
    pstSrcTmpDir->u32DirNameLen = u32FilePathLen;
    strncpy(pstSrcTmpDir->azTmpDir, pazSrcFilePath, pstSrcTmpDir->u32DirNameLen);

    index += 1;
    while(index < CVI_FILE_PATH_LEN_MAX && '/' == pazSrcFilePath[index])
    {
        index++;
    }
    char *cNext = strstr(pazSrcFilePath + index, "/");
    if(NULL == cNext)
    {
        CVI_LOGE("pazSrcFilePath:%s index:%d, not find next '/'\n", pazSrcFilePath, index);
        return CVI_DTCF_UNSUPPORT_PATH;
    }
    for(i = DTCF_DIR_EMR_FRONT; i < DTCF_DIR_BUTT; i ++)
    {
        if(0 == strncmp(g_azDirNames[i], pazSrcFilePath + index, (cNext - pazSrcFilePath - index)))
        {
            pstSrcTmpDir->enDir = (CVI_DTCF_DIR_E)i;
            index += strlen(g_azDirNames[i]);
            break;
        }
    }
    if(DTCF_DIR_BUTT == i)
    {
        CVI_LOGE("pazSrcFilePath:%s index:%d, not support\n", pazSrcFilePath, index);
        return CVI_DTCF_UNSUPPORT_PATH;
    }
    index += 1;
    while(index < CVI_FILE_PATH_LEN_MAX && '/' == pazSrcFilePath[index])
    {
        index++;
    }

    strncpy(aszFileName, pazSrcFilePath + index, CVI_FILE_PATH_LEN_MAX - 1);

    if (!DTCF_FileNameRuleCheck(aszFileName)) {
        return CVI_DTCF_UNSUPPORT_PATH;
    }

    strncpy(pstSrcTmpDir->azTimeStr, pazSrcFilePath + index, CVI_DTCF_TIME_STR_LEN);

    char *s = strstr(pazSrcFilePath + index, ".");
    if(NULL == s)
    {
        CVI_LOGE("pazSrcFilePath:%s index:%d, not find '.'\n", pazSrcFilePath, index);
        return CVI_DTCF_UNSUPPORT_PATH;
    }

    for(j = CVI_DTCF_FILE_TYPE_MP4; j < CVI_DTCF_FILE_TYPE_BUTT; j ++)
    {
        if(0 == strncmp(g_szFileTypeSuffix[j], s + 1, strlen(g_szFileTypeSuffix[j])))
        {
            pstSrcTmpDir->enType = (CVI_DTCF_FILE_TYPE_E)j;
            break;
        }
    }

    if(CVI_DTCF_FILE_TYPE_BUTT == j)
    {
        CVI_LOGE("pazSrcFilePath:%s :%s, not support\n", pazSrcFilePath, s);
        return CVI_DTCF_UNSUPPORT_PATH;
    }
    return 0;
}

static int32_t DTCF_mkdir(const char *dir, mode_t mode)
{
    MW_UNUSED(mode);

    int32_t s32Ret = mkdir(dir, CVI_DTCF_DIR_MODE);
    if(s32Ret < 0)
    {
        if(errno == EEXIST)
        {
            CHECK_DTCF_PATH_IS_DIR(dir);
        }
        else
        {
            CVI_LOGE("mkdir %s fail s32Ret:%d\n",dir, s32Ret);
            CVI_LOGE("[%d]:%s \n", errno, strerror(errno));
            return CVI_DTCF_ERR_EINVAL_PAEAMETER;
        }
    }
    return 0;
}

static int32_t DTCF_mkdirs(const char *muldir, mode_t mode)
{
    int32_t i=0;
    int32_t len=0;
    int32_t s32Ret = 0;
    char str[CVI_FILE_PATH_LEN_MAX + 1] = { 0 };

    strncpy(str, muldir, CVI_FILE_PATH_LEN_MAX);

    len = strlen(str);
    for (i = 0; i < len; i++)
    {
        if (0 == i && str[i] == '/')
        {
            i++;
        }
        if (str[i] == '/')
        {
            str[i] = '\0';
            s32Ret = mkdir(str, mode);
            if (s32Ret < 0)
            {
                if (errno == EEXIST)
                {
                    CHECK_DTCF_PATH_IS_DIR(str);
                }
                else
                {
                    CVI_LOGE("mkdir str:%s fail s32Ret:%d\n", str, s32Ret);
                    perror(str);
                    return s32Ret;
                }
            }

            str[i] = '/';
        }
    }

    if (len > 0)
    {
        s32Ret = mkdir(str, mode);
        if (s32Ret < 0)
        {
            if (errno == EEXIST)
            {
                CHECK_DTCF_PATH_IS_DIR(str);
            }
            else
            {
                CVI_LOGE("mkdir str:%s fail s32Ret:%d\n", str, s32Ret);
                perror(str);
                return s32Ret;
            }
        }
    }
    return 0;
}

static int32_t DTCF_ExpandFileList(uint32_t u32size)
{
    uint32_t u32ChangePoit = 10;
    if(s_stDtcfScanFileList.u32DirentlistCount + u32size > CVI_DTCF_SCAN_FILE_AMOUNT_MAX)
    {
        CVI_LOGE("u32DirentlistCount:%d +  u32size:%u > CVI_DTCF_SCAN_FILE_AMOUNT_MAX:%d\n", s_stDtcfScanFileList.u32DirentlistCount, u32size, CVI_DTCF_SCAN_FILE_AMOUNT_MAX);
        return CVI_DTCF_ERR_EINVAL_PAEAMETER;
    }

    if(s_stDtcfScanFileList.u32DirentlistCount + u32size >= s_stDtcfScanFileList.u32listCount)
    {
        while(u32ChangePoit < CVI_DTCF_SCAN_FILE_AMOUNT_MAX)
        {
            if(u32ChangePoit < s_stDtcfScanFileList.u32DirentlistCount + u32size)
            {
                u32ChangePoit = u32ChangePoit * 2;
            }
            else
            {
                break;
            }
        }
        CVI_DTCF_DIRENT_S *newDirEnt = NULL;
        newDirEnt = (CVI_DTCF_DIRENT_S *) malloc ((u32ChangePoit) * sizeof (CVI_DTCF_DIRENT_S));
        if(NULL == newDirEnt)
        {
            CVI_LOGE("malloc mem error, errno:%d\n", errno);
            return CVI_DTCF_ERR_SYSTEM_ERROR;
        }
        if(NULL == memset(newDirEnt, 0x0, (u32ChangePoit) * sizeof (CVI_DTCF_DIRENT_S)))
        {
            free(newDirEnt);
            newDirEnt = NULL;
            return CVI_DTCF_ERR_EINVAL_PAEAMETER;
        }
        if(NULL != s_stDtcfScanFileList.pstDirentList)
        {
            if( NULL == memcpy( newDirEnt, s_stDtcfScanFileList.pstDirentList, (s_stDtcfScanFileList.u32DirentlistCount*sizeof(CVI_DTCF_DIRENT_S)) ) )
            {
                free(newDirEnt);
                newDirEnt = NULL;
                return CVI_DTCF_ERR_EINVAL_PAEAMETER;
            }
            free(s_stDtcfScanFileList.pstDirentList);
        }

        s_stDtcfScanFileList.pstDirentList = newDirEnt;
        s_stDtcfScanFileList.u32listCount = u32ChangePoit;
    }


    return 0;
}

static int32_t DTCF_Addfile(const char *fileName, CVI_DTCF_DIR_E enDir)
{
    uint32_t i = 0;
    uint32_t low, high, mid;
    int32_t ret = DTCF_ExpandFileList(1);
    if (ret != 0) {
        CVI_LOGE("DTCF_ExpandFileList 1 fail ret:%d\n", ret);
        return ret;
    }

    struct dirent *p = (struct dirent *) malloc(sizeof(struct dirent));
    if (p == NULL) {
        CVI_LOGE("malloc dirent fail\n");
        return CVI_DTCF_ERR_NOMEM;
    }

    memset(p, 0x0, sizeof(struct dirent));

    p->d_reclen = strlen(fileName);
    ret = snprintf(p->d_name, NAME_MAX, "%s", fileName);
    if (ret >= NAME_MAX || ret <= 0) {
        free(p);
        p = NULL;
        CVI_LOGE("snprintf pazFilePath fail, ret:%d\n", ret);
        return CVI_DTCF_ERR_EINVAL_PAEAMETER;
    }

    CVI_DTCF_DIRENT_S stDirentTmp = { NULL, DTCF_DIR_BUTT };
    stDirentTmp.pdirent = p;
    stDirentTmp.enDir = enDir;

    if (s_stDtcfScanFileList.u32DirentlistCount == 0) {
        s_stDtcfScanFileList.pstDirentList[0].pdirent = stDirentTmp.pdirent;
        s_stDtcfScanFileList.pstDirentList[0].enDir = stDirentTmp.enDir;
        s_stDtcfScanFileList.u32DirentlistCount = s_stDtcfScanFileList.u32DirentlistCount + 1;
        return 0;
    }

    low = 0;
    high = s_stDtcfScanFileList.u32DirentlistCount - 1;
    while (low <= high && high < s_stDtcfScanFileList.u32DirentlistCount) {
        mid = (low + high) / 2;
        ret = sortBydname((void *) &s_stDtcfScanFileList.pstDirentList[mid], (void *) &stDirentTmp);
        if (ret > 0) {
            high = mid - 1;
        } else if (ret == 0 && s_stDtcfScanFileList.pstDirentList[mid].enDir == stDirentTmp.enDir) {
            free(p);
            p = NULL;
            CVI_LOGE("%s is exit at %d \n", fileName, mid);
            return CVI_DTCF_SAME_FILENAME_PATH;
        } else {
            low = mid + 1;
        }
    }

    for (i = s_stDtcfScanFileList.u32DirentlistCount; i > high + 1; i--) {
        s_stDtcfScanFileList.pstDirentList[i].pdirent =
                s_stDtcfScanFileList.pstDirentList[i - 1].pdirent;
        s_stDtcfScanFileList.pstDirentList[i].enDir =
                s_stDtcfScanFileList.pstDirentList[i - 1].enDir;
    }
    s_stDtcfScanFileList.pstDirentList[high + 1].pdirent = stDirentTmp.pdirent;
    s_stDtcfScanFileList.pstDirentList[high + 1].enDir = stDirentTmp.enDir;
    s_stDtcfScanFileList.u32DirentlistCount = s_stDtcfScanFileList.u32DirentlistCount + 1;

    return 0;
}


static int32_t DTCF_ResetDirNames()
{
    char azDirNames[DTCF_DIR_BUTT][CVI_DIR_LEN_MAX] =
            {
                CVI_EMR_DIR,
                CVI_EMR_S_DIR,
                CVI_MOVIES_DIR,
                CVI_MOVIES_S_DIR,
                CVI_PARK_DIR,
                CVI_PARK_S_DIR,
                CVI_EMR_B_DIR,
                CVI_EMR_B_S_DIR,
                CVI_MOVIES_B_DIR,
                CVI_MOVIES_B_S_DIR,
                CVI_PARK_B_DIR,
                CVI_PARK_B_S_DIR,
                CVI_PHOTO_DIR,
                CVI_PHOTO_B_DIR
            };
    if(memcpy(g_azDirNames, azDirNames, sizeof(char) * CVI_DIR_LEN_MAX * DTCF_DIR_BUTT) == NULL) {
        CVI_LOGE("reset dtcf dir name failed\n");
        return CVI_DTCF_ERR_EINVAL_PAEAMETER;
    }

    s_azRootDir[0] = '\0';

    return 0;
}

int32_t CVI_DTCF_Init(const char *pazRootDir, const char azDirNames[DTCF_DIR_BUTT][CVI_DIR_LEN_MAX])
{
    uint32_t i;
    int32_t ret = 0;
    CHECK_DTCF_NULL_PTR(pazRootDir);
    DTCF_THREAD_MUTEX_LOCK(s_pthDtcfInterface_lock);

    ret = DTCF_CheckRootDir(pazRootDir);
    if (ret != 0)
    {
        CVI_LOGE("DTCF init failed:0x%x\n", ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return ret;
    }

    if((DTCF_STATUS_BUTT != s_enDtcfStatu && DTCF_STATUS_DESTROY != s_enDtcfStatu)
            || DTCF_STATUS_CREATED == s_enDtcfStatu)
    {
        CVI_LOGE("status error, current status:%d\n", s_enDtcfStatu);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return CVI_DTCF_ERR_STATUS_ERROR;
    }

    ret = DTCF_mkdirs(pazRootDir, CVI_DTCF_DIR_MODE);
    if(ret != 0)
    {
        CVI_LOGE("DTCF_mkdirs azRootDir:%s fail s32Ret:%x\n", pazRootDir, ret);
        perror(pazRootDir);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return ret;
    }

    ret = snprintf(s_azRootDir, CVI_FILE_PATH_LEN_MAX-1, "%s", pazRootDir);
    if(ret <= 0)
    {
        CVI_LOGE("snprintf pazFilePath fail, s32Ret:%d\n", ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return CVI_DTCF_ERR_EINVAL_PAEAMETER;
    }

    if(NULL != azDirNames)
    {
        char (*pazDirNamesTmp)[CVI_DIR_LEN_MAX] = (char (*)[CVI_DIR_LEN_MAX])azDirNames;
        memset(g_azDirNames, 0x0, sizeof(char)*CVI_DIR_LEN_MAX*DTCF_DIR_BUTT);

        for(i = 0; i < DTCF_DIR_BUTT; i++)
        {
            ret = snprintf(g_azDirNames[i], CVI_DIR_LEN_MAX-1, "%s", pazDirNamesTmp[i]);
            if(ret < 0)
            {
                CVI_LOGE("snprintf g_azDirNames[%d] fail, s32Ret:%d\n", i, ret);
                (void)DTCF_ResetDirNames();
                DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
                return CVI_DTCF_ERR_EINVAL_PAEAMETER;
            }
            else if(ret >= CVI_DIR_LEN_MAX)
            {
                CVI_LOGE("snprintf pazDirNamesTmp[i]:%s is too long, s32Ret:%d\n", pazDirNamesTmp[i], ret);
                (void)DTCF_ResetDirNames();
                DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
                return CVI_DTCF_ERR_EINVAL_PAEAMETER;
            }
        }
    }

    for(i = 0; i < DTCF_DIR_BUTT; i++)
    {
        if(strlen(g_azDirNames[i]) == 0) {
            continue;
        }

        char azTmpDir[CVI_FILE_PATH_LEN_MAX] = {0};
        ret = snprintf(azTmpDir, CVI_FILE_PATH_LEN_MAX-1, "%s/%s", s_azRootDir, g_azDirNames[i]);
        if(ret <= 0)
        {
            CVI_LOGE("snprintf stTmpDir.azTmpDir fail, s32Ret:%d\n", ret);
            (void)DTCF_ResetDirNames();
            DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
            return CVI_DTCF_ERR_EINVAL_PAEAMETER;
        }

        ret = DTCF_mkdir(azTmpDir, CVI_DTCF_DIR_MODE);
        if(ret != 0)
        {
            CVI_LOGE("mkdir g_azDirNames[%d]:%s fail s32Ret:%d\n", i, g_azDirNames[i], ret);
            (void)DTCF_ResetDirNames();
            DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
            return ret;
        }
    }

    s_enDtcfStatu = DTCF_STATUS_CREATED;
    DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
    return 0;
}

int32_t CVI_DTCF_DeInit(void)
{
    if ((DTCF_STATUS_BUTT == s_enDtcfStatu || DTCF_STATUS_DESTROY == s_enDtcfStatu)
            && (DTCF_STATUS_CREATED != s_enDtcfStatu)) {
        CVI_LOGE("status error, current status:%d\n", s_enDtcfStatu);
        return CVI_DTCF_ERR_STATUS_ERROR;
    }

    DTCF_THREAD_MUTEX_LOCK(s_pthDtcfInterface_lock);
    if (s_stDtcfScanFileList.pstDirentList != NULL) {
        dtcf_FreeScanNameList();
    }
    (void)DTCF_ResetDirNames();
    s_enDtcfStatu = DTCF_STATUS_DESTROY;
    DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);


    return 0;
}

int32_t CVI_DTCF_GetDirNames(char (*pazDirNames)[CVI_DIR_LEN_MAX], uint32_t u32DirAmount)
{
    uint32_t i;
    CHECK_DTCF_NULL_PTR(pazDirNames);
    CHECK_DTCF_STATUS_CREATED(s_enDtcfStatu);
    DTCF_THREAD_MUTEX_LOCK(s_pthDtcfInterface_lock);
    if(0 == u32DirAmount || u32DirAmount > DTCF_DIR_BUTT)
    {
        CVI_LOGE("u32DirAmount :%d is error\n", u32DirAmount);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return CVI_DTCF_ERR_EINVAL_PAEAMETER;
    }

    for(i = 0; i < u32DirAmount; i++)
    {
        strncpy(pazDirNames[i], g_azDirNames[i], CVI_DIR_LEN_MAX-1);
    }
    DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
    return 0;
}

/*
 */
int32_t CVI_DTCF_Scan(CVI_DTCF_DIR_E enDirs[DTCF_DIR_BUTT], uint32_t u32DirCount, uint32_t *pu32FileAmount)
{
    uint32_t i;
    CHECK_DTCF_NULL_PTR(enDirs);
    CHECK_DTCF_NULL_PTR(pu32FileAmount);
    CHECK_DTCF_STATUS_CREATED(s_enDtcfStatu);
    DTCF_THREAD_MUTEX_LOCK(s_pthDtcfInterface_lock);

    if (u32DirCount == 0 || u32DirCount > DTCF_DIR_BUTT) {
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return CVI_DTCF_ERR_EINVAL_PAEAMETER;
    }

    if (NULL != s_stDtcfScanFileList.pstDirentList) {
        dtcf_FreeScanNameList();
    }

    for (i = 0; i < u32DirCount; i++) {
        int32_t s32ScanNamelistTmpCount = 0;
        struct dirent **dirScanNameTmpList = NULL;
        if (enDirs[i] >= DTCF_DIR_BUTT) {
            CVI_LOGE("DTCF enDir:%d out of Range\n", enDirs[i]);
            DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
            return CVI_DTCF_ERR_EINVAL_PAEAMETER;
        }

        if (strlen(g_azDirNames[(int32_t)enDirs[i]]) == 0) {
            DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
            return CVI_DTCF_ERR_EINVAL_PAEAMETER;
        }

        char dirBuf[CVI_FILE_PATH_LEN_MAX] = { 0 };
        int32_t ret = snprintf(dirBuf, CVI_FILE_PATH_LEN_MAX - 1, "%s/%s/", s_azRootDir, g_azDirNames[(int32_t)enDirs[i]]);
        if (ret <= 0) {
            CVI_LOGE("snprintf stTmpDir.dirBuf fail, ret:%d\n", ret);
            DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
            return CVI_DTCF_ERR_EINVAL_PAEAMETER;
        }
        // in order to clean codeDEX warning
        char *dirp = dirBuf;
        s32ScanNamelistTmpCount = scandir(dirp, &dirScanNameTmpList, selector, NULL);
        if (s32ScanNamelistTmpCount < 0) {
            CVI_LOGE("scandir %s return:%d\n", dirBuf, s32ScanNamelistTmpCount);
            if(NULL != dirScanNameTmpList)
            {
                CVI_LOGE("scandir free dirScanNameTmpList\n");
                free(dirScanNameTmpList);
            }
            continue;
        }

        if (dirScanNameTmpList != NULL) {
            ret = DTCF_ExpandFileList(s32ScanNamelistTmpCount);
            if (ret != 0) {
                CVI_LOGE("DTCF_ExpandFileList fail, ret:%x\n", ret);
                FREE_SCANDIR_LIST(dirScanNameTmpList, s32ScanNamelistTmpCount);
                DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
                return ret;
            }

            int32_t n;
            for (n = 0; n < s32ScanNamelistTmpCount; n++) {
                s_stDtcfScanFileList.pstDirentList[s_stDtcfScanFileList.u32DirentlistCount + n].pdirent =  dirScanNameTmpList[n];
                s_stDtcfScanFileList.pstDirentList[s_stDtcfScanFileList.u32DirentlistCount + n].enDir = enDirs[i];
            }
            s_stDtcfScanFileList.u32DirentlistCount = s_stDtcfScanFileList.u32DirentlistCount + s32ScanNamelistTmpCount;
            free(dirScanNameTmpList);
        }
        s_stDtcfScanFileList.enScanDirs[i] = enDirs[i];
    }
    s_stDtcfScanFileList.u32ScanDirCount = u32DirCount;
    *pu32FileAmount = s_stDtcfScanFileList.u32DirentlistCount;
    if (s_stDtcfScanFileList.u32DirentlistCount > 0 && s_stDtcfScanFileList.pstDirentList != NULL) {
        qsort (s_stDtcfScanFileList.pstDirentList, s_stDtcfScanFileList.u32DirentlistCount, sizeof (CVI_DTCF_DIRENT_S), sortBydname);
    }
    s_enDtcfStatu = DTCF_STATUS_SCANED;
    DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
    return 0;
}


int32_t CVI_DTCF_GetFileByIndex(uint32_t u32Index, char *pazFileName, uint32_t u32Length, CVI_DTCF_DIR_E *penDir)
{
    CHECK_DTCF_NULL_PTR(pazFileName);
    CHECK_DTCF_NULL_PTR(penDir);
    CHECK_DTCF_STATUS_SCANED(s_enDtcfStatu);
    CHECK_DTCF_FILE_PATH_LENGTH(u32Length);
    DTCF_THREAD_MUTEX_LOCK(s_pthDtcfInterface_lock);

    if(u32Index >= s_stDtcfScanFileList.u32DirentlistCount)
    {
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return CVI_DTCF_ERR_EINVAL_PAEAMETER;
    }

    int32_t s32Ret = snprintf(pazFileName, u32Length-1, "%s/%s/%s", s_azRootDir, g_azDirNames[(int32_t)s_stDtcfScanFileList.pstDirentList[u32Index].enDir], s_stDtcfScanFileList.pstDirentList[u32Index].pdirent->d_name);
    if(s32Ret <= 0)
    {
        CVI_LOGE("snprintf pazFileName fail, s32Ret:%d\n", s32Ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return CVI_DTCF_ERR_EINVAL_PAEAMETER;
    }
    else if((uint32_t)s32Ret >= u32Length)
    {
        CVI_LOGE("snprintf u32Length:%d is not length enough, s32Ret:%d\n", u32Length, s32Ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return CVI_DTCF_ERR_EINVAL_PAEAMETER;
    }

    *penDir = s_stDtcfScanFileList.pstDirentList[u32Index].enDir;
    DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
    return 0;
}

int32_t CVI_DTCF_DelFileByIndex(uint32_t u32Index, uint32_t *pu32FileAmount)
{
    CHECK_DTCF_NULL_PTR(pu32FileAmount);
    CHECK_DTCF_STATUS_SCANED(s_enDtcfStatu);
    DTCF_THREAD_MUTEX_LOCK(s_pthDtcfInterface_lock);

    if(u32Index >= s_stDtcfScanFileList.u32DirentlistCount)
    {
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return CVI_DTCF_ERR_EINVAL_PAEAMETER;
    }

    if(NULL != s_stDtcfScanFileList.pstDirentList[u32Index].pdirent)
    {
        free(s_stDtcfScanFileList.pstDirentList[u32Index].pdirent);
        s_stDtcfScanFileList.pstDirentList[u32Index].pdirent = NULL;
    }

    memmove(s_stDtcfScanFileList.pstDirentList + u32Index, s_stDtcfScanFileList.pstDirentList + u32Index + 1,
    	   (sizeof(CVI_DTCF_DIRENT_S) * (s_stDtcfScanFileList.u32DirentlistCount - u32Index - 1)));

    s_stDtcfScanFileList.u32DirentlistCount--;
    *pu32FileAmount = s_stDtcfScanFileList.u32DirentlistCount;
    DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
    return 0;
}

int32_t CVI_DTCF_AddFile(const char *pazSrcFilePath, CVI_DTCF_DIR_E enDir)
{
    CHECK_DTCF_NULL_PTR(pazSrcFilePath);
    CHECK_DTCF_STATUS_SCANED(s_enDtcfStatu);
    uint32_t i = 0;
    CVI_DTCF_TEMP_DIR_S stSrcTmpDir = {{0},0,0,0,{0}};
    char fileNameBuf[CVI_FILE_PATH_LEN_MAX] = {0};
    DTCF_THREAD_MUTEX_LOCK(s_pthDtcfInterface_lock);
    int32_t ret = DTCF_checkFilePath(pazSrcFilePath, &stSrcTmpDir);
    if (ret != 0) {
        CVI_LOGE("DTCF_checkFilePath error ret:%x\n", ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return ret;
    }
    if (enDir != stSrcTmpDir.enDir) {
        CVI_LOGE("pazSrcFilePath:%s enDir:%d is not match\n", pazSrcFilePath, enDir);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return CVI_DTCF_ERR_EINVAL_PAEAMETER;
    }

    for (i = 0; i < s_stDtcfScanFileList.u32ScanDirCount; i++) {
        if (enDir == s_stDtcfScanFileList.enScanDirs[i]) {
            break;
        }
    }

    if (i == s_stDtcfScanFileList.u32ScanDirCount) {
        CVI_LOGE("enDir:%d is not scan\n", enDir);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return CVI_DTCF_ERR_EINVAL_PAEAMETER;
    }

    ret = snprintf(fileNameBuf, CVI_FILE_PATH_LEN_MAX - 1, "%s%s.%s", stSrcTmpDir.azTimeStr, g_szFileNameSuffix[enDir], g_szFileTypeSuffix[stSrcTmpDir.enType]);
    if (ret <= 0) {
        CVI_LOGE("snprintf pazFilePath fail, ret:%x\n", ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return CVI_DTCF_ERR_EINVAL_PAEAMETER;
    } else if (ret >= CVI_FILE_PATH_LEN_MAX) {
        CVI_LOGE("snprintf u32Length:%d is not length enough, ret:%d\n",
            CVI_FILE_PATH_LEN_MAX, ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return CVI_DTCF_ERR_EINVAL_PAEAMETER;
    }

    char *fileName = fileNameBuf;
    ret = DTCF_Addfile(fileName, enDir);
    if (ret != 0) {
        CVI_LOGE("DTCF_Addfile fail, ret:%x\n", ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return ret;
    }
    DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
    return 0;
}

int32_t CVI_DTCF_GetOldestFileIndex(CVI_DTCF_DIR_E enDirs[DTCF_DIR_BUTT], uint32_t u32DirCount, uint32_t *pu32Index)
{
    CHECK_DTCF_NULL_PTR(enDirs);
    CHECK_DTCF_NULL_PTR(pu32Index);
    CHECK_DTCF_STATUS_SCANED(s_enDtcfStatu);
    DTCF_THREAD_MUTEX_LOCK(s_pthDtcfInterface_lock);

    int32_t i = 0;
    uint32_t j = 0;
    int32_t index = -1;

    if(0 == u32DirCount || u32DirCount > DTCF_DIR_BUTT)
    {
        CVI_LOGE("u32DirCount is illegal, s32Ret:%x\n", CVI_DTCF_ERR_EINVAL_PAEAMETER);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return CVI_DTCF_ERR_EINVAL_PAEAMETER;
    }

    for (j = 0; j < u32DirCount; j++)
    {
        if (DTCF_DIR_BUTT <= enDirs[j])
        {
            CVI_LOGE("enDirs[%d] is illegal, s32Ret:%x\n", j,CVI_DTCF_ERR_EINVAL_PAEAMETER);
            DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
            return CVI_DTCF_ERR_EINVAL_PAEAMETER;
        }
    }

    CVI_LOGI("u32DirCount:%d u32DirentlistCount:%d\n", u32DirCount, s_stDtcfScanFileList.u32DirentlistCount);
    for(i = s_stDtcfScanFileList.u32DirentlistCount - 1; i >= 0; i--)
    {
        //CVI_LOGI("i:%d s_stDtcfScanFileList.pstDirentList[i].enDir:%d \n", i, s_stDtcfScanFileList.pstDirentList[i].enDir);
        for(j = 0; j < u32DirCount; j++)
        {
            if(enDirs[j] == s_stDtcfScanFileList.pstDirentList[i].enDir)
            {
                index = i;
                break;
            }
        }

        if(index >= 0)
        {
            break;
        }
    }

    if(index < 0 || (uint32_t)index >= s_stDtcfScanFileList.u32DirentlistCount)
    {
        CVI_LOGE("can't find file index:%d\n", index);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return CVI_DTCF_ERR_EINVAL_PAEAMETER;
    }
    *pu32Index = index;
    DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
    return 0;
}

int32_t CVI_DTCF_GetFileAmount(uint32_t *pu32FileAmount)
{
    CHECK_DTCF_NULL_PTR(pu32FileAmount);
    CHECK_DTCF_STATUS_SCANED(s_enDtcfStatu);
    DTCF_THREAD_MUTEX_LOCK(s_pthDtcfInterface_lock);
    *pu32FileAmount = s_stDtcfScanFileList.u32DirentlistCount;
    DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
    return 0;
}

int32_t CVI_DTCF_GetOldestFilePath(CVI_DTCF_DIR_E enDir, char *pazFilePath, uint32_t u32Length)
{
    CHECK_DTCF_NULL_PTR(pazFilePath);
    CHECK_DTCF_ENDIR_RANGE(enDir);
    CHECK_DTCF_STATUS_SCANED(s_enDtcfStatu);
    CHECK_DTCF_FILE_PATH_LENGTH(u32Length);

    DTCF_THREAD_MUTEX_LOCK(s_pthDtcfInterface_lock);
    if(strlen(g_azDirNames[enDir]) == 0)
    {
        CVI_LOGE("g_azDirNames[%d] undefined\n", enDir);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return CVI_DTCF_ERR_UNDEFINE_DIR;
    }

    CVI_DTCF_TEMP_DIR_S stTmpDir = {{0},0,0,0,{0}};
    int32_t s32Ret = snprintf(stTmpDir.azTmpDir, CVI_FILE_PATH_LEN_MAX-1, "%s/%s/", s_azRootDir, g_azDirNames[enDir]);
    if(s32Ret <= 0)
    {
        CVI_LOGE("snprintf stTmpDir.azTmpDir fail, s32Ret:%d\n", s32Ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return CVI_DTCF_ERR_EINVAL_PAEAMETER;
    }
    else if(s32Ret >= CVI_FILE_PATH_LEN_MAX)
    {
        CVI_LOGE("snprintf u32Length:%d is not length enough, s32Ret:%d\n", u32Length, s32Ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return CVI_DTCF_ERR_EINVAL_PAEAMETER;
    }
    int32_t i = 0;
    int32_t index = -1;
    CVI_LOGI("enDir:%d u32DirentlistCount:%d\n", enDir, s_stDtcfScanFileList.u32DirentlistCount);
    for(i = s_stDtcfScanFileList.u32DirentlistCount - 1; i >= 0; i--)
    {
        CVI_LOGI("i:%d s_stDtcfScanFileList.pstDirentList[i].enDir:%d \n", i, s_stDtcfScanFileList.pstDirentList[i].enDir);
        if(enDir == s_stDtcfScanFileList.pstDirentList[i].enDir)
        {
            index = i;
            break;
        }
    }
    if(index < 0 || (uint32_t)index >= s_stDtcfScanFileList.u32DirentlistCount)
    {
        CVI_LOGE("can't find enDir:%d file index:%d\n", enDir,index);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return CVI_DTCF_ERR_EINVAL_PAEAMETER;
    }

    s32Ret = snprintf(pazFilePath, u32Length-1, "%s%s", stTmpDir.azTmpDir, s_stDtcfScanFileList.pstDirentList[index].pdirent->d_name);
    if(s32Ret <= 0)
    {
        CVI_LOGE("snprintf pazFilePath fail, s32Ret:%d\n", s32Ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return CVI_DTCF_ERR_EINVAL_PAEAMETER;
    }
    else if((uint32_t)s32Ret >= u32Length)
    {
        CVI_LOGE("snprintf u32Length:%d is not length enough, s32Ret:%d\n", u32Length, s32Ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return CVI_DTCF_ERR_EINVAL_PAEAMETER;
    }

    DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
    return 0;
}

static int32_t CVI_DTCF_GetTime(char *pazTime, CVI_DTCF_DIR_E enDir)
{
    CHECK_DTCF_NULL_PTR(pazTime);
    struct tm *t = NULL;
    struct  timeval    tv;
    if(0 != gettimeofday(&tv,NULL))
    {
        CVI_LOGE("gettimeofday error, errno:%d\n", errno);
        perror("gettimeofday");
        return CVI_DTCF_ERR_SYSTEM_ERROR;
    }

    if(tv.tv_usec > 500000)
    {
        tv.tv_sec++;
    }

    t = localtime(&tv.tv_sec);
    if(NULL == t)
    {
        CVI_LOGE("localtime error, errno:%d\n", errno);
        perror("localtime");
        return CVI_DTCF_ERR_SYSTEM_ERROR;
    }

    CVI_LOGI("dir[%d] %4d_%02d%02d_%02d%02d%02d  %ld\n", enDir, t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec,tv.tv_usec);

    static struct tm s_lasttime[DTCF_DIR_BUTT];
    static int32_t s_s32Num[DTCF_DIR_BUTT]= {0};

    struct tm *last_time = &s_lasttime[enDir];
    if(t->tm_year==last_time->tm_year&&
       t->tm_mon==last_time->tm_mon&&
       t->tm_mday==last_time->tm_mday&&
       t->tm_hour==last_time->tm_hour&&
       t->tm_min==last_time->tm_min&&
       t->tm_sec==last_time->tm_sec)
    {
        s_s32Num[enDir]++;
    }
    else
    {
        s_s32Num[enDir] = 0;
    }

    // if tCVIs API was called 100 times or more IN THE SAME SECOND, will cause string length over CVI_DTCF_TIME_STR_LEN
    if(s_s32Num[enDir] >= 100)
    {
        CVI_LOGE("s_s32Num[%d]=%d, file name over lenght error!\n", enDir, s_s32Num[enDir]);
        return CVI_DTCF_ERR_EINVAL_PAEAMETER;
    }

    memcpy(last_time, t, sizeof(struct tm));

    snprintf(pazTime, CVI_DTCF_TIME_STR_LEN + 1, "%4d_%02d_%02d_%02d%02d%02d_%02d",
        t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
        t->tm_hour, t->tm_min, t->tm_sec, s_s32Num[enDir]);

    return 0;
}

int32_t CVI_DTCF_CreateFilePath(CVI_DTCF_FILE_TYPE_E enFileType, CVI_DTCF_DIR_E enDir, char *pazFilePath, uint32_t u32Length)
{
    CHECK_DTCF_NULL_PTR(pazFilePath);
    CHECK_DTCF_ENDIR_RANGE(enDir);
    CHECK_DTCF_FILETYPE_RANGE(enFileType);
    CHECK_DTCF_STATUS_CREATED(s_enDtcfStatu);
    CHECK_DTCF_FILE_PATH_LENGTH(u32Length);
    DTCF_THREAD_MUTEX_LOCK(s_pthDtcfInterface_lock);
    if(strlen(g_azDirNames[enDir]) == 0)
    {
        CVI_LOGE("g_azDirNames[%d] undefined\n", enDir);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return CVI_DTCF_ERR_UNDEFINE_DIR;
    }

    char azTime[CVI_DTCF_TIME_STR_LEN + 1] = {0};
    int32_t s32Ret = CVI_DTCF_GetTime(azTime, enDir);
    if(0 != s32Ret)
    {
        CVI_LOGE("CVI_DTCF_GetTime fail, s32Ret:0x%x\n", s32Ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return s32Ret;
    }
    s32Ret = snprintf(pazFilePath, u32Length-1, "%s/%s/%s%s.%s", s_azRootDir, g_azDirNames[enDir], azTime, g_szFileNameSuffix[enDir], g_szFileTypeSuffix[enFileType]);
    if(s32Ret <= 0)
    {
        CVI_LOGE("snprintf pazFilePath fail, s32Ret:%d\n", s32Ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return CVI_DTCF_ERR_EINVAL_PAEAMETER;
    }
    else if((uint32_t)s32Ret >= u32Length)
    {
        CVI_LOGE("snprintf u32Length:%d is not length enough, s32Ret:%d\n", u32Length, s32Ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return CVI_DTCF_ERR_EINVAL_PAEAMETER;
    }
    DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
    return 0;
}

int32_t CVI_DTCF_GetRelatedFilePath(const char *pazSrcFilePath, CVI_DTCF_DIR_E enDir, char *pazDstFilePath, uint32_t u32Length)
{
    int32_t s32Ret = 0;
    CHECK_DTCF_NULL_PTR(pazSrcFilePath);
    CHECK_DTCF_NULL_PTR(pazDstFilePath);
    CHECK_DTCF_ENDIR_RANGE(enDir);
    CHECK_DTCF_STATUS_CREATED(s_enDtcfStatu);
    CHECK_DTCF_FILE_PATH_LENGTH(u32Length);
    CVI_DTCF_TEMP_DIR_S stSrcTmpDir = {{0},0,0,0,{0}};

    DTCF_THREAD_MUTEX_LOCK(s_pthDtcfInterface_lock);
    s32Ret = DTCF_checkFilePath(pazSrcFilePath,&stSrcTmpDir);
    if(0 != s32Ret)
    {
        CVI_LOGE("DTCF_checkFilePath error s32Ret:%x\n",s32Ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return s32Ret;
    }

    if(enDir == stSrcTmpDir.enDir)
    {
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return CVI_DTCF_SAME_DIR_PATH;
    }

    if(CVI_DIR_LEN_MAX < strlen(g_azDirNames[enDir]) || strlen(g_azDirNames[enDir]) <= 0)
    {
        CVI_LOGE("enDir:%d is not define\n", enDir);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return CVI_DTCF_ERR_UNDEFINE_DIR;
    }

    s32Ret = snprintf(pazDstFilePath, u32Length-1, "%s/%s/%s%s.%s", s_azRootDir, g_azDirNames[enDir], stSrcTmpDir.azTimeStr, g_szFileNameSuffix[enDir], g_szFileTypeSuffix[stSrcTmpDir.enType]);
    if(s32Ret <= 0)
    {
        CVI_LOGE("snprintf pazFilePath fail, s32Ret:%x\n", s32Ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return CVI_DTCF_ERR_EINVAL_PAEAMETER;
    }
    else if((uint32_t)s32Ret >= u32Length)
    {
        CVI_LOGE("snprintf u32Length:%d is not length enough, s32Ret:%d\n", u32Length, s32Ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return CVI_DTCF_ERR_EINVAL_PAEAMETER;
    }

    DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
    return 0;
}

int32_t CVI_DTCF_GetEmrFilePath(const char *pazSrcFilePath, char *pazDstFilePath, uint32_t u32Length)
{
    int32_t s32Ret = 0;
    CHECK_DTCF_NULL_PTR(pazSrcFilePath);
    CHECK_DTCF_NULL_PTR(pazDstFilePath);
    CHECK_DTCF_STATUS_CREATED(s_enDtcfStatu);
    CHECK_DTCF_FILE_PATH_LENGTH(u32Length);
    CVI_DTCF_TEMP_DIR_S stSrcTmpDir = {{0},0,0,0,{0}};

    DTCF_THREAD_MUTEX_LOCK(s_pthDtcfInterface_lock);
    s32Ret = DTCF_checkFilePath(pazSrcFilePath,&stSrcTmpDir);
    if(0 != s32Ret)
    {
        CVI_LOGE("DTCF_checkFilePath error s32Ret:%d\n",s32Ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return s32Ret;
    }

    CVI_DTCF_DIR_E enEmrDir;
    switch(stSrcTmpDir.enDir)
    {
        case DTCF_DIR_NORM_FRONT:
        {
            enEmrDir = DTCF_DIR_EMR_FRONT;
            break;
        }
        case DTCF_DIR_NORM_FRONT_SUB:
        {
            enEmrDir = DTCF_DIR_EMR_FRONT_SUB;
            break;
        }
        case DTCF_DIR_NORM_REAR:
        {
            enEmrDir = DTCF_DIR_EMR_REAR;
            break;
        }
        case DTCF_DIR_NORM_REAR_SUB:
        {
            enEmrDir = DTCF_DIR_EMR_REAR_SUB;
            break;
        }
        default:
        {
            DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
            return CVI_DTCF_UNSUPPORT_PATH;
        }
    }

    if((CVI_DTCF_FILE_TYPE_MP4 != stSrcTmpDir.enType) && (CVI_DTCF_FILE_TYPE_TS != stSrcTmpDir.enType))
    {
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return CVI_DTCF_UNSUPPORT_PATH;
    }
    s32Ret = snprintf(pazDstFilePath, u32Length-1, "%s/%s/%s%s.%s", s_azRootDir, g_azDirNames[enEmrDir], stSrcTmpDir.azTimeStr, g_szFileNameSuffix[enEmrDir], g_szFileTypeSuffix[stSrcTmpDir.enType]);
    if(s32Ret <= 0)
    {
        CVI_LOGE("snprintf pazFilePath fail, s32Ret:%d\n", s32Ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return CVI_DTCF_ERR_EINVAL_PAEAMETER;
    }
    else if((uint32_t)s32Ret >= u32Length)
    {
        CVI_LOGE("snprintf u32Length:%d is not length enough, s32Ret:%d\n", u32Length, s32Ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return CVI_DTCF_ERR_EINVAL_PAEAMETER;
    }

    DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
    return 0;
}

int32_t CVI_DTCF_GetFileDirType(const char *pazSrcFilePath, CVI_DTCF_DIR_E *penDir)
{
    int32_t s32Ret = 0;
    CHECK_DTCF_NULL_PTR(pazSrcFilePath);
    CHECK_DTCF_NULL_PTR(penDir);
    CHECK_DTCF_STATUS_CREATED(s_enDtcfStatu);
    CVI_DTCF_TEMP_DIR_S stSrcTmpDir = {{0},0,0,0,{0}};

    DTCF_THREAD_MUTEX_LOCK(s_pthDtcfInterface_lock);
    s32Ret = DTCF_checkFilePath(pazSrcFilePath,&stSrcTmpDir);
    if(0 != s32Ret)
    {
        CVI_LOGE("DTCF_checkFilePath error s32Ret:%d\n",s32Ret);
        DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
        return s32Ret;
    }
    *penDir = stSrcTmpDir.enDir;
    DTCF_THREAD_MUTEX_UNLOCK(s_pthDtcfInterface_lock);
    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
