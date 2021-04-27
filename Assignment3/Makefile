CC = gcc
CFLAGS = -c -g -Wall -Werror
EXE = proddonuts consdonuts

all: $(EXE)

$(EXE): donuts.h proddonuts.o consdonuts.o utilities.o
	$(CC) -o proddonuts proddonuts.o utilities.o
	$(CC) -o consdonuts consdonuts.o utilities.o

proddonuts.o: donuts.h proddonuts.c
	$(CC) $(CFLAGS) proddonuts.c

consdonuts.o: donuts.h consdonuts.c
	$(CC) $(CFLAGS) consdonuts.c

utilities.o: utilities.c
	$(CC) $(CFLAGS) utilities.c

clean:
	rm -f $(EXE) *.o
