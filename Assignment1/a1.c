// Copyright 2021 Matt Zagame
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/wait.h>

// stores process exit status from wait(int *status)
typedef union {
    int exit_status;
    struct {
        unsigned sig_num : 7;
        unsigned core_dmp : 1;
        unsigned exit_num : 8;
    } parts;
} LE_Wait_Status;

int counter = 0, counter_2G = 0;

void sig_handler(int signal);   // signal handler function

int main(void) {
    pid_t pid, ppid;
    int ruid, rgid, euid, egid;
    int priority;
    char msg_buf[20];
    int msg_pipe[2];
    sigset_t mask;
    struct sigaction new_action;
    LE_Wait_Status status;

    printf("\nThis is the Parent process report:\n");
    pid = getpid();         // process ID
    ppid = getppid();       // parent process ID
    ruid = getuid();        // real user ID
    euid = geteuid();       // effective user ID
    rgid = getgid();        // real group ID
    egid = getegid();       // effective group ID
    priority = getpriority(PRIO_PROCESS, 0);

    printf("\nPARENT:  Process ID is:\t\t%ld\n"
    "PARENT:  Process parent ID is:\t%ld\n"
    "PARENT:  Real UID is:\t\t%d\n"
    "PARENT:  Real GID is:\t\t%d\n"
    "PARENT:  Effective UID is:\t%d\n"
    "PARENT:  Effective GID is:\t%d\n"
    "PARENT:  Process priority:\t%d\n",
    (long)pid, (long)ppid, ruid, euid, rgid, egid, priority);

    // create pipe for communication/synchronization between parent and child
    if (pipe(msg_pipe) == -1) {
        perror("\npipe() failed, exiting");
        exit(1);
    }
    sprintf(msg_buf, "\nPipe message\n");

    printf("\nPARENT: will now create child, read pipe, signal child,"
    "\nand wait for obituary from child\n");

    // fork() switch statement
    switch (pid = fork()) {
        case -1:
            perror("\nfork() failed, exiting");
            exit(1);

        case 0:     /***** Child Fork *****/
            // install sig_handler function with sigaction
            sigemptyset(&mask);
            new_action.sa_mask = mask;
            new_action.sa_handler = sig_handler;
            new_action.sa_flags = 0;

            if (sigaction(SIGTERM, &new_action, NULL) == -1) {
                perror("\nsigaction() failed, exiting");
                exit(1);
            }

            printf("\nThis is the Child process report:\n");
            pid = getpid();
            ppid = getppid();
            ruid = getuid();
            euid = geteuid();
            rgid = getgid();
            egid = getegid();
            priority = getpriority(PRIO_PROCESS, 0);

            printf("\nCHILD:  Process ID is:\t\t%ld\n"
            "CHILD:  Process parent ID is:\t%ld\n"
            "CHILD:  Real UID is:\t\t%d\n"
            "CHILD:  Real GID is:\t\t%d\n"
            "CHILD:  Effective UID is:\t%d\n"
            "CHILD:  Effective GID is:\t%d\n"
            "CHILD:  Process priority:\t%d\n",
            (long)pid, (long)ppid, ruid, euid, rgid, egid, priority);

            printf("\nCHILD: about to write pipe and go to endless loop\n");
            fflush(stdout);

            // write to pipe
            if (write(msg_pipe[1], msg_buf, sizeof(msg_buf)) == -1) {
                perror("\nwrite() failed, exiting");
                exit(1);
            }

            // wait for signal from parent
            while (counter_2G < 10) {
                counter++;
                if (counter < 0) {
                    counter = 0;
                    counter_2G++;
                }
            }
            printf("\nCHILD: timed out after 20 billion iterations\n");
            exit(1);

        default:        /***** Parent Fork *****/
            printf("\nPARENT: created child with PID %d\n", pid);

            // read from pipe
            if (read(msg_pipe[0], msg_buf, sizeof(msg_buf)) == -1) {
                perror("\nread() failed, exiting");
                exit(1);
            }
            if (kill(pid, SIGTERM) == -1) {             // send SIGTERM to child
                perror("\nkill() failed, exiting");
                exit(1);
            }

            // parent blocks
            printf("\nPARENT: read pipe and sent SIGTERM, now wait to exit\n");
            if ((pid = wait(&status.exit_status)) == -1) {
                perror("\nwait() failed, exiting");
                exit(1);
            }

            printf("PARENT: child %d exited with exit code %d, goodbye\n",
            pid, status.parts.exit_num);

            return 0;
    }
}

void sig_handler(int signal) {
    printf("\nCHILD: Awake in handler - You got me with Signal Number %d"
    "\nafter spinning for more than %d %s loop iterations\n", signal,
    ((counter_2G) ? counter_2G*2 : counter), ((counter_2G) ? "Billion" : "\0"));
    printf("\nCHILD: now beginning to exec target program, goodbye\n");
    fflush(stdout);
    execl("./Assign1", "Assign1", (char *)NULL);
    perror("\nexecl() failed after receiving termination signal, exiting");
    exit(1);
}
