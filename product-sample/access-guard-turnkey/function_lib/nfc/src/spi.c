#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include "spi.h"

static const char *device = "/dev/spidev32766.0";
static unsigned char mode = SPI_MODE_0;
static unsigned char bits = 8;
static unsigned int speed = 1 * 1000 * 1000;
//static unsigned short delay = 0;
static int fd_spidev;

static void hex_dump(const void *src, size_t length, size_t line_size, char *prefix)
{
	int i = 0;
	const unsigned char *address = src;
	const unsigned char *line = address;
	unsigned char c;
	if (NULL == src) {
		return;
	}
	printf("%s | ", prefix);
	while (length-- > 0) {
		printf("%02X ", *address++);
		if (!(++i % line_size) || (length == 0 && i % line_size)) {
			if (length == 0) {
				while (i++ % line_size)
					printf("__ ");
			}
			printf(" | ");  /* right close */
			while (line < address) {
				c = *line++;
				printf("%c", (c < 33 || c == 255) ? 0x2E : c);
			}
			printf("\n");
			if (length > 0)
				printf("%s | ", prefix);
		}
	}
}

static void SPI_Transfer(int fd, unsigned char *tx, unsigned char *rx, size_t lens, unsigned char bits)
{
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = lens,
		//.delay_usecs = delay,
		//.speed_hz = speed,
		.bits_per_word = bits,
	};

	PERIPHERAL_GPIO_SetValue(NFC_CS_GPIO, 0);

	if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) < 1) {
		perror("can't send spi message");
	}

	PERIPHERAL_GPIO_SetValue(NFC_CS_GPIO, 1);

	//hex_dump(tx, len, 32, "TX");
	//hex_dump(rx, len, 32, "RX");
}
static int SPI_Init_GPIO(PERIPHERAL_GPIO_NUM_E eGpio)
{
	int ret = 0;

	ret = PERIPHERAL_GPIO_Export(eGpio);
	if (ret < 0) {
		perror("can't export gpio port");
		return ret;
	}

	ret = PERIPHERAL_GPIO_DirectionOutput(eGpio);
	if (ret < 0) {
		perror("can't direct out gpio port");
		return ret;
	}

	return ret;
}
int SPI_Init()
{
	int ret = 0;

	ret = SPI_Init_GPIO(NFC_CS_GPIO);
	if (ret < 0) {
		perror("can't init nfc cs gpio port");
		return ret;
	}

	ret = SPI_Init_GPIO(NFC_RST_GPIO);
	if (ret < 0) {
		perror("can't init nfc rst gpio port");
		return ret;
	}

	fd_spidev = open(device, O_RDWR);
	if (fd_spidev < 0) {
		perror("can't open device");
		return fd_spidev;
	}
	printf("fd_spidev:%d\n", fd_spidev);
	//spi mode
	ret = ioctl(fd_spidev, SPI_IOC_WR_MODE32, &mode);
	if (ret == -1) {
		perror("can't set spi mode");
		return ret;
	}

	ret = ioctl(fd_spidev, SPI_IOC_RD_MODE32, &mode);
	if (ret == -1) {
		perror("can't get spi mode");
		return ret;
	}
	//bits per word
	ret = ioctl(fd_spidev, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1) {
		perror("can't set bits per word");
		return ret;
	}

	ret = ioctl(fd_spidev, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1) {
		perror("can't get bits per word");
		return ret;
	}

	//max speed hz
	ret = ioctl(fd_spidev, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1) {
		perror("can't set max speed hz");
		return ret;
	}

	ret = ioctl(fd_spidev, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1) {
		perror("can't get max speed hz");
		return ret;
	}

	printf("spi mode: 0x%x\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed / 1000);

	return ret;
}

void SPI_Uninit()
{
	if (fd_spidev) {
		close(fd_spidev);
	}
}
//*********************************************
//模式零           写数据
//*********************************************
void SPI_Send_Dat(unsigned char *dat)
{
	SPI_Transfer(fd_spidev, dat, NULL, 2, 8);
}

/*********************************************
模式零         读数据
*********************************************/
unsigned char SPI_Receiver_Dat(unsigned char *dat)
{
	unsigned char read_data[2];
	SPI_Transfer(fd_spidev, dat, read_data, 2, 8);
	return read_data[1];
}

/*********************************************
模式零         地址读数据
*********************************************/
unsigned char  SPIRead(unsigned char addr)
{
	unsigned char send_data[2];
	send_data[0] = (addr << 1) | 0x80;
	send_data[1] = 0x00;
	return SPI_Receiver_Dat(send_data);
}

/*********************************************
模式零         地址写数据
*********************************************/
void SPIWrite(unsigned char addr, unsigned char reg_value)
{
	unsigned char send_data[2];
	send_data[0] = (addr << 1) & 0x7e;
	send_data[1] = reg_value;
	SPI_Send_Dat(send_data);
}

//SPI连续写函数,用于WRITE FIFO函数
void SPIWrite_Sequence(unsigned char sequence_length, unsigned char addr, unsigned char *reg_value)
{
	unsigned char j, send_data[2];
	if (sequence_length == 0)
		return;

	send_data[0] = (addr << 1) & 0x7e;

	for (j = 0; j < sequence_length; j++) {
		send_data[1] = *(reg_value + j);
		SPI_Send_Dat(send_data);
	}
}

//SPI连续读函数,用于READ FIFO函数
void SPIRead_Sequence(unsigned char sequence_length, unsigned char addr, unsigned char *reg_value)
{
	unsigned char j, send_data[2];
	if (sequence_length == 0)
		return;

	send_data[0] = (addr << 1) | 0x80;
	send_data[1] = 0x00;

	for (j = 0; j < sequence_length; j++) {
		*(reg_value + j) = SPI_Receiver_Dat(send_data);
	}
}
