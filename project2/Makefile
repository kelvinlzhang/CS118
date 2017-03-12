CC=gcc
CFLAGS= -g

default: server client

client: client.o
	$(CC) $(CFLAGS) packethandler.c -o client client.c
	
server: server.o
	$(CC) $(CFLAG) packethandler.c -o server server.c

clean:
	rm -rf client server *.o
