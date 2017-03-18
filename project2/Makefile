CC=gcc
CFLAGS= -g

default: server client

client: client.o
	$(CC) $(CFLAGS) -o client client.c
	
server: server.o
	$(CC) $(CFLAG) -o server server.c

clean:
	rm -rf client server *.o
