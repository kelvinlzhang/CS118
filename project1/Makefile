CC=gcc
CFLAGS=-I. -g -O -Wall -Wextra
OBJ = webserver.o

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

webserver: $(OBJ);
	$(CC) -o $@ $^ $(CFLAGS)
	./$@

clean:
	rm *.o

