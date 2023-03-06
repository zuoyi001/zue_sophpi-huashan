#include "cviapp_lt9611.h"
#include "sample_comm.h"

#define CVIAPP_AO_PCM_CARD_0    1

int main(int argc, char **argv)
{
	//CVI_S32 s32Ret = CVI_SUCCESS;
	//CVI_S32 op;

	UNUSED(argc);
	UNUSED(argv);

    CVIAPP_Lt9611Init(E_CVIAPP_LT9611_VIDEO_1920x1080_60HZ);
    //CVIAPP_VoInit(VO_OUTPUT_1080P60);

    //CVIAPP_AoInit();        //cvi_ao
   // CVIAPP_AO_Init(CVIAPP_AO_PCM_CARD_0);

}
