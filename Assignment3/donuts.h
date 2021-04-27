#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/signal.h>

#define SEMKEY (key_t)324971480
#define MEMKEY (key_t)324971480
#define NUMFLAVORS 4
#define NUMSLOTS 40
#define NUMSEMIDS 3
#define PRODUCER 0
#define CONSUMER 1
#define OUTPTR 2

struct donut_ring {
    int flavor[NUMFLAVORS][NUMSLOTS];
    int outptr[NUMFLAVORS];
};

extern int p(int, int);
extern int v(int, int);
extern int semsetall(int, int, int);
