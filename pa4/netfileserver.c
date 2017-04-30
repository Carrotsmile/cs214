#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/socket.h>
#include <time.h>
#include <sys/time.h>
#include <netdb.h>
#include <sys/stat.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>

extern int errno;
extern int h_errno;


#define PORT 25565
#define MAX_CLIENTS 10
#define MAX_SOCKETS 10
#define MAX_STREAMS_PER_CLIENT 4
#define TIMEOUT 2

typedef struct sock_client
{
	int port;
	int client;

	int client_socket;
	int fildes;
	int read_bytes;
	size_t nbytes;

	char * buf;
	struct sockaddr_in serv_addr;
	socklen_t addrLen;
} sock_client;

sock_client socks[MAX_SOCKETS];

int getbindsocket( sock_client * cli, int port)
{
	int sockFD;
	struct sockaddr_in serv_addr;
	sockFD = socket(AF_INET, SOCK_STREAM, 0);
	if(sockFD < 0)
	{
		printf("could not create socket for port: %d\n", port);
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	socklen_t addrLen = sizeof(serv_addr);

	int bind_sock = bind(sockFD, (struct sockaddr *) &serv_addr, addrLen);
	if(bind_sock == -1)
	{
		printf("could not bind socket for port: %d\n", port);
		return -1;
	}
	cli->serv_addr = serv_addr;
	cli->addrLen = addrLen;
	return sockFD;
}

void netopen(int client)
{

}

void netread(int client)
{

}

void netwrite(int client)
{

}

void netclose(int client)
{

}
//start a new thread for each client
void *client_service(void * st)
{
	int client = *(int*) st;
	int t;
	int type;
	while(t < sizeof(type))
	{
		t += recv(client, &type, sizeof(t), 0);
		if(t == 0)
		{
			continue;
		}
	}

	switch(type)
	{
		case 1:
			netopen(client);
			break;
		case 2:
			netread(client);
			break;
		case 3:
			netwrite(client);
			break;
		case 4:
			netclose(client);
			break;
		default:
			break;
	}
	close(client);
	free(st);
	pthread_exit(NULL);
}

int main(int argc, char * argv[])
{
	pthread_t tid;
	//if signum is signaled to the process, then it will be ignored
	int sig_error = signal(SIGPIPE, SIG_IGN);
	if(sig_error == SIG_ERR)
	{
		printf("[%d] %s\n", errno, strerror(errno));
		return -1;
	}

	int sockfd;
	char ipstr[INET6_ADDRSTRLEN];
	struct sockaddr_in serv_addr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		printf("Could not open socket on server.\n");
		exit(1);
	}

	//assign sockaddr fields
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	socklen_t addrLen = sizeof(serv_addr);

	//now try to bind the sockfd to a port
	int bind_sock = bind(sockfd, (struct sockaddr *) &serv_addr, addrLen);
	if(bind_sock == -1)
	{
		printf("could not bind the socket, no open connections\n");
		exit(0);
	}

	//initialize the sockets in the array
	int i;
	int port_i = 2 + PORT;
	errno = 0;
	for(i = 0; i < MAX_SOCKETS; i++)
	{
		socks[i].client = getbindsocket(&socks[i], port_i);
		socks[i].port = port_i;
		if(socks[i].client == -1)
		{
			//client socket not initialized
		}
		socks[i].client_socket = 0;
		port_i += 2;
	}

	//start listening for clients
	int status = listen(sockfd, MAX_CLIENTS);
	if(status < 0)
	{
		printf("Listen error\n");
		return 0;
	}
	printf("Listening on port %d\n", PORT);

	//start waiting for a client to connect and perform on of the four main functions
	while(1)
	{
		int client = accept(sockfd, (struct sockaddr *) &serv_addr, &addrLen);
		int * client_number = (int*) malloc(sizeof(int));

		*client_number = client;
		inet_ntop(AF_INET, &serv_addr.sin_addr.s_addr, ipstr, sizeof(ipstr));
		printf("connection established!\n");
		pthread_create(&tid, NULL, client_service, (void*) client_number);
	}

	pthread_exit(NULL);
}
