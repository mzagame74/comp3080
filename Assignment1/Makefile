CC = gcc
CFLAGS = -c -g -Wall -Werror 
OBJS = a1.o
DEPS = a1.c
EXE = a1

all: $(EXE)

$(EXE): $(OBJS)
	$(CC) -o $(EXE) $(OBJS)

$(OBJS): $(DEPS)
	$(CC) $(CFLAGS) $(DEPS)

clean:
	rm -f $(EXE) $(OBJS)
