#include "stdio.h"
#include "cvi_vpss.h"


int main(void)
{
    CVI_S32 s32Ret;
    VIDEO_FRAME_INFO_S stVideoFrame;
    s32Ret = CVI_VPSS_GetChnFrame(0, 2,	&stVideoFrame, -1);

    if (s32Ret != CVI_SUCCESS) {
        printf("vpss get frame error.\n");
    }

    printf("frame info: %d, %d, %d.\n",stVideoFrame.stVFrame.u32Width, \
        stVideoFrame.stVFrame.u32Height,stVideoFrame.stVFrame.enPixelFormat);

    return 0;
}
