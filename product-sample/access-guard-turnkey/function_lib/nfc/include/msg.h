#ifndef MSG_H
#define MSG_H
#include <stdio.h>

#define SEND_MSG(fmt...)					\
	do {							\
		printf(fmt);					\
	} while (0)


#define SEND_HEX(data, lens)					\
	do {							\
		for (int i = 0; i < lens; i++) {		\
			printf("%02X", data[i]);		\
		}						\
	} while (0)
#endif
