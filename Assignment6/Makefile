CC = gcc
CFLAGS = -c -g -Wall -Werror
DEPS = main.c
OBJS = main.o
EXE = a6

all: $(EXE)

$(EXE): $(OBJS)
	$(CC) -o $(EXE) $(OBJS)

$(OBJS): $(DEPS)
	$(CC) $(CFLAGS) $(DEPS)

clean:
	rm -f $(EXE) *.o
