// Copyright 2021 Matt Zagame
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>

void sig_handler(int signal);

int main(int argc, char *argv[]) {
    int in_pipe[2], out_pipe[2], fd, n;
    char buf[81];
    pid_t pid;
    struct sigaction new_sig_state;
    sigset_t mask;
    FILE *in_pipe_read_end;
    char last_name[32], first_name[32];
    int area_code, old_code, prefix, telnum;
    int acode_count = 0, total_count = 0;

    // install signal handler
    sigemptyset(&mask);
    new_sig_state.sa_mask = mask;
    new_sig_state.sa_handler = sig_handler;
    new_sig_state.sa_flags = 0;

    if (sigaction(SIGPIPE, &new_sig_state, NULL) == -1) {
        perror("sigaction() failed, exiting");
        exit(1);
    }

    // create two unnamed pipes
    if (pipe(in_pipe) == -1 || pipe(out_pipe) == -1) {
        perror("pipe() failed, exiting");
        exit(1);
    }

    // fork switch statement
    switch (pid = fork()) {
        case -1:
            perror("fork() failed, exiting");
            exit(1);

        /************* child process *************/
        case 0:
            // close stdin and stdout for duping
            if (close(0) == -1 || close(1) == -1) {
                perror("close() failed, exiting");
                exit(1);
            }
            /* dup read channel from parent's input pipe and dup write channel
            from parent's output pipe */
            if (dup(out_pipe[0]) != 0 || dup(in_pipe[1]) != 1) {
                perror("dup() failed, exiting");
                exit(1);
            }

            // close all inherited pipe channels
            if (close(in_pipe[0]) == -1 || close(in_pipe[1]) == -1 ||
            close(out_pipe[0]) == -1 || close(out_pipe[1]) == -1) {
                perror("close() failed, exiting");
                exit(1);
            }

            // run sort by column 3, then 1, then 2
            if (execlp("sort", "sort", "-k3.3n", "-k1.1", "-k2.2", NULL)
            == -1) {
                perror("execl() failed, exiting");
                exit(1);
            }

        /************* parent process *************/
        default:
            // open file to sort
            if ((fd = open("./cs308a2_sort_data", O_RDONLY)) == -1) {
                perror("open() failed, exiting");
                kill(pid, SIGTERM);
                exit(1);
            }
            // read from file and write contents to child
            while ((n = read(fd, buf, sizeof(buf) - 1)) > 0) {
                buf[n] = '\0';
                if (write(out_pipe[1], buf, strlen(buf)) == -1) {
                    perror("write() failed, exiting");
                    exit(1);
                }
                //printf("*");      // uncomment this line to see data transfer
            }
            printf("\nALL DATA SENT\n");

            // close unused channels
            if (close(in_pipe[1]) == -1 || close(out_pipe[0]) == -1 ||
            close(out_pipe[1]) == -1 || close(fd)) {
                perror("close() failed, exiting");
                exit(1);
            }

            // read child's sort output and write to stdout
            in_pipe_read_end = fdopen(in_pipe[0], "r");

            fscanf(in_pipe_read_end, "%s %s %d %d %d\n", last_name, first_name,
            &area_code, &prefix, &telnum);
            printf("\nFIRST LINE: %s %s %d %d %d\n\n", last_name, first_name,
            area_code, prefix, telnum);
            acode_count++;
            old_code = area_code;

            // scan and count total number of each area code
            while (fscanf(in_pipe_read_end, "%s %s %d %d %d\n", last_name,
            first_name, &area_code, &prefix, &telnum) != EOF) {
                if (area_code != old_code) {
                    printf("area code %d occurs %d times\n", old_code,
                    acode_count);
                    total_count += acode_count;
                    acode_count = 1;
                    old_code = area_code;
                } else {
                    acode_count++;
                }
            }
            total_count += acode_count;
            printf("area code %d occurs %d times\n", old_code, acode_count);

            if (close(in_pipe[0]) == -1) {
                perror("close() failed, exiting");
                exit(1);
            }

            printf("\n%d RECORDS PROCESSED ... goodbye\n", total_count);
            wait(NULL);
    }   // end switch

    return 0;
}

void sig_handler(int signal) {
    perror("\nSIGPIPE received, exiting\n");
    exit(1);
}
