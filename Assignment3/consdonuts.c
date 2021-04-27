#include "donuts.h"

int shmid, semid[3];

int main(int argc, char *argv[]) {
    int i, j, k, m, n, donut;
    struct donut_ring *shared_ring;
    struct timeval randtime;
    unsigned short xsub1[3];
    //char *dtype[] = {"plain", "jelly", "chocolate", "glazed"};
    int doz[12][4];     // contains up to 12 donuts of 4 different types
    int d[4];           // keeps count of 4 different types of donuts
    time_t t;
    struct tm *tp;
    float ms;
    int ims;

    // setup shared memory for ring buffer and allocate semaphores
    if ((shmid = shmget(MEMKEY, sizeof(struct donut_ring), 0)) == -1) {
        perror("shared memory get failed: ");
        exit(1);
    }
    if ((shared_ring = (struct donut_ring *)shmat(shmid, NULL, 0)) ==
    (void *)-1) {
        perror("shared memory attach failed: ");
        exit(1);
    }
    for (i = 0; i < NUMSEMIDS; i++) {
        if ((semid[i] = semget(SEMKEY + i, NUMFLAVORS, 0)) == -1) {
            perror("semaphore allocation failed: ");
            exit(1);
        }
    }

    gettimeofday(&randtime, NULL);
    xsub1[0] = (ushort)randtime.tv_usec;
    xsub1[1] = (ushort)(randtime.tv_usec >> 16);
    xsub1[2] = (ushort)(getpid());

    // prepare for 10 runs
    for (i = 0; i < 10; i++) {
        // initialize dozen and flavor arrays to zero
        for (m = 0; m < 12; ++m) {
            for (n = 0; n < 4; ++n) {
                doz[m][n] = 0;
            }
        }
        for (n = 0; n < 4; ++n) { d[n] = 0; }

        // get info for dozen header
        t = time(NULL);
        tp = localtime(&t);
        gettimeofday(&randtime, NULL);

        // get fractional second value
        ms = (float)randtime.tv_usec / 1000000;
        ims = (int)(ms*1000);

        printf("consumer PID %d   time: %d:%d:%d.%d   dozen number: %d\n\n",
        getpid(), tp->tm_hour, tp->tm_min, tp->tm_sec, ims, i + 1);

        // consume a dozen donuts
        for (k = 0; k < 12; k++) {
            j = nrand48(xsub1) & 3;
            //printf("in consumer %s with rand %d\n", argv[1], j);

            if (p(semid[CONSUMER], j) == -1) {      // consumer waits
                perror("p operation failed: ");
                exit(3);
            }
            if (p(semid[OUTPTR], j) == -1) {        // wait for outptr lock
                perror("p operation failed: ");
                exit(3);
            }
            // get donut from ring buffer and increment counter
            donut = shared_ring->flavor[j][shared_ring->outptr[j]];
            shared_ring->outptr[j] = (shared_ring->outptr[j] + 1) % NUMSLOTS;

            if (v(semid[PRODUCER], j) == -1) {      // signal producer
                perror("v operation failed: ");
                exit(3);
            }
            if (v(semid[OUTPTR], j) == -1) {        // release outptr lock
                perror("v operation failed: ");
                exit(3);
            }
            // store donut in doz and increment d counter
            doz[d[j]][j] = donut;
            d[j] = d[j] + 1;
            //printf("donut type %s, \tserial number: %d\n", dtype[j], donut);
        }   // end getting one dozen

        printf("Plain\t\tJelly\t\tChocolate\tGlazed\n");
        for (m = 0; m < 12; ++m) {
            (doz[m][0] == 0) ? printf("\t\t") : printf("%d\t\t", doz[m][0]);
            (doz[m][1] == 0) ? printf("\t\t") : printf("%d\t\t", doz[m][1]);
            (doz[m][2] == 0) ? printf("\t\t") : printf("%d\t\t", doz[m][2]);
            (doz[m][3] == 0) ? printf("\t\t\n") : printf("%d\n", doz[m][3]);
            if ((doz[m][0] == 0) && (doz[m][1] == 0) && (doz[m][2] == 0) &&
            (doz[m][3] == 0)) { break; }
        }
        usleep(100);    // force context-switch with a microsleep
    }   // end getting 10 dozen, now finish

    fprintf(stderr, " CONSUMER %s DONE\n", argv[1]);
    return 0;
}
