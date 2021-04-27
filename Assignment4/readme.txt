Matt Zagame
Matt_Zagame@student.uml.edu
COMP3080
Assignment 4

Degree of success: 100%

This assignment demonstrates thread synchronization within a single process by
using the pthread library. The producer and consumer threads both enter into
their critical sections using pthread mutex locks to modify data in the donut
shop data structure when they are safe to do so. Both producers and consumers
have condition variables that must be satisfied before they can modify data in
their critical sections. Once either a producer or consumer is done, it will
signal the other that the conditions are now acceptable.

By modifying the queue size of the ring buffer, it was possible to analyze how
often the 30 producers and 50 consumers would wind up in a deadlock. The two
graphs included in this directory show the probability of deadlock versus a
queue size ranging between 1000-3000, and the probability of deadlock when
using a fixed queue size of 1900 slots versus the number of dozens of donuts
to be consumed between 1000-3000.

The results of this assignment were very similar to those of assignment 3.
The graph of deadlock probability vs queue size shows a downward trend where
more queue slots equals less of a chance of deadlock. Since the ring buffers are
larger for each donut flavor, there is less chance of one filling up and having
a producer and consumer stuck waiting on each other. The graph of deadlock
probability using a fixed queue size of 1900 vs number of dozens of donuts has
an upwards trend which shows that the more dozens of donuts consumed, the
higher the chance of deadlock. Since more donuts are consumed per consumer
thread, there is a higher chance of getting stuck waiting if no donuts are
produced.
