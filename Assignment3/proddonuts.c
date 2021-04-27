#include "donuts.h"

int shmid, semid[3];
void sig_handler(int);

int main(int argc, char *argv[]) {
    int in_ptr[NUMFLAVORS];
    int serial[NUMFLAVORS];
    int i, j, nsigs;
    struct donut_ring *shared_ring;
    struct timeval randtime;
    unsigned short xsub1[3];
    struct sigaction new_action;
    sigset_t mask_sigs;
    int sigs[] = {SIGHUP, SIGINT, SIGQUIT, SIGBUS, SIGTERM, SIGSEGV, SIGFPE};

    nsigs = sizeof(sigs) / sizeof(int);
    sigemptyset(&mask_sigs);

    // producer initializes serial counters and in-pointers
    for (i = 0; i < NUMFLAVORS; i++) {
        in_ptr[i] = 0;
        serial[i] = 1;
    }

    // install signal handler for sigs
    for (i = 0; i < nsigs; i++) {
        sigaddset(&mask_sigs, sigs[i]);
    }
    for (i = 0; i < nsigs; i++) {
        new_action.sa_handler = sig_handler;
        new_action.sa_mask = mask_sigs;
        new_action.sa_flags = 0;
        if (sigaction(sigs[i], &new_action, NULL) == -1) {
            perror("sigaction failed: ");
            exit(1);
        }
    }

    // setup shared memory for ring buffer and allocate semaphores
    if ((shmid = shmget(MEMKEY, sizeof(struct donut_ring), IPC_CREAT | 0600))
    == -1) {
        perror("shared memory get failed: ");
        exit(1);
    }
    if ((shared_ring = (struct donut_ring *)shmat(shmid, NULL, 0)) ==
    (void *)-1) {
        perror("shared memory attach failed: ");
        sig_handler(-1);
    }
    for (i = 0; i < NUMSEMIDS; i++) {
        if ((semid[i] = semget(SEMKEY + i, NUMFLAVORS, IPC_CREAT | 0600)) == -1)
        {
            perror("semaphore allocation failed: ");
            sig_handler(-1);
        }
    }

    gettimeofday(&randtime, NULL);
    xsub1[0] = (ushort)randtime.tv_usec;
    xsub1[1] = (ushort)(randtime.tv_usec >> 16);
    xsub1[2] = (ushort)(getpid());      // unique value

    // initialize semaphore values with semsetall
    if (semsetall(semid[PRODUCER], NUMFLAVORS, NUMSLOTS) == -1) {
        perror("semsetall failed: ");
        sig_handler(-1);
    }
    if (semsetall(semid[CONSUMER], NUMFLAVORS, 0) == -1) {
        perror("semsetall failed: ");
        sig_handler(-1);
    }
    if (semsetall(semid[OUTPTR], NUMFLAVORS, 1) == -1) {
        perror("semsetall failed: ");
        sig_handler(-1);
    }

    // producer forever loop
    while (1) {
        j = nrand48(xsub1) & 3;     // random value 0-3
        printf("in producer with rand = %d\n", j);

        if (p(semid[PRODUCER], j) == -1) {          // producer waits
            perror("p operation failed: ");
            sig_handler(-1);
        }
        // produce random donut into correct ring buffer and increment counters
        shared_ring->flavor[j][in_ptr[j]] = serial[j];
        in_ptr[j] = (in_ptr[j] + 1) % NUMSLOTS;
        serial[j]++;

        printf("prod type %d serial %d\n", j, serial[j] - 1);

        if (v(semid[CONSUMER], j) == -1) {          // signal consumer
            perror("v operation failed: ");
            sig_handler(-1);
        }
    }

    return 0;
}

void sig_handler(int sig) {
    printf("In signal handler with signal #%d\n", sig);
    if (shmctl(shmid, IPC_RMID, 0) == -1) {
        perror("handler failed shmctl RMID: ");
    }
    for (int i = 0; i < NUMSEMIDS; i++) {
        if (semctl(semid[i], 0, IPC_RMID) == -1) {
            perror("handler failed semctl RMID: ");
        }
    }
    exit(5);
}
