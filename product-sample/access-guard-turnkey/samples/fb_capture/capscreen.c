#include "./iolib/iolib.h"
#define MAX_LINE 100
#define CAPTUREPORT 8888

int getUdpServerSocketId(void)
{

	struct sockaddr_in servaddr;
	int sockfd;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd == -1)
	{
		perror("create udp socket....\n");
	}
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(CAPTUREPORT);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
/*	inet_aton(argv[1], &servaddr.sin_addr.s_addr);
	if(inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
	{
		printf("set ip address error!\n");
		exit(0);
	}*/
 	if(bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1)
 	{
		perror("call to bind");
     	exit(1);
	}
 	return sockfd;
}

int sendbigdata(int fd, char* buf, int n, int flag, struct sockaddr *addr, int len)
{
	int i=n;
	int tmp = 0;
	while(i > 0){
		tmp = sendto(fd, buf+n-i, i > (10*1024)? (10*1024):i, 0, (struct sockaddr *) addr, len);
		i -= tmp;
	}
	return i;
}

int main(int argc, char *argv[])
{
	char imagebuf[5*1024*1024];
	int n;

	memset(imagebuf, 0, sizeof(imagebuf));
	printf("%s, %d, \n",__func__,__LINE__);
	n = Captureimage(imagebuf);

	return 0;
}

