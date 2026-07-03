CC=gcc
CFLAGS=-Wall -Wextra -pthread -std=c11

all: mts

mts: mts.c
	$(CC) $(CFLAGS) -o mts mts.c

clean:
	rm -f mts output.txt