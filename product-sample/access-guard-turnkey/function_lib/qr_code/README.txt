1. zbar 吃的数据是YUV数据，实际上不管什么类型的数据zbar都会把他转换为Y800类型的YUV数据，也就是只有Y值的YUV数据

2. 在cv1835平台上通过
CVI_VPSS_GetChnFrame(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec);
来获取一帧的frame，如果要提取一帧的YUV数据，需要做虚拟地址的映射，具体代码实现如下：
void * Scanner_Thread(void)
{
    VIDEO_FRAME_INFO_S stVideoFrame;
    size_t image_size;
    CVI_VOID *vir_addr;
    CVI_U32 plane_offset;
    while (1)
    {
        zscanner_init(640, 360);
        CVI_VPSS_GetChnFrame(0, 2, &stVideoFrame, 5000);
        image_size = stVideoFrame.stVFrame.u32Length[0] + stVideoFrame.stVFrame.u32Length[1] + stVideoFrame.stVFrame.u32Length[2];
        vir_addr = CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[0], image_size);
        CVI_SYS_IonInvalidateCache(stVideoFrame.stVFrame.u64PhyAddr[0], vir_addr, image_size);
        plane_offset = 0;
        if (stVideoFrame.stVFrame.u32Length[0] != 0) {
            stVideoFrame.stVFrame.pu8VirAddr[0] = vir_addr + plane_offset;
            plane_offset += stVideoFrame.stVFrame.u32Length[0];
        }
        zscanner_scan((char*)stVideoFrame.stVFrame.pu8VirAddr[0]);
        CVI_SYS_Munmap(vir_addr, image_size);
        CVI_VPSS_ReleaseChnFrame(0, 2, &stVideoFrame);
    }
}

3. 虚拟地址pu8VirAddr[]中的YUV分量
stVideoFrame.stVFrame.pu8VirAddr[0]	Y
stVideoFrame.stVFrame.pu8VirAddr[1]	U
stVideoFrame.stVFrame.pu8VirAddr[2]	V

4. zbar只吃Y分量即可（stVideoFrame.stVFrame.pu8VirAddr[0]）
