Matt Zagame
Matt_Zagame@student.uml.edu
COMP3080
Assignment 2

Degree of success: 100%

sort_ck and grep_ck are two similar programs that fork a child process to run
the sort and grep linux programs respectively. sort_ck runs without any
arguments and grep_ck requires a specified file to run grep on. Each program
makes use of two unnamed pipes and the dup() system call to send data between
the parent and child process. The parent is responsible for reading information
from the given data files and sending it to the child process. Meanwhile, the
child will dup() the inherited pipe channels onto channels 0 and 1 and exec the
appropriate linux program. After the program runs, it will send the data back
to the parent and the parent will output the results of the data.

When running grep_ck with cs308a2_grep_data_1 the result is 28 matches found and
reported properly by the parent process. However, when running grep_ck with the
much larger file cs308a2_grep_data_2, only so much data can be processed through
the pipe before it fills up entirely and deadlock occurs. output.txt shows how
much data is sent to the child before deadlock occurs indicated by a '*' per 80
bytes sent.
