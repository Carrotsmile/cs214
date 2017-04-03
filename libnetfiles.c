
#include "libnetfiles.h"

//port is made an 'not well known port', not associated with any services in /etc/services
#define PORT 25565
#define MAX_PORTS 10

//error information
extern int errno;
extern int h_errno;

//state information on file descriptors and relevant path info
int mode = -1;
int sockFD;
char ip[100];
char * hostname_lnf;
socklen_t addrLen;

int netserverinit(char * hostname, int filemode)
{
	//checks to see if the file mode is correct
	if(filemode < 0 || filemode > 2)
	{
		h_errno = INVALID_FILE_MODE;
		printf("Invalid filemode\n");
		return -1;		
	}

	hostname_lnf = hostname;
	mode = filemode;


}

//make a socket connection with port
int makeSocket(int port)
{

}

int netopen(const char * pathname, int flags)
{

}

ssize_t netread(int fildes, void * buf, size_t nbyte)
{

}

ssize_t netwrite(int fildes, const void * buf, size_t nbyte)
{

}

int netclose(int fd)
{

}
