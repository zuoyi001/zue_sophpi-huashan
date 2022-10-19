#include <stdio.h>
#include <sys/time.h>
#include <netdb.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <netinet/ip_icmp.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <signal.h>
#include <math.h>

#include "ping.h"

 
#define ICMP_DATA_LEN 56		//ICMPĬ�����ݳ���
#define ICMP_HEAD_LEN 8			//ICMPĬ��ͷ������
#define ICMP_LEN  (ICMP_DATA_LEN + ICMP_HEAD_LEN)
#define SEND_BUFFER_SIZE 128		//���ͻ�������С
#define RECV_BUFFER_SIZE 128		//���ջ�������С
#define SEND_NUM 10 			//���ͱ�����
#define MAX_WAIT_TIME 3

 
char SendBuffer[SEND_BUFFER_SIZE];
char RecvBuffer[RECV_BUFFER_SIZE];
int nRecv = 0;	//ʵ�ʽ��յ��ı�����
struct timeval FirstSendTime;	//���Լ����ܵ�ʱ��
struct timeval LastRecvTime;
double min = 0.0;
double avg = 0.0;
double max = 0.0;
double mdev = 0.0;

struct hostent * pHost = NULL;		//����������Ϣ
int sock_icmp;				//icmp�׽���
int nSend = 1;
char *IP = NULL;
int bRun = 1;  
u_int16_t Compute_cksum(struct icmp *pIcmp)
{
	u_int16_t *data = (u_int16_t *)pIcmp;
	int len = ICMP_LEN;
	u_int32_t sum = 0;
	
	while (len > 1)
	{
		sum += *data++;
		len -= 2;
	}
	if (1 == len)
	{
		u_int16_t tmp = *data;
		tmp &= 0xff00;
		sum += tmp;
	}
 
	//ICMPУ��ʹ���λ
	while (sum >> 16)
		sum = (sum >> 16) + (sum & 0x0000ffff);
	sum = ~sum;
	
	return sum;
}
 
void SetICMP(u_int16_t seq)
{
	struct icmp *pIcmp;
	struct timeval *pTime;
 
	pIcmp = (struct icmp*)SendBuffer;
	
	/* ���ͺʹ���ֱ�ΪICMP_ECHO,0����������� */
	pIcmp->icmp_type = ICMP_ECHO;
	pIcmp->icmp_code = 0;
	pIcmp->icmp_cksum = 0;		//У���
	pIcmp->icmp_seq = seq;		//���
	pIcmp->icmp_id = getpid();	//ȡ���̺���Ϊ��־
	pTime = (struct timeval *)pIcmp->icmp_data;
	gettimeofday(pTime, NULL);	//���ݶδ�ŷ���ʱ��
	pIcmp->icmp_cksum = Compute_cksum(pIcmp);
	
	if (1 == seq)
		FirstSendTime = *pTime;
}
 
void SendPacket(int sock_icmp, struct sockaddr_in *dest_addr, int nSend)
{
	SetICMP(nSend);
	if (sendto(sock_icmp, SendBuffer, ICMP_LEN, 0,
		(struct sockaddr *)dest_addr, sizeof(struct sockaddr_in)) < 0)
	{
		perror("sendto");
		return;
	}
}
 
double GetRtt(struct timeval *RecvTime, struct timeval *SendTime)
{
	struct timeval sub = *RecvTime;
 
	if ((sub.tv_usec -= SendTime->tv_usec) < 0)
	{
		--(sub.tv_sec);
		sub.tv_usec += 1000000;
	}
	sub.tv_sec -= SendTime->tv_sec;
	
	return sub.tv_sec * 1000.0 + sub.tv_usec / 1000.0; //ת����λΪ����
}
 
int unpack(struct timeval *RecvTime)
{
	struct ip *Ip = (struct ip *)RecvBuffer;
	struct icmp *Icmp;
	int ipHeadLen;
	double rtt;
 
	ipHeadLen = Ip->ip_hl << 2;	//ip_hl�ֶε�λΪ4�ֽ�
	Icmp = (struct icmp *)(RecvBuffer + ipHeadLen);
 
	//�жϽ��յ��ı����Ƿ����Լ��������ĵ���Ӧ
	if ((Icmp->icmp_type == ICMP_ECHOREPLY) && Icmp->icmp_id == getpid())
	{
		struct timeval *SendTime = (struct timeval *)Icmp->icmp_data;
		rtt = GetRtt(RecvTime, SendTime);
			
		printf("%u bytes from %s: icmp_seq=%u ttl=%u time=%.1f ms\n",
			ntohs(Ip->ip_len) - ipHeadLen,
			inet_ntoa(Ip->ip_src),
			Icmp->icmp_seq,
			Ip->ip_ttl,
			rtt);
		
		if (rtt < min || 0 == min)
			min = rtt;
		if (rtt > max)
			max = rtt;
		avg += rtt;
		mdev += rtt * rtt;
		
		return 0;
	}
		
	return -1;
}
 
 
void Statistics()
{
	double tmp;
	avg /= nRecv;
	tmp = mdev / nRecv - avg * avg;
	mdev = sqrt(tmp);
	
	if (NULL != pHost)
		printf("--- %s  ping statistics ---\n", pHost->h_name);
	else
		printf("--- %s  ping statistics ---\n", IP);
		
	printf("%d packets transmitted, %d received, %d%% packet loss, time %dms\n"
		, nSend
		, nRecv
		, (nSend - nRecv) / nSend * 100
		, (int)GetRtt(&LastRecvTime, &FirstSendTime));
	printf("rtt min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms\n",
		min, avg, max, mdev);
	
	close(sock_icmp);
	//exit(0);
}
 
int RecvePacket(int sock_icmp, struct sockaddr_in *dest_addr)
{
	int RecvBytes = 0;
	int addrlen = sizeof(struct sockaddr_in);
	struct timeval RecvTime;
 
	if ((RecvBytes = recvfrom(sock_icmp, RecvBuffer, RECV_BUFFER_SIZE,
			0, (struct sockaddr *)dest_addr, (socklen_t *)&addrlen)) < 0)
	{
		perror("recvfrom");
		return 0;
	}
	//printf("nRecv=%d\n", RecvBytes);
	gettimeofday(&RecvTime, NULL);
	LastRecvTime = RecvTime;
 
	if (unpack(&RecvTime) == -1)
	{
		return -1; 
	}
	nRecv++;

	return 0;
}

void setStop(int run)
{
	bRun = run;
}
void pingEx(char *ip)
{
 
	struct protoent *protocol;
	struct sockaddr_in dest_addr; 	//IPv4ר��socket��ַ,����Ŀ�ĵ�ַ
 
	in_addr_t inaddr;		//ip��ַ�������ֽ���

	if ((protocol = getprotobyname("icmp")) == NULL)
	{
		perror("getprotobyname");
		exit(EXIT_FAILURE);
	}
 
	/* ����ICMP�׽��� */
	//AF_INET:IPv4, SOCK_RAW:IPЭ�����ݱ��ӿ�, IPPROTO_ICMP:ICMPЭ��
	if ((sock_icmp = socket(PF_INET, SOCK_RAW, protocol->p_proto/*IPPROTO_ICMP*/)) < 0)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}
	dest_addr.sin_family = AF_INET;
 
	/* �����ʮ����ip��ַת��Ϊ�����ֽ��� */
	if ((inaddr = inet_addr(ip)) == INADDR_NONE)
	{
		/* ת��ʧ�ܣ�������������,��ͨ����������ȡip */
		if ((pHost = gethostbyname(ip)) == NULL)
		{
			herror("gethostbyname()");
			exit(EXIT_FAILURE);
		}
		memmove(&dest_addr.sin_addr, pHost->h_addr_list[0], pHost->h_length);
	}
	else
	{
		memmove(&dest_addr.sin_addr, &inaddr, sizeof(struct in_addr));
	}
 
	if (NULL != pHost)
		printf("PING %s", pHost->h_name);
	else
		printf("PING %s", ip);
	printf("(%s) %d bytes of data.\n", inet_ntoa(dest_addr.sin_addr), ICMP_LEN);
 
	IP = ip;
	nSend = 0;
	nRecv = 0;
	while ((1 == bRun)&&(nSend < SEND_NUM))
	{
		int unpack_ret;
		
		SendPacket(sock_icmp, &dest_addr, nSend);
		
		unpack_ret = RecvePacket(sock_icmp, &dest_addr);
		if (-1 == unpack_ret)	//��ping�ػ�ʱ���յ����Լ������ı���,���µȴ�����
			RecvePacket(sock_icmp, &dest_addr);
			
 
		sleep(1);
		nSend++;
	}
	
	Statistics();	//�����Ϣ���ر��׽���
}

