CC = gcc
CFLAGS = -c -g -Wall -Werror
LIBS = -lpthread
OBJS = pc_threads.o
EXE = pc_threads

all: $(EXE)

$(EXE): $(OBJS) donuts.h
	$(CC) -o $(EXE) $(OBJS) $(LIBS)

$(OBJS): donuts.h pc_threads.c
	$(CC) $(CFLAGS) pc_threads.c

clean:
	rm -f $(EXE) *.o
