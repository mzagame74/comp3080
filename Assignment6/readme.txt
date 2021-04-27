Matt Zagame
Matt_Zagame@student.uml.edu
COMP3080
Assignment 6

Degree of success: 100%

This program imitates the ls linux command by printing out status information
about files in a directory. The program can be run in one of two ways, either
without arguments, which will display information about every file in the
current directory, or by specifying which files to print information about as
arguments. The program will then use information about each directory entry in
an lstat call which will populate a stat struct which contains lots of valuable
information about a file. This information is then displayed to the terminal.

The program can be compiled with make and run as such:
./a6
        OR
./a6 file1 file2 ...
