Matt Zagame
Matt_Zagame@student.uml.edu
COMP3080
Assignment 3

Degree of success: 100%

This assignment demonstrates thread synchronization by solving the producer/
consumer problem. Using semaphores, the producer process creates donuts (random
integers) and inserts them into the appropriate ring buffer slot when allowed
in its critical section. When it is safe for a consumer process to enter its
critical section, it extracts a donut from the ring buffer and decrements the
outptr counter with an additional binary semaphore lock.

By modifying the queue size of the ring buffer, it was possible to analyze how
often a single producer and several consumers processes would wind up in a
deadlock. The two graphs included in this directory show the probability of
deadlock when changing the queue size used by one producer and five consumers,
and the probability of deadlock when using a queue size of 40 slots and a
varying number of consumers. Each probability point (y variable) on both graphs
is the result of running 20 tests for each x variable using the provided shell
script.

For the graph of deadlock probability vs queue size, a downward trend shows that
the more queue slots are available, the less chance of deadlock. For the graph
of deadlock probability vs number of consumers using a fixed queue size of 40,
an upwards trend shows that more consumers means a higher chance of deadlock.
