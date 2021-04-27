// The following data structures are suggested for the first fit
// and best fit implementations for assignment 5
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define NUMBER_ENTRIES 1001
#define FALSE 0
#define TRUE 1
#define DONE 2

// data structures for first-fit and best-fit policies
struct request {
    int is_req;
    int is_allocated;
    int size;
    int match_alloc;        // which allocation number is being freed
    int base_adr;           // base address of allocation
    int next_boundary_adr;
    int memory_left;
    int largest_chunk;
    int elements_on_free_list;
} req_array[NUMBER_ENTRIES];

struct free_list {
    struct free_list *next;
    struct free_list *previous;
    int block_size;
    int block_adr;
    int adjacent_adr;
} list_head, *top;

// data structures for buddy system policy
struct bud_request {
    int is_req;
    int is_allocated;
    int size;
    int act_size;
    int base_adr;
    int next_boundary_adr;
    int memory_left;
    int largest_chunk;
    int elements_on_free_list;
    struct lel *this_req;
} bud_req_array[NUMBER_ENTRIES];

// a block list element on one of the block sized list of addresses
struct lel {
    struct lel *next;
    struct lel *previous;
    int adr;
    int bud_adr;
    int bit;
};

/* a list head for one of the block sized lists that hold struct lel elements
(need one for each in-range power of 2) */
struct lh {
    struct lel *head;
    int cnt;
};

// some globals and function declarations
int total_free_space, policy;
int free_list_length = 0, total_free, total_allocs = 0;
struct lh lst_ary[21];      // array of lists for byte allocations (2^5 - 2^20)
int mal_cnt = 0;            // memory array list count

int allocate_memory(struct request *);
int update_list(int);
struct lel *find_el(int bit);
int update_lists(struct lel *el);
