Matt Zagame
Matt_Zagame@student.uml.edu
COMP3080
Assignment 5

Degree of success: 50%

This assignment simulates linked-list memory management using the algorithms
first-fit, best-fit and the buddy system. In my approach I was unable to
complete the buddy system and got most of the first fit and best fit systems to
work. I had some trouble figuring out how to store and manage the linked-lists.
I started by initializing the request array which stores the information
about each request in the given data file, and also initializing the free list.
Then as the data file is read, either an allocation or a free operation will
take place. In a first-fit allocation the free list is searched until the first
block that fits is selected and then reflected on the free list. Best-fit will
create a new free list and match it with the block that leaves the smallest
fragment in memory. A free operation will need to check that its corresponding
allocation request has been made and then it will check for allocated blocks
and update the free list to reflect the free operation. In the case that the
new blocks next boundary address is equal to the next elements address, the two
blocks must be coalesced on the free list.

An example run of the program: ./a5 ff 1 proj5_data

This will simulate all of the requests in the proj5_data data file using a total
memory space of 1MB and the first-fit algorithm.
