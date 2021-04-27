#include "main.h"

int main(int argc, char *argv[]) {
    int seq_num, type_val;
    char *type, *file_name;
    FILE *fp;

    if (argc != 4) {
        perror("usage: a5 policy total_memory_free_space file_name\n");
        exit(1);
    }
    // policies: first fit, best fit, buddy system
    if (strcmp(argv[1], "ff") == 0) { policy = 0; }
    else if (strcmp(argv[1], "bf") == 0) { policy = 1; }
    else if (strcmp(argv[1], "bs") == 0) { policy = 2; }
    else {
        perror("policy must be either ff (first fit), bf (best fit), or bs"
        "(buddy system)\n");
        exit(1);
    }
    // total memory free pool size (1MB or 512KB)
    total_free_space = atoi(argv[2]);
    if (total_free_space == 1) { total_free_space *= 1024; }
    else if (total_free_space != 512) {
        perror("total memory must be either 1 for 1MB or 512 for 512KB\n");
        exit(1);
    }
    total_free = total_free_space;
    file_name = argv[3];    // file containing the allocation and free requests

    // initialize all 1001 elements in array
    for (int i = 0; i < NUMBER_ENTRIES; i++) {
        req_array[i].is_req = FALSE;
        req_array[i].is_allocated = FALSE;
    }

    // setup first free block in free list
    top = malloc(sizeof(struct free_list));
    top->next = NULL;
    top->previous = &list_head;
    top->block_size = total_free_space;
    top->block_adr = 0;
    top->adjacent_adr = total_free_space;

    // second free block
    list_head.next = top;
    list_head.previous = NULL;

    // open the data file and begin to read line by line
    if ((fp = fopen(file_name, "r")) == NULL) {
        perror("cannot open specified file\n");
        exit(1);
    }

    // read data file
    while (fscanf(fp, "%d %s %d", &seq_num, type, &type_val) != EOF) {
        // if this is a request for space
        if (strcmp(type, "alloc") == 0) {
            req_array[seq_num].is_req = TRUE;
            req_array[seq_num].size = type_val;
            if (allocate_memory(&req_array[seq_num]) == -1) {
                perror("allocate_memory failed, exiting");
                exit(1);
            }
            req_array[seq_num].elements_on_free_list = 0;
            req_array[seq_num].largest_chunk = 0;
            for (struct free_list *p = list_head.next; p; p = p->next) {
                ++req_array[seq_num].elements_on_free_list;
                if (p->block_size > req_array[seq_num].largest_chunk) {
                    req_array[seq_num].largest_chunk = p->block_size;
                }
            }
        }   // else this is a free operation
        else {
            req_array[seq_num].size = req_array[type_val].size;
            req_array[seq_num].is_allocated =
                req_array[type_val].is_allocated;
            update_list(type_val);
            req_array[seq_num].memory_left = total_free;
            req_array[seq_num].elements_on_free_list = 0;
            req_array[seq_num].largest_chunk = 0;
            for (struct free_list *p = list_head.next; p; p = p->next) {
                ++req_array[seq_num].elements_on_free_list;
                if (p->block_size > req_array[seq_num].largest_chunk) {
                    req_array[seq_num].largest_chunk = p->block_size;
                }
            }
        }
    }
    fclose(fp);

    // print results
    if (policy == 0) {
        printf("MANAGEMENT POLICY = First Fit\tPOOL SIZE = %d KB\n\n",
    total_free_space);
    }
    else if (policy == 1) {
        printf("MANAGEMENT POLICY = Best Fit\tPOOL SIZE = %d KB\n\n",
    total_free_space);
    }
    else {
        printf("MANAGEMENT POLICY = Buddy System\tPOOL SIZE = %d KB\n\n",
    total_free_space);
    }
    printf("TOTAL ALLOCATIONS: %d\n\n", total_allocs);
    printf("Seq #  Type   Size/Rq  Free Total  Free Elems   Largest"
    "Chunk\n");

    for (int i = 1; i < NUMBER_ENTRIES; i++) {
        if (req_array[i].is_req == 1) {
            printf("%d\talloc\t%d\t %d\t\t%d\t%d\n", i,
            req_array[i].size, req_array[i].memory_left,
            req_array[i].elements_on_free_list, req_array[i].largest_chunk);
        }
        else {
            printf("%d\tfree\t%d\t %d\t\t%d\t%d\n", i,
            req_array[i].size, req_array[i].memory_left,
            req_array[i].elements_on_free_list, req_array[i].largest_chunk);
        }
    }

    return 0;
}

/* allocate_memory processes the request structure by finding a place in the
linked-list where a memory allocation can take place and updates the element in
the request array to point to the new allocation */
int allocate_memory(struct request *req) {

    if (policy == 0) {          // first-fit
        for (struct free_list *p = list_head.next; p; p = p->next) {
            if (req->size <= p->block_size) {
                // memory is available to allocate
                req->is_allocated = TRUE;
                req->base_adr = p->block_adr;
                req->next_boundary_adr = req->base_adr + req->size;

                total_free -= req->size;
                req->memory_left = total_free;

                // check first block that fits
                if ((p->block_size -= req->size) == 0) {
                    p->previous->next = p->next;
                    p->next->previous = p->previous;
                    free(p);
                    return 0;
                }
                p->block_adr += req->size;
                return 0;
            }
            total_allocs++;
        }
        req->memory_left = total_free;
        return 0;
    }

    else if (policy == 1) {     // best fit
        struct free_list *new_list = NULL;
        int mem_left = 0;
        int first = TRUE;   // is this the first element?

        // create a new list to match with the smallest fragment
        for (struct free_list *p = list_head.next; p; p = p->next) {
            if (req->size <= p->block_size) {
                if (p->block_size - req->size <= mem_left || first) {
                    mem_left = p->block_size - req->size;
                    new_list = p;
                }
                first = FALSE;
            }
            total_allocs++;
        }

        if (new_list != NULL) {
            req->is_allocated = TRUE;
            req->base_adr = new_list->block_adr;
            req->next_boundary_adr = req->base_adr + req->size;

            total_free -= req->size;
            req->memory_left = total_free;

            // check for the block that leaves the smallest fragment
            if ((new_list->block_size -= req->size)
            == 0) {
                new_list->previous->next = new_list->next;
                new_list->next->previous = new_list->previous;
                free(new_list);
                return 0;
            }
            new_list->block_adr += req->size;
            return 0;
        }
        req->memory_left = total_free;
        return 0;
    }

    else return -1;             // buddy system
}

/* update_list takes an integer that references an element in the request array,
checks if it was allocated, and updates the list to indicate the element has
been returned */
int update_list(int element) {
    struct free_list *new_list, *coalesce_list;

    if (req_array[element].is_allocated == FALSE) {
        return 0;
    }
    req_array[element].is_allocated = DONE;
    total_free += req_array[element].size;

    // check the free list for allocated blocks
    for (struct free_list *p = list_head.next; p; p = p->next) {
        if (!(req_array[element].base_adr > p->block_adr)) {
            new_list = malloc(sizeof(struct free_list));
            new_list->block_size = req_array[element].size;
            new_list->block_adr = req_array[element].base_adr;
            new_list->adjacent_adr = new_list->block_adr +
                new_list->block_size;

            new_list->next = p;
            p->previous->next = new_list;
            new_list->previous = p->previous;
            p->previous = new_list;

            /* if next boundary address == address of next element on free list
            then coalesce blocks */
            if (new_list->adjacent_adr == new_list->next->block_adr) {
                coalesce_list = new_list->next;
                new_list->block_size = new_list->block_size +
                    new_list->next->block_size;
                new_list->adjacent_adr = new_list->next->adjacent_adr;
                new_list->next = new_list->next->next;

                if (new_list->next) {
                    new_list->next->previous = new_list;
                }

                free(coalesce_list);
            }
        }
    }
    return 0;
}
