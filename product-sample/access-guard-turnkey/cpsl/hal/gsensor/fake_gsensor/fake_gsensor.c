
#include <stdio.h>
#include <unistd.h>
#include "cvi_hal_gsensor.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

int gsensor_init(void)
{
	/* TODO */
	return 0;
}
int gsensor_deinit(void)
{
	/* TODO */
	return 0;
}
int gsensor_i2c_bus_init(int busnum)
{
	/* TODO */
	(void) busnum;
	return 0;
}
int gsensor_i2c_bus_deinit(void)
{
	/* TODO */
	return 0;
}
int gsensor_read_data(short *x, short *y, short *z)
{
	/* TODO */
	(void) x;
	(void) y;
	(void) z;
	return 0;
}
int gsensor_set_sensitity(unsigned char num)
{
	/* TODO */
	(void) num;
	return 0;
}
int gsensor_read_int_status(unsigned char *flag)
{
	/* TODO */
	(void) flag;
	return 0;
}
CVI_HAL_GSENSOR_OBJ_S gsensorObj = {
	.i2c_bus_init = gsensor_i2c_bus_init,
	.i2c_bus_deinit = gsensor_i2c_bus_deinit,
	.init = gsensor_init,
	.deinit = gsensor_deinit,
	.read_data = gsensor_read_data,
	.set_sensitity = gsensor_set_sensitity,
	.read_int_status = gsensor_read_int_status,
};
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
