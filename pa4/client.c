#include "libnetfiles.h"

int main(int argc, char * argv[])
{
	//test only for use on local machine
	char * hostname = "localhost";
	netserverinit(hostname, 1);

	char * file1 = "files/book1.pdf";
	int fd1 = netopen(file1, O_RDWR);

	char * file2 = "files/book2.djvu";
	int fd2 = netopen(file2, O_RDWR);

	char * file3 = "files/book3.pdf";
	FILE * file = fopen(file3, "r");
	fseek(file, 0L, SEEK_END);
	int size = ftell(file);

	char * txt = calloc(1, size);
	
	printf("Read: %zd\n", netread(fd1, txt, size));
	printf("Write: %zd\n", netwrite(fd2, txt, size));
	printf("[%d] %s\n", errno, strerror(errno));

	int fdc1 = netclose(fd1);
	printf("Netclosed %d\n [%d]%s\n", fdc1, errno, strerror(errno));
	free(txt);	
	return 0;
}
