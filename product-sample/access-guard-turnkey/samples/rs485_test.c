#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/serial.h>
#include <sys/ioctl.h>
#include <asm-generic/ioctls.h>
#include <termios.h>
#include <stdbool.h>
#include <strings.h>

static int speed_arr[] = {B115200, B19200, B9600, B4800, B2400, B1200, B300};
static int buad_rate[] = {115200, 19200, 9600, 4800, 2400, 1200, 300};

int uart_set_param(int fd_uart, int speed, int flow_ctrl, int databits, int stopbits, char parity)
{
	int i;
	struct termios options;

	if (tcgetattr(fd_uart, &options) != 0) {
		perror("tcgetattr");
		return -1;
	}

	for (i = 0; i < sizeof(speed_arr) / sizeof(int); i++) {
		if (speed == buad_rate[i]) {
			cfsetispeed(&options, speed_arr[i]);
			cfsetospeed(&options, speed_arr[i]);
		}
	}

	bzero(&options, sizeof(options));
	options.c_cflag |= CLOCAL | CREAD;
	options.c_cflag &= ~CSIZE;

	switch (databits) {
	case 7:
		options.c_cflag |= CS7;
		break;
	case 8:
		options.c_cflag |= CS8;
		break;
	}

	switch (parity) {
	case 'n':
	case 'N':
		options.c_cflag &= ~PARENB;
		options.c_iflag &= ~INPCK;
		break;
	case 'o':
	case 'O':
		options.c_cflag |= (PARODD | PARENB);
		options.c_iflag |= INPCK;
		break;
	case 'e':
	case 'E':
		options.c_cflag |= PARENB;
		options.c_cflag &= ~PARODD;
		options.c_iflag |= INPCK;
		break;
	case 's':
	case 'S':
		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;
		break;
	default:
		fprintf(stderr, "Unsupported parity\n");
		return -1;
	}

	if (stopbits == 1)
		options.c_cflag &= ~CSTOPB;
	else if (stopbits == 2)
		options.c_cflag |= CSTOPB;

	options.c_cc[VTIME] = 0;
	options.c_cc[VMIN] = 0;

	tcflush(fd_uart, TCIFLUSH);

	options.c_oflag = ~ICANON;

	if ((tcsetattr(fd_uart, TCSANOW, &options)) != 0) {
		perror("tcsetattr");
		return -1;
	}

	return 0;
}

int main()
{
    int fd;

	/* Open your specific device (e.g., /dev/mydevice): */
	fd = open ("/dev/ttyS1", O_RDWR);
	if (fd < 0) {
		/* Error handling. See errno. */
        return -1;
	}
    //init_serial(fd, 115200, 8, 1, "N");

	if (uart_set_param(fd, 9600, 0, 8, 1, 'N') < 0) {
		printf("uart set param failed\n");
	} else {
		printf("uart set param ok\n");
	}

	struct serial_rs485 rs485conf;

	/* Enable RS485 mode: */
	rs485conf.flags |= SER_RS485_ENABLED;
	/* Set logical level for RTS pin equal to 1 when sending: */
	rs485conf.flags |= SER_RS485_RTS_ON_SEND;
	/* or, set logical level for RTS pin equal to 0 when sending: */
	//rs485conf.flags &= ~(SER_RS485_RTS_ON_SEND);
	/* Set logical level for RTS pin equal to 1 after sending: */
	//rs485conf.flags |= SER_RS485_RTS_AFTER_SEND;
	/* or, set logical level for RTS pin equal to 0 after sending: */
	rs485conf.flags &= ~(SER_RS485_RTS_AFTER_SEND);
	/* Set rts delay before send, if needed: */
	rs485conf.delay_rts_before_send = 5;
	/* Set rts delay after send, if needed: */
	rs485conf.delay_rts_after_send = 20;
	/* Set this flag if you want to receive data even whilst sending data */
	//rs485conf.flags |= SER_RS485_RX_DURING_TX;

	//printf("TIOCSRS485: %x \n\n",TIOCSRS485);

	if (ioctl(fd, TIOCSRS485, &rs485conf) < 0) {
		/* Error handling. See errno. */
        return -1;
	}
	/* Use read() and write() syscalls here... */
	write(fd,"123456789",9);

	sleep(2);

	write(fd,"987654321",9);

	/* Close the device when finished: */
	if (close(fd) < 0) {
		/* Error handling. See errno. */
	}

    return 0;
}
