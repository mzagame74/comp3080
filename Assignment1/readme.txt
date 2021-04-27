Matt Zagame
Matt_Zagame@student.uml.edu
COMP3080
Assignment 1

Degree of success: 100%

This program demonstrates the use of several system calls in C. First, the
parent process prints out information about itself. A pipe is created for
synchronization between the parent and child processes. Then fork is called
and the child prints out information about itself and enters a long-lasting
loop, waiting for the parent to send a termination signal. The parent uses the
kill system call on the child which forces the child into its signal handler,
executing the Assign1 program. Once this program is running, it enters another
long-lasting loop and prompts the user to enter the kill command on its PID
from the terminal. At the end of the program run, the exit code status of the
child process is displayed.
