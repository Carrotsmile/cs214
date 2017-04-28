
#include "libnetfiles.h"

//port is made an 'not well known port', not associated with any services in /etc/services
#define PORT 25565
#define MAX_PORTS 10
#define LARGE_FILE 2048

//
struct sockaddr_in serv_addr;

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

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	struct hostent * server;
	server = gethostbyname(hostname);

	bcopy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length);
	return 0;

}

//make a socket connection with port
//returns "file descriptor" for socket
int makeSocket(int port)
{
	//socket address information, prepared here for later use
	struct sockaddr_in serv_addr_new;
	serv_addr_new.sin_family = AF_INET;
	serv_addr_new.sin_port = htons(port);

	//contains information about the host/server like
	//host name, host address, host address length, list of addresses
	//address type
	struct hostent *server;
	server = gethostbyname(hostname_lnf);

	//copy over the address to the serv_addr structure
	bcopy( (char*) server->h_addr, (char*) &serv_addr.sin_addr.s_addr, server->h_length);

	//store socket descriptor
	int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

	//socket_fd did not init correctly
	if( socket_fd == -1)
	{
		//record error number in errno and print it
		printf("[%d] %s\n", errno, strerror(errno));
		return -1;
	}

	addrLen = sizeof(serv_addr_new);

	//convert ip to human readable form (should sizeof(ip) be strlen(ip) instead?)
	inet_ntop(AF_INET, &serv_addr_new.sin_addr, ip, sizeof(ip));

	//try to connect to the file server
	int status_connect = connect(sockFD, (struct sockaddr *) &serv_addr_new, addrLen);

	//negative status_connect indicates error
	if(status_connect < 0)
	{
		errno = ETIMEDOUT;
		printf("[%d] %s\n", errno, strerror(errno));
		return -1;
	}

	//return socket descriptor
	return socket_fd;
}

//opens a file from the server
int netopen(const char * pathname, int flags)
{

	//flags must be O_RDWR, O_RDONLY, O_WRONLY
	if(flags < 0 || flags > 2)
	{
		h_errno = INVALID_FILE_MODE;
		printf("Invalid flags for netopen.\n");
		return -1;
	}

	//set global variables for later connecting
	mode = flags;
	hostname_lnf = pathname;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	struct hostent *server;
	server = gethostbyname(hostname_lnf);

	if(server == NULL)
	{
		h_errno = HOST_NOT_FOUND;
		printf("Host server not found.\n");
		mode = -1;
		return -1;
	}

	//since the server has been found, copy the name of it to serv_addr
	bcopy( (char*) server->h_addr, (char*)&serv_addr.sin_addr, server->h_length );
	return 0;
}

ssize_t netread(int fildes, void * buf, size_t nbyte)
{

	//if signum is signaled to the process, then it will be ignored
	int sig_error = signal(SIGPIPE, SIG_IGN);
	if(sig_error == SIG_ERR)
	{
		printf("[%d] %s\n", errno, strerror(errno));
		return 0;
	}
	//check if mode is correct
	if( mode == -1 )
	{
		h_errno = HOST_NOT_FOUND;
		return -1;
	}
	//check if bytes requested is valid
	if( (int) nbyte < 0 )
	{
		errno = EINVAL;
		return -1;
	}
	//check if parameters are valid
	if( fildes >= 0 || buf == NULL )
	{
		errno = EBADF;
		return -1;
	}

	sockFD = socket(AF_INET, SOCK_STREAM, 0);
	//indicates socket not created
	if(sockFD < 0)
	{
		printf("[%d] %s\n", errno, strerror(errno));
		return -1;
	}

	//TODO, create function that does this
	addrLen = sizeof(serv_addr);
	inet_ntop(AF_INET, &serv_addr.sin_addr, ip, sizeof(ip));
	int status = connect(sockFD, (struct sockaddr *) &serv_addr, addrLen);
	if(status < 0)
	{
		errno = ETIMEDOUT;
		printf("[%d] %s\n", errno, strerror(errno));
		return -1;
	}

	//store the type to send
	int t = 2;
	//store the port to recieve from
	int port_in = 0;
	int port_bytes = 0;

	//first, send the type, file descriptor, and file size to the server
	int type = send(sockFD, &t, sizeof(t), 0);
	int count_fildes = send(sockFD, &fildes, sizeof(fildes), 0);
	int count_nbyte = send(sockFD, &nbyte, sizeof(nbyte), 0);

	//check if the server got these
	if(type == -1 || count_fildes == -1 || count_nbyte)
	{
		printf("Error sending\n");
		close(sockFD);
		return -1;
	}

	//try to read port
	port_bytes = recv(sockFD, &port_in, sizeof(port_in), 0);
	if(port_bytes == -1)
	{
		printf("[%d] %s\n", errno, strerror(errno));
	}

	//close sockFD
	close(sockFD);

	//now that we know the server got the file descriptor info, we can readable
	int recv_bytes = 0;
	int tmp_bytes = 0;
	int client = makeSocket(port_in);
	while(recv_bytes < nbyte)
	{
		errno = 0;
		tmp_bytes = recv(client, buf, nbyte, 0);
		if(tmp_bytes == 0 && recv_bytes < nbyte)
		{
			errno = ECONNRESET;
			close(client);
			return 0;
		}
		if(tmp_bytes == -1)
		{
			close(client);
			return 0;
		}
		recv_bytes += tmp_bytes;
	}
	printf("Recieved %d / %zd\n", recv_bytes, nbyte);
	close(client);
	return 0;
}

ssize_t netwrite(int fildes, const void * buf, size_t nbyte)
{
	//if signum is signaled to the process, then it will be ignored
	int sig_error = signal(SIGPIPE, SIG_IGN);
	if(sig_error == SIG_ERR)
	{
		printf("[%d] %s\n", errno, strerror(errno));
		return 0;
	}
	//check if mode is correct
	if( mode == -1 )
	{
		h_errno = HOST_NOT_FOUND;
		return -1;
	}
	//check if bytes requested is valid
	if( (int) nbyte < 0 )
	{
		errno = EINVAL;
		return -1;
	}
	//check if parameters are valid
	if( fildes >= 0 || buf == NULL )
	{
		errno = EBADF;
		return -1;
	}

	sockFD = socket(AF_INET, SOCK_STREAM, 0);
	//indicates socket not created
	if(sockFD < 0)
	{
		printf("[%d] %s\n", errno, strerror(errno));
		return -1;
	}

	//TODO, create function that does this
	addrLen = sizeof(serv_addr);
	inet_ntop(AF_INET, &serv_addr.sin_addr, ip, sizeof(ip));
	int status = connect(sockFD, (struct sockaddr *) &serv_addr, addrLen);
	if(status < 0)
	{
		errno = ETIMEDOUT;
		printf("[%d] %s\n", errno, strerror(errno));
		return -1;
	}

	int t = 3;
	//first, send the type, file descriptor, and file size to the server
	int type = send(sockFD, &t, sizeof(type), 0);
	int count_fildes = send(sockFD, &fildes, sizeof(fildes), 0);
	int count_nbyte = send(sockFD, &nbyte, sizeof(nbyte), 0);
	if(type == -1 || count_fildes == -1 || count_nbyte == -1)
	{
		printf("Sending error.\n");
		close(sockFD);
		return -1;
	}

	int port = 0;
	int port_get_error = 0;
	port_get_error = recv(sockFD, &port, sizeof(port), 0);
	if(port_get_error == -1)
	{
		//did not get port to send to
	}

	close(sockFD);
	//create new socket and send data
	int client = makeSocket(port);
	int data_sent = send(client, buf, nbyte, 0);
	int err = 0;
	int err_bytes = 0;

	//try to get an error to see if server recieved the data
	while(err_bytes < sizeof(err))
	{
		int temp = recv(client, &err, sizeof(err), 0);
		if(temp == 0)
		{
			errno = ECONNRESET;
			close(client);
			return -1;
		}
		err_bytes += temp;
	}

	if(err_bytes != sizeof(int))
	{
		printf("Did not recieve error!\n");
		return 0;
	}
	if(err != 0)
	{
		errno = err;
		printf("[%d] %s\n", errno, strerror(errno));
		return -1;
	}
	close(client);
	return 0;

}

int netclose(int fd)
{
	if(mode == -1)
	{
		h_errno = HOST_NOT_FOUND;
		return -1;
	}

	if(fd >= 0)
	{
		errno = EBADF;
		return -1;
	}

	//TODO replace this with a function that automatically does it
	sockFD = socket(AF_INET, SOCK_STREAM, 0);
	if(sockFD == -1)
	{
		printf("[%d] %s\n", errno, strerror(errno));
		return -1;
	}
	addrLen = sizeof(serv_addr);
	inet_ntop(AF_INET, &serv_addr.sin_addr, ip, sizeof(ip));
	int status = connect(sockFD, (struct sockaddr *) &serv_addr, addrLen);
	if(status < 0)
	{
		errno = ETIMEDOUT;
		printf("[%d] %s\n", errno, strerror(errno));
		return -1;
	}

	int t = 4;
	int err = 0;
	//TODO create function that sends initial byte information about a file
	int type = send(sockFD, &t, sizeof(type), 0);
	int count_fildes = send(sockFD, &fd, sizeof(fd), 0);

	if(type == -1 || count_fildes == -1)
	{
		printf("Error sending.\n");
		close(sockFD);
		return -1;
	}

	int recv_err = 0;
	while(recv_err < sizeof(err))
	{
		recv_err = recv(sockFD, &err, sizeof(err), 0);
		if(recv_err == 0)
		{
			printf("no data.\n");
			return -1;
		}
	}
	if(err != 0)
	{
		errno = err;
		err = -1;
	}
	return err;
}
