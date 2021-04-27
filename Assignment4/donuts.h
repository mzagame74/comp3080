#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sched.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#define NUMFLAVORS 4
#define NUMSLOTS 1900
#define NUMCONSUMERS 50
#define NUMPRODUCERS 30
#define NUMDOZENS 2000

typedef struct {
    int flavor[NUMFLAVORS][NUMSLOTS];
    int outptr[NUMFLAVORS];
    int in_ptr[NUMFLAVORS];
    int serial[NUMFLAVORS];
    int spaces[NUMFLAVORS];
    int donuts[NUMFLAVORS];
} DONUT_SHOP;

void *sig_waiter(void *arg);
void *producer(void *arg);
void *consumer(void *arg);
void sig_handler(int sig);
