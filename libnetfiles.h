
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>

//contains functions for switching bytes to correct host order: ex: htons, etc.
#include <arpa/inet.h>

#include <netinet/in.h>
#include <pthread.h>
#include <netdb.h>
#include <time.h>
#include <errno.h>
#include <fcnt1.h>

#ifndef _LIBNETFILES_H_
#define _LIBNETFILES_H_

//base component of the program
int netopen(const char * pathname, int flags);
ssize_t netread(int fildes, void * buf, size_t nbyte);
ssize_t netwrite(int fildes, const void * buf, size_t nbyte);
int netclose(int fd);

int netserverinit(char * hostname, int filemode);

#endif // _LIBNETFILES_H_

