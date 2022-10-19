#include "mir3da.h"
#include "cvi_hal_gsensor.h"
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define I2C_BUS_PREF "/dev/i2c-"
#define USAGE_MESSAGE                                   \
    "Usage:\n"                                          \
    "  %s r [addr] [register]   "                       \
    "to read value from [register]\n"                   \
    "  %s w [addr] [register] [value]   "               \
    "to write a value [value] to register [register]\n" \
    ""

static int set_i2c_register(int file, unsigned char addr, unsigned char reg,
			    unsigned char value)
{
	unsigned char outbuf[2];
	struct i2c_rdwr_ioctl_data packets;
	struct i2c_msg messages[1];

	messages[0].addr = addr;
	messages[0].flags = 0;
	messages[0].len = sizeof(outbuf);
	messages[0].buf = outbuf;

	/* The first byte indicates which register we'll write */
	outbuf[0] = reg;

	/*
	 * The second byte indicates the value to write.  Note that for many
	 * devices, we can write multiple, sequential registers at once by
	 * simply making outbuf bigger.
	 */
	outbuf[1] = value;

	/* Transfer the i2c packets to the kernel and verify it worked */
	packets.msgs = messages;
	packets.nmsgs = 1;
	if (ioctl(file, I2C_RDWR, &packets) < 0) {
		perror("Unable to send data");
		return 1;
	}

	return 0;
}

static int get_i2c_register(int file, unsigned char addr, unsigned char reg,
			    unsigned char *val)
{
	unsigned char inbuf, outbuf;
	struct i2c_rdwr_ioctl_data packets;
	struct i2c_msg messages[2];

	/*
	 * In order to read a register, we first do a "dummy write" by writing
	 * 0 bytes to the register we want to read from.  This is similar to
	 * the packet in set_i2c_register, except it's 1 byte rather than 2.
	 */
	outbuf = reg;
	messages[0].addr = addr;
	messages[0].flags = 0;
	messages[0].len = sizeof(outbuf);
	messages[0].buf = &outbuf;

	/* The data will get returned in this structure */
	messages[1].addr = addr;
	messages[1].flags = I2C_M_RD /* | I2C_M_NOSTART*/;
	messages[1].len = sizeof(inbuf);
	messages[1].buf = &inbuf;

	/* Send the request to the kernel and get the result back */
	packets.msgs = messages;
	packets.nmsgs = 2;
	if (ioctl(file, I2C_RDWR, &packets) < 0) {
		perror("Unable to send data");
		return 1;
	}
	*val = inbuf;

	return 0;
}

int i2c_file = 0;

int gsensor_i2c_bus_init(int busnum)
{
	char busname[15];
	sprintf(busname, "%s%d", I2C_BUS_PREF, busnum);
	if ((i2c_file = open(busname, O_RDWR)) < 0) {
		perror("Unable to open i2c control file");
		return -1;
	}
	return 0;
}

int gsensor_i2c_bus_deinit(void)
{
	close(i2c_file);
	return 0;
}
#define abs(x) (((x) < 0) ? -(x) : (x))
#define IDC_SADDR_G_Sensor_W 0x27
#define IDC_SADDR_G_Sensor_R 0x27
#define IDC_STYPE_G_Sensor IDC2

#ifdef KERNEL_DRV
void __I2CWriteByte8(unsigned char chip_addr, unsigned char reg_addr,
		     unsigned char value);
unsigned char __I2CReadByte8(unsigned char chip_addr, unsigned char reg_addr);

void __I2CWriteByte8(unsigned char chip_addr, unsigned char reg_addr,
		     unsigned char value)
{
	int ret;
	unsigned char buf[2];
	struct i2c_client *client = gsensor_i2c_client;

	gsensor_i2c_client->addr = chip_addr;

	buf[0] = reg_addr;
	buf[1] = value;
	// printf("reg_addr[%2x] value[%2x]\n", reg_addr, value);
	ret = i2c_master_send(client, buf, 2);
	udelay(300);
}

unsigned char __I2CReadByte8(unsigned char chip_addr, unsigned char reg_addr)
{
	int ret_data = 0xFF;
	int ret;
	struct i2c_client *client = gsensor_i2c_client;
	unsigned char buf[2];

	gsensori2c_client->addr = chip_addr;

	buf[0] = reg_addr;
	ret = i2c_master_recv(client, buf, 1);
	if (ret >= 0) {
		ret_data = buf[0];
	}
	return ret_data;
}
#endif
/*return value: 0: is ok    other: is failed*/
int i2c_read_byte_data(unsigned char addr, unsigned char *data)
{
	int ret = 0;
	ret = get_i2c_register(i2c_file, IDC_SADDR_G_Sensor_R, addr, data);
	return ret;
}

/*return value: 0: is ok    other: is failed*/
int i2c_write_byte_data(unsigned char addr, unsigned char data)
{
	int ret = 0;
	ret = set_i2c_register(i2c_file, IDC_SADDR_G_Sensor_W, addr, data);
	return ret;
}

/*return value: 0: is count    other: is failed*/
int i2c_read_block_data(unsigned char base_addr, unsigned char count,
			unsigned char *data)
{
	int i = 0;

	for (i = 0; i < count; i++) {
		if (i2c_read_byte_data(base_addr + i, (data + i))) {
			return -1;
		}
	}

	return count;
}

int mir3da_register_read(unsigned char addr, unsigned char *data)
{
	int res = 0;

	res = i2c_read_byte_data(addr, data);
	if (res != 0) {
		return res;
	}

	return res;
}

int mir3da_register_write(unsigned char addr, unsigned char data)
{
	int res = 0;

	res = i2c_write_byte_data(addr, data);
	if (res != 0) {
		return res;
	}

	return res;
}

int mir3da_register_read_continuously(unsigned char addr, unsigned char count,
				      unsigned char *data)
{
	int res = 0;

	res = (count == i2c_read_block_data(addr, count, data)) ? 0 : 1;
	if (res != 0) {
		return res;
	}

	return res;
}

int mir3da_register_mask_write(unsigned char addr, unsigned char mask,
			       unsigned char data)
{
	int res = 0;
	unsigned char tmp_data;

	res = mir3da_register_read(addr, &tmp_data);
	if (res) {
		return res;
	}

	tmp_data &= ~mask;
	tmp_data |= data & mask;
	res = mir3da_register_write(addr, tmp_data);

	return res;
}

static int mir3da_set_enable(char enable)
{
	int res = 0;
	if (enable)
		res = mir3da_register_mask_write(NSA_REG_POWERMODE_BW, 0xC0, 0x40);
	else
		res = mir3da_register_mask_write(NSA_REG_POWERMODE_BW, 0xC0, 0x80);
	return res;
}
/*return value: 0: is ok    other: is failed*/
int gsensor_init(void)
{
	int res = 0;
	unsigned char data = 0;

	mir3da_register_read(NSA_REG_WHO_AM_I, &data);
	if (data != 0x13) {
		printf("------mir3da read chip id  error= %x-----\r\n", data);
		return -1;
	}

	printf("------mir3da chip id = %x-----\r\n", data);

	res = mir3da_register_mask_write(NSA_REG_SPI_I2C, 0x24, 0x24);

	usleep(5000);

	res |= mir3da_register_mask_write(NSA_REG_G_RANGE, 0x03, 0x00);
	res |= mir3da_register_mask_write(NSA_REG_POWERMODE_BW, 0xFF, 0x34);
	res |= mir3da_register_mask_write(NSA_REG_ODR_AXIS_DISABLE, 0xFF, 0x06);
	res |= mir3da_register_write(0x16, 0x00);
	res |= mir3da_register_write(0x19, 0x0);
	mir3da_set_enable(1);
	return res;
}

int gsensor_deinit(void)
{
	mir3da_set_enable(0);
	return 0;
}

int gsensor_set_sensitity(unsigned char num)
{
	int res = 0;

	res |= mir3da_register_write(0x11, 0x34);
	res |= mir3da_register_write(0x10, 0x07);
	res |= mir3da_register_write(0x16, 0x07);
	res |= mir3da_register_write(0x19, 0x04);
	res |= mir3da_register_write(0x20, 0x01);
	res |= mir3da_register_write(0x21, 0x06);
	res |= mir3da_register_write(0x27, 0x01);
	res |= mir3da_register_write(0x28, num);

	return res;
}

int mir3da_close_interrupt(void)
{
	int res = 0;
	res = mir3da_register_write(NSA_REG_INTERRUPT_SETTINGS1, 0x00);
	res = mir3da_register_write(NSA_REG_INTERRUPT_MAPPING1, 0x00);
	return res;
}

/*return value: 0: is ok    other: is failed*/
int gsensor_read_data(short *x, short *y, short *z)
{
	unsigned char tmp_data[6] = {0};

	if (mir3da_register_read_continuously(NSA_REG_ACC_X_LSB, 6, tmp_data) !=
	    0) {
		return -1;
	}

	*x = ((short)(tmp_data[1] << 8 | tmp_data[0])) >> 4;
	*y = ((short)(tmp_data[3] << 8 | tmp_data[2])) >> 4;
	*z = ((short)(tmp_data[5] << 8 | tmp_data[4])) >> 4;

	return 0;
}

int gsensor_read_int_status(unsigned char *flag)
{
	unsigned char data = 0;
	int ret = 0;
	ret = mir3da_register_read(NSA_REG_MOTION_FLAG, &data);
	if (data & 0x04) {
		*flag = 1;
	} else {
		*flag = 0;
	}
	return ret;
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
#ifdef KERNEL_DRV
#define I2C_0 (0)
#define I2C_1 (1)
#define I2C_2 (2)
#define I2C_3 (3)

struct i2c_client *gsensor_i2c_client;

static struct i2c_board_info hi_info = {
	I2C_BOARD_INFO("gsensor", 0x64),
};

int gsensor_open(struct inode *inode, struct file *file);
int gsensor_close(struct inode *inode, struct file *file);
long gsensor_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

/*******************************************************************************************************
 * Initialize Function
 *
 *******************************************************************************************************/
int gsensor_open(struct inode *inode, struct file *file)
{
	return 0;
}

int gsensor_close(struct inode *inode, struct file *file)
{
	return 0;
}

long gsensor_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{


	return 0;
}
static int i2c_client_init(void)
{
	struct i2c_adapter *i2c_adap;

	i2c_adap = i2c_get_adapter(I2C_1);
	gsensor_i2c_client = i2c_new_device(i2c_adap, &hi_info);
	i2c_put_adapter(i2c_adap);

	return 0;
}

static void i2c_client_exit(void)
{
	i2c_unregister_device(gsensor_i2c_client);
}

static struct file_operations gsensor_fops = {.owner = THIS_MODULE,
	       .unlocked_ioctl = gsensor_ioctl,
		.open = gsensor_open,
		 .release = gsensor_close
};

static struct miscdevice gsensor_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "gsensor",
	.fops = &gsensor_fops,
};

static int __init gsensor_module_init(void)
{
	int ret = 0;
	unsigned char chip_id = 0xFF;

	ret = misc_register(&gsensor_dev);

	if (ret) {
		printf("ERROR: could not register gsensor devices:%#x \n", ret);
		return -1;
	}

	i2c_client_init();

	gsensor_init();
}

/*******************************************************************************
 *	Description		: It is called when "rmmod nvp61XX_ex.ko"
 *command run Argurments		: void Return value	: void Modify :
 *warning			:
 *******************************************************************************/
static void __exit gsensor_module_exit(void)
{
	misc_deregister(&gsensor_dev);
	i2c_client_exit();

	printf("%s\n", __func__);
}

module_init(gsensor_module_init);
module_exit(gsensor_module_exit);

MODULE_LICENSE("GPL");

/*******************************************************************************
 *	End of file
 *******************************************************************************/
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
