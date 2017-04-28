
CC = gcc
CFLAGS = -c -Wall -g
LFLAGS = -lm
DFLAGS = -pthread

all: client netfileserver

client: libnetfiles.o client.o
	$(CC) $(DFLAGS) libnetfiles.o client.o -o client $(LFLAGS)

netfileserver:
	$(CC) $(DFLAGS) netfileserver.c -o netfileserver $(LFLAGS)

libnetfiles.o: libnetfiles.c libnetfiles.h
	$(CC) $(CFLAGS) libnetfiles.c

netfileserver.o: netfileserver.c
	$(CC) $(CFLAGS) netfileserver.c

client.o: client.c libnetfiles.h
	$(CC) $(CFLAGS) client.c


clean:
	rm *.o netfileserver client

