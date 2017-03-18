CC=gcc
CFLAGS= -g 

default: server client

client: client.o
	$(CC) $(CFLAGS) -o client client.c -lrt
	
server: server.o
	$(CC) $(CFLAG) vector.c server.c -o server -lrt

clean:
	rm -rf client server *.o
