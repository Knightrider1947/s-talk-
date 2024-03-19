CC = gcc
CFLAGS = -I.

s-talk: main.o threads_sockets.o list.o
	$(CC) -o s-talk main.o threads_sockets.o list.o

main.o: main.c
	$(CC) $(CFLAGS) -c main.c 

threads_sockets.o: threads_sockets.c threads_sockets.h
	$(CC) $(CFLAGS) -c threads_sockets.c

clean:
	rm -f main.o threads_sockets.o s-talk
