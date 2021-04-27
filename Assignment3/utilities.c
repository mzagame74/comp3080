/* utilities.c: semsetall(), p(), and v() functions  */
/*              for shared memory / semaphore donuts */

#include "donuts.h"

int p(int semidgroup, int donut_type) {
    struct sembuf semopbuf;

    semopbuf.sem_num = donut_type;
    semopbuf.sem_op = (-1);     // -1 is a p operation
    semopbuf.sem_flg = 0;

    if (semop(semidgroup, &semopbuf, 1) == -1) {
        return -1;
    }
    return 0;
}

int v(int semidgroup, int donut_type) {
    struct sembuf semopbuf;

    semopbuf.sem_num = donut_type;
    semopbuf.sem_op = (+1);     // +1 is a v operation
    semopbuf.sem_flg = 0;

    if (semop(semidgroup, &semopbuf, 1) == -1) {
        return -1;
    }
    return 0;
}

int semsetall(int semgroup, int number_in_group, int set_all_value) {
    union semun {
        int val;                    /* value for SETVAL */
        struct semi_ds *buf;        /* buffer for IPC_STAT, IPC_SET */
        unsigned short int *array;  /* array for GETALL, SETALL */
        struct seminfo *__buf;      /* buffer for IPC_INFO */
    } sem_ctl_un;

    sem_ctl_un.val = set_all_value;

    for (int i = 0; i < number_in_group; i++) {
        if (semctl(semgroup, i, SETVAL, sem_ctl_un) == -1) {
            return -1;
        }
    }
    return 0;
}
