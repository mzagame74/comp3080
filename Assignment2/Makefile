CC = gcc
CFLAGS = -c -g -Wall -Werror

all: sort_ck grep_ck

sort_ck: sort_ck.o
	$(CC) -o sort_ck sort_ck.o

grep_ck: grep_ck.o
	$(CC) -o grep_ck grep_ck.o

sort_ck.o: sort_ck.c
	$(CC) $(CFLAGS) sort_ck.c

grep_ck.o: grep_ck.c
	$(CC) $(CFLAGS) grep_ck.c

clean:
	rm -f sort_ck grep_ck *.o
