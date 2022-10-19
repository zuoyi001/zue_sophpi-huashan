
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/vfs.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <netinet/if_ether.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <time.h>
#include <sys/sysinfo.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

int get_partition_total_size(const char* path)
{
    struct statfs diskInfo;  
    statfs(path, &diskInfo);  
    unsigned long long totalBlocks = diskInfo.f_bsize;  
    unsigned long long totalSize = totalBlocks * diskInfo.f_blocks;  
    size_t mbTotalsize = totalSize>>20;  
    unsigned long long freeDisk = diskInfo.f_bfree*totalBlocks;  
    size_t mbFreedisk = freeDisk>>20;  
    return mbTotalsize;

}

int get_partition_available_size(const char* path)
{
    struct statfs diskInfo;  
    statfs(path, &diskInfo);  
    unsigned long long totalBlocks = diskInfo.f_bsize;  
    unsigned long long totalSize = totalBlocks * diskInfo.f_blocks;  
    size_t mbTotalsize = totalSize>>20;  
    unsigned long long freeDisk = diskInfo.f_bfree*totalBlocks;  
    size_t mbFreedisk = freeDisk>>20;  
    return mbFreedisk;
}


void get_mac(char* card,char* mac_addr)
{

    //unsigned char macaddr[ETH_ALEN]; //ETH_ALEN（6）是MAC地址长度
    struct ifreq req;

    int err,i;

    int s=socket(AF_INET,SOCK_DGRAM,0); //internet协议族的数据报类型套接口

    req.ifr_addr.sa_family = AF_INET;
    strcpy(req.ifr_name,card); //将设备名作为输入参数传入

    err=ioctl(s,SIOCGIFHWADDR,&req); //执行取MAC地址操作

    close(s);

    if(err != -1)
    { 
        unsigned char *mac;
        mac = (unsigned char *)req.ifr_hwaddr.sa_data;
        sprintf(mac_addr, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }
}


int get_time()
{
    time_t  now;
    struct  tm  *w;
    time(&now);
    w=localtime(&now);
    printf("%04d/%02d/%02d\n%02d:%02d:%02d\n",w->tm_year+1900,
            w->tm_mon+1,w->tm_mday,w->tm_hour,w->tm_min,w->tm_sec);

    return 0;
}



  
int print_system_boot_time()
{
    struct sysinfo info;
    time_t cur_time = 0;
    time_t boot_time = 0;
    struct tm *ptm = NULL;
    if (sysinfo(&info)) {
    fprintf(stderr, "Failed to get sysinfo, errno:%u, reason:%s\n",
        errno, strerror(errno));
    return -1;
    }
    time(&cur_time);
    if (cur_time > info.uptime) {
    boot_time = cur_time - info.uptime;
    }
    else {
    boot_time = info.uptime - cur_time;
    }
    ptm = gmtime(&boot_time);
    printf("System boot time: %d-%-d-%d %d:%d:%d\n", ptm->tm_year + 1900,
        ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
    return 0; 
}

int get_system_boot_time(char* time)
{
    FILE *fp;
    char file_path[] = "/tmp/boottime";
    fp = fopen(file_path,"r");
    if (NULL == fp) {
        return -1;
    }

    char *line = NULL;
    size_t len = 0;
    char boot_time_prefix[] = "Total boottime:";
    while (getline(&line, &len, fp) != -1) {
        char *pch;
        pch = strstr(line, boot_time_prefix);
        if (NULL != pch) {
            sprintf(time, "%s", pch + sizeof(boot_time_prefix));
        }
    }

    fclose(fp);

    if (line) {
        free(line);
    }

    return 0;
}

void get_current_date(char *date)
{
    time_t now = time(NULL);
	struct tm tm = *localtime(&now);
    char format_date[64];
	snprintf(format_date, sizeof(format_date), "%04d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900,
		tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    strncpy(date, format_date, strlen(format_date));
}

static void get_network_config(char *interface, unsigned long request, char *result)
{
    struct ifreq ifr;
    int err,i;

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, interface, IFNAMSIZ-1);
    err = ioctl(fd, request, &ifr);

    close(fd);

    if (err != -1) {
        char *result_address = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
        strncpy(result, result_address, strlen(result_address));
    }
}

void get_network_ip_address(char *interface, char *ip_address)
{
    get_network_config(interface, SIOCGIFADDR, ip_address);
}

void get_network_netmask(char *interface, char *netmask)
{
    get_network_config(interface, SIOCGIFNETMASK, netmask);
}

void get_network_broadcast(char *interface, char *broadcast_address)
{
    get_network_config(interface, SIOCGIFBRDADDR, broadcast_address);
}
