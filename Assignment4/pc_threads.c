#define _GNU_SOURCE
#include "donuts.h"

DONUT_SHOP shared_ring;
pthread_mutex_t prod[NUMFLAVORS];
pthread_mutex_t cons[NUMFLAVORS];
pthread_cond_t prod_cond[NUMFLAVORS];
pthread_cond_t cons_cond[NUMFLAVORS];
pthread_t thread_id[NUMPRODUCERS + NUMCONSUMERS];
pthread_t sig_wait_id;

int main(int argc, char *argv[]) {
    int i, j, nsigs;
    struct timeval first_time, last_time;
    struct sigaction new_act;
    int arg_array[NUMPRODUCERS + NUMCONSUMERS];
    sigset_t all_signals;
    int sigs[] = {SIGBUS, SIGSEGV, SIGFPE};

    pthread_attr_t thread_attr;
    //struct sched_param sched_struct;
    int cn;
    float etime;
    char msg[300];

    // get initial timestamp for performance measure
    gettimeofday(&first_time, (struct timezone *)0);
    for (i = 0; i < NUMPRODUCERS + NUMCONSUMERS; i++) {
        arg_array[i] = i + 1;   /* cons[0] has ID = 1 */
    }

    // pthread mutex and condition init and global init
    for (i = 0; i < NUMFLAVORS; i++) {
        pthread_mutex_init(&prod[i], NULL);
        pthread_mutex_init(&cons[i], NULL);
        pthread_cond_init(&prod_cond[i], NULL);
        pthread_cond_init(&cons_cond[i], NULL);
        shared_ring.outptr[i] = 0;
        shared_ring.in_ptr[i] = 0;
        shared_ring.serial[i] = 1;
        shared_ring.spaces[i] = NUMSLOTS;
        shared_ring.donuts[i] = 0;
    }

    // install signal handler
    sigfillset(&all_signals);
    nsigs = sizeof(sigs) / sizeof(int);
    for (i = 0; i < nsigs; i++) {
        sigdelset(&all_signals, sigs[i]);
    }
    sigprocmask(SIG_BLOCK, &all_signals, NULL);
    sigfillset(&all_signals);
    for (i = 0; i < nsigs; i++) {
        new_act.sa_handler = sig_handler;
        new_act.sa_mask = all_signals;
        new_act.sa_flags = 0;
        if (sigaction (sigs[i], &new_act, NULL) == -1) {
            perror("can't set signals: ");
            exit(1);
        }
    }
    //printf("just before threads created\n");

    // create signal handler thread, producer and consumer threads
    if (pthread_create(&sig_wait_id, NULL, sig_waiter, NULL) != 0) {
        printf("pthread_create failed ");
        exit(3);
    }

    pthread_attr_init(&thread_attr);
    pthread_attr_setinheritsched(&thread_attr, PTHREAD_INHERIT_SCHED);

#ifdef GLOBAL
    pthread_attr_setinheritsched(&thread_attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&thread_attr, SCHED_OTHER);
    sched_struct.sched_priority = sched_get_priority_max(SCHED_OTHER);
    pthread_attr_setschedparam(&thread_attr, &sched_struct);
    pthread_attr_setscope(&thread_attr, PTHREAD_SCOPE_SYSTEM);
#endif

    for (i = 0; i < NUMCONSUMERS; i++, j++) {
        if (pthread_create(&thread_id[i], &thread_attr, consumer,
        (void *)&arg_array[i]) != 0) {
            printf("pthread_create failed ");
            exit(3);
        }
    }

    for (; i < NUMPRODUCERS + NUMCONSUMERS; i++) {
        if (pthread_create(&thread_id[i], &thread_attr, producer, NULL) != 0) {
            printf("pthread_create failed ");
            exit(3);
        }
    }
    //printf("just after threads created\n");

    // wait for all consumers to finish
    write(1, "\n\n", 2);
    for (i = 0; i < NUMCONSUMERS; i++) {
        pthread_join(thread_id[i], NULL);
        write(1, "*", 1);
    }

    // get final timestamp and calculate elapsed time
    gettimeofday(&last_time, (struct timezone *)0);
    if ((i = last_time.tv_sec - first_time.tv_sec) == 0) {
        j = last_time.tv_usec - first_time.tv_usec;
    } else {
        if (last_time.tv_usec - first_time.tv_usec < 0) {
            i--;
            j = 1000000 + (last_time.tv_usec - first_time.tv_usec);
        } else {
            j = last_time.tv_usec - first_time.tv_usec;
        }
    }
    printf("\n\nElapsed consumer time is %d sec and %d usec, or %f sec\n", i, j,
    (etime = i + (float)j/1000000));
    if ((cn = open("./run_times", O_WRONLY|O_CREAT|O_APPEND, 0644)) == -1) {
        perror("can't open sys time file ");
        exit(1);
    }
    sprintf(msg, "%f\n", etime);
    write(cn, msg, strlen(msg));

    return 0;
}

/********** producer function **********/
void *producer(void *arg) {
    int i, j;
    unsigned short xsub[3];
    struct timeval randtime;

    gettimeofday(&randtime, (struct timezone *)0);
    xsub[0] = (ushort) randtime.tv_usec;
    xsub[1] = (ushort) (randtime.tv_usec >> 16);
    xsub[2] = (ushort) (pthread_self());

    // producer forever loop
    while(1) {
        // produce 60 donuts
        for (i = 0; i < 60; ++i) {
            j = nrand48(xsub) & 3;
            pthread_mutex_lock(&prod[j]); // get mutex lock for flavor j
            while (shared_ring.spaces[j] == 0)
            {
                pthread_cond_wait(&prod_cond[j], &prod[j]); // wait if no space
            }
            // enter critical section
            shared_ring.flavor[j][shared_ring.in_ptr[j]] =shared_ring.serial[j];
            shared_ring.spaces[j]--;
            shared_ring.serial[j]++;
            shared_ring.in_ptr[j] = (shared_ring.in_ptr[j] + 1) % NUMSLOTS;
            pthread_mutex_unlock(&prod[j]); // release mutex lock

            pthread_mutex_lock(&cons[j]);
            shared_ring.donuts[j]++; // increment donut count
            pthread_mutex_unlock(&cons[j]);

            pthread_cond_signal(&cons_cond[j]);
        }
        pthread_yield();
    }
    return NULL;
}

/********** consumer function **********/
void *consumer(void *arg) {
    int i, j, donut;
    char lbuf[5];
    unsigned short xsub[3];
    struct timeval randtime;
    char file_name[10] = "cons";
    FILE* cn;
    int doz[12][4];
    int d[4], m, n;
    time_t t;
    struct tm * tp;
    float ms;
    int ims;
    int id = *(int *)arg;

    gettimeofday(&randtime, (struct timezone *)0);
    xsub[0] = (ushort) randtime.tv_usec;
    xsub[1] = (ushort) (randtime.tv_usec >> 16);
    xsub[2] = (ushort) (id);

    if ((id % 10) == 0) {
        sprintf(file_name, "cons%d", id);
        if ((cn = fopen(file_name, "w+")) == NULL) {
            perror("failed to open consumer output file ");
        }
    }
    sprintf(lbuf, " %d ", id);

    // consumer number of dozens loop
    for (i = 0; i < NUMDOZENS; i++) {
        // initialize dozen and flavor arrays to zero
        for (m = 0; m < 12; ++m) {
            for (n = 0; n < 4; ++n) { doz[m][n] = 0; }
        }
        for (n = 0; n < 4; ++n) { d[n] = 0; }

        // get info for dozen header
        t = time(NULL);
        tp = localtime(&t);
        gettimeofday(&randtime, NULL);

        // get fractional second value
        ms = (float)randtime.tv_usec / 1000000;
        ims = (int)(ms*1000);

        if ((i < 10) && ((id % 10) == 0)) {
            fprintf(cn, "consumer thread #%d   time: %d:%d:%d.%d   dozen #%d"
            "\n\n", id, tp->tm_hour, tp->tm_min, tp->tm_sec, ims, i + 1);
        }

        // consume a dozen donuts
        for (m = 0; m < 12; m++) {
            j = nrand48(xsub) & 3;
            pthread_mutex_lock(&cons[j]);   // get mutex lock for flavor j
            while (shared_ring.donuts[j] == 0) {
                pthread_cond_wait(&cons_cond[j], &cons[j]); // wait if no donuts
            }
            // enter critical section
            donut = shared_ring.flavor[j][shared_ring.outptr[j]];
            shared_ring.donuts[j]--;
            shared_ring.outptr[j] = (shared_ring.outptr[j] + 1) % NUMSLOTS;
            pthread_mutex_unlock(&cons[j]);  // release mutex lock

            pthread_mutex_lock(&prod[j]);
            shared_ring.spaces[j]++;        // increment space count
            pthread_mutex_unlock(&prod[j]);

            pthread_cond_signal(&prod_cond[j]);

            doz[d[j]][j] = donut;
            d[j] = d[j] + 1;
        }

        // print the first ten dozen donuts for every other 10 consumer threads
        if ((i < 10) && ((id % 10) == 0)) {
            fprintf(cn, "plain\t\tjelly\t\tcoconut\t\thoney-dip\n");
            for (m = 0; m < 12; ++m) {
                (doz[m][0] == 0) ? fprintf(cn, "\t\t") :
                    fprintf(cn, "%d\t\t", doz[m][0]);
                (doz[m][1] == 0) ? fprintf(cn, "\t\t") :
                    fprintf(cn, "%d\t\t", doz[m][1]);
                (doz[m][2] == 0) ? fprintf(cn, "\t\t") :
                    fprintf(cn, "%d\t\t", doz[m][2]);
                (doz[m][3] == 0) ? fprintf(cn, "\t\t\n") :
                    fprintf(cn, "%d\n", doz[m][3]);
                if ((doz[m][0] == 0) && (doz[m][1] == 0) && (doz[m][2] == 0) &&
                    (doz[m][3] == 0)) { break; }
            }
        }
        usleep(100);
    }   // end getting 10 dozen

    write(1, lbuf, 4);
    return NULL;
}

/********** pthread asynchronous signal handler **********/
void *sig_waiter(void *arg) {
    sigset_t sigterm_signal;
    int signo;

    sigemptyset(&sigterm_signal);
    sigaddset(&sigterm_signal, SIGTERM);
    sigaddset(&sigterm_signal, SIGINT);

    if (sigwait(&sigterm_signal, &signo) != 0) {
        printf("\nsigwait failed, exiting\n");
        exit(2);
    }
    printf("Process exits on SIGNAL %d\n\n", signo);
    exit(1);
    return NULL;    // unreachable
}

/********** signal handler **********/
void sig_handler(int sig) {
    pthread_t signaled_thread_id;
    int i, thread_index;

    signaled_thread_id = pthread_self();

    // check for own id in an array of thread ids
    for (i = 0; i < NUMCONSUMERS; i++) {
        if (signaled_thread_id == thread_id[i]) {
            thread_index = i + 1;
            break;
        }
    }
    printf("\nThread %d took signal #%d, PROCESS HALT\n", thread_index, sig);
    exit(1);
}
