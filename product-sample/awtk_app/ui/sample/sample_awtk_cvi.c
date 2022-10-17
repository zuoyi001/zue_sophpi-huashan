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

#include "window_common.h"


int main(int argc, char *argv[])
{
	// SAMPLE_AWTK_initCVI();
	uiapp_start();

	while(1);
	uiapp_stop();
	// SAMPLE_AWTK_stopCVI();

	return 0;
}

