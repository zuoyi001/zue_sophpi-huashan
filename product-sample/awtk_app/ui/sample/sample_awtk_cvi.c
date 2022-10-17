#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <semaphore.h>

#include "window_common.h"

static sem_t s_ExitSem;

int main(int argc, char *argv[])
{
	sem_init(&s_ExitSem, 0, 0);

	uiapp_start();

	while((0 != sem_wait(&s_ExitSem)) && (errno == EINTR));

	uiapp_stop();

	return 0;
}

