#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "iolib.h"
ssize_t my_read(int fd, void *buf, size_t length) 
{ 
	ssize_t done = length; 

	while(done > 0) { 
		done = read(fd, buf, length); 
		
		if(done != length) 
			if(errno == EINTR) 
				done = length; 
			else{
				perror("fail to read"); 
				return -1; 
			}
		else
			break;
	} 

	return done; 
}

ssize_t my_write(int fd, void *buf, size_t length) 
{ 
	ssize_t done = length;

	while(done > 0) { 
		done = write(fd, buf, length);

		if(done != length) 
			if(errno == EINTR) 
				done = length; 
			else{
				perror("fail to write"); 
				return -1; 
			}
		else
			break;
	} 

	return done;
}
int checkSelect(int handfd,char *rwflag)
{
	int ret, retval=0xff;
    fd_set readfds, writefds;
	struct timeval	timeout;
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    static int numfds = 0;
    if (handfd >= numfds)
    {
       	numfds = handfd + 1;              /* Record maximum fd + 1 */
    }
    if (strchr(rwflag, 'r') != NULL)
    {
        FD_SET(handfd, &readfds);
    }
    if (strchr(rwflag, 'w') != NULL)
    {
        FD_SET(handfd, &writefds);
    }
	while (1)
	{
		/*设置阻塞时间为5秒*/
		timeout.tv_sec = 0;
		timeout.tv_usec = 20;
		retval =0xff;
		ret = select(numfds, &readfds, &writefds, NULL, &timeout);
		switch (ret)
		{
			case 0:
				//exit(0);
				break;
			case -1:
				perror("select");
				exit(1);
				break;
			default:
				 if (FD_ISSET(handfd, &readfds))
				 {
					 retval =0;
					 FD_CLR(handfd, &readfds);
					 FD_SET(handfd, &readfds);
				 }
				 if(FD_ISSET(handfd, &writefds))
				 {
					 retval =1;
					 FD_CLR(handfd, &writefds);
					 FD_SET(handfd, &writefds);
				 }
				 break;
		  }
		// printf("time out1--------------\n");
		 return retval;
	}
}
