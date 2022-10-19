#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/watchdog.h>
#include <sys/ioctl.h>

int main()
{
    int wdt_fd = -1;
    static const char V = 'V';
    static const int enable = WDIOS_ENABLECARD;
    unsigned int htimer_duration = 5;

    wdt_fd = open("/dev/watchdog", O_WRONLY, 0666);
    printf("wdt_fd = %d.\n", wdt_fd);
    write(wdt_fd, &V, 1);  /* Magic, see watchdog-api.txt in kernel */
    close(wdt_fd);
    wdt_fd = open("/dev/watchdog", O_WRONLY, 0666);
    printf("wdt_fd = %d.\n", wdt_fd);
    

    ioctl(wdt_fd, WDIOC_SETOPTIONS,  (void*) &enable);
    //ioctl(wdt_fd, WDIOC_SETTIMEOUT, &htimer_duration);
    ioctl(wdt_fd, WDIOC_KEEPALIVE, 0);

    printf("wangliang mark.\n");
    sleep(10);

    close(wdt_fd);
    return 0;
}

