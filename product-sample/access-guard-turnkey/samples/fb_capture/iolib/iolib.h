#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <syslog.h>

extern ssize_t my_read(int fd, void *buffer, size_t length);
extern ssize_t my_write(int fd, void *buffer, size_t length);
extern int checkSelect(int handfd,char *rwflag);
