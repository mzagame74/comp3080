CC = gcc
CFLAGS = -c -g
DEPS = main.c
OBJS = main.o
EXE = a5

all: $(EXE)

$(EXE): $(OBJS)
	$(CC) -o $(EXE) $(OBJS)

$(OBJS): $(DEPS)
	$(CC) $(CFLAGS) $(DEPS)

clean:
	rm -f $(EXE) *.o
