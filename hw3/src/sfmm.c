/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "sfmm.h"
#include <errno.h>

int wild_check = 0; /* Check to see if find_block() returned wilderness block or not */
// sf_block *prologue;
// sf_block *wilderness;
// sf_block *epilogue;
int index_block_used;
/*
    Initializes the heap with Prologue, wilderness block, and epilogue
*/

void initialize_heap(void* start)  {
    sf_block* prologue = start + 48;
    // prologue = start + 48;
    prologue -> header = 64;
    prologue -> header |= THIS_BLOCK_ALLOCATED;
    prologue -> header |= PREV_BLOCK_ALLOCATED;

    sf_block* wilderness;
    wilderness = prologue + 2;
    wilderness -> prev_footer = prologue -> header;
    wilderness -> header = 3968 | 2;

    sf_block* epilogue = sf_mem_end()-16;
    // epilogue = wilderness + 124;
    epilogue -> prev_footer = wilderness -> header;
    epilogue -> header = 1;

    for (int i = 0; i < NUM_FREE_LISTS; i++){
        if (i == NUM_FREE_LISTS-1){
            sf_free_list_heads[i].body.links.next = wilderness;
            wilderness->body.links.next = (sf_free_list_heads + NUM_FREE_LISTS-1);
            wilderness->body.links.prev = (sf_free_list_heads + NUM_FREE_LISTS-1);
            break;
        }
        sf_free_list_heads[i].body.links.next = &sf_free_list_heads[i];
        sf_free_list_heads[i].body.links.prev = &sf_free_list_heads[i];
    }
}

/*
    @params: checks size of block to be alloc
    returns: 0: need more memory
             1: don't need more memory
*/


int check_more_mem(int size_of_block){
    // Start after prologue to see if we have blocks that will fit the requested malloc
    sf_block* prologue = sf_mem_start() + 48;
    sf_block* epilogue = sf_mem_end()-16;

    sf_block *where = prologue + 2;

    while (where < epilogue){
        // printf("Where: %p, epilogue: %p\n", where,epilogue);
        if (((where->header) & THIS_BLOCK_ALLOCATED) == THIS_BLOCK_ALLOCATED){
            // printf("current header: %ld\n",where->header);
            where += ((where->header) & BLOCK_SIZE_MASK)/32;
        } else {
            if (size_of_block > (where->header & BLOCK_SIZE_MASK)){
                where += ((where->header) & BLOCK_SIZE_MASK)/32;
            } else if (size_of_block <= (where->header & BLOCK_SIZE_MASK)){
                return 1;
            }
        }

    }
    return 0;
}


/*
    @ param: Multiple of 64 M to calculate the index in the fib sequence
    returns: index in the fib sequence
*/
int find_index(int m){
    int value1 = 3;
    int value2 = 5;

    // printf("value1: %d, value2: %d, index: %d, m: %d\n", value1,value2,index,m);
    if (m == 1){
        return 0;
    } else if (m == 2){
        return 1;
    } else if (m == 3){
        return 2;
    } else {
    // for (int i = 0; i < NUM_FREE_LISTS; i++){

        for (int i = 3; i < NUM_FREE_LISTS-1; i++){
            // printf("value1: %d, value2: %d, i: %d\n",value1, value2, i);
            if (m > value1 && m <= value2){
                return i;
            } else if (i == NUM_FREE_LISTS-2){
                return i;
            }

            else {
                int temp = value1+value2;
                value1 = value2;
                value2 = temp;
            }
        }
    }
    return -1;

}

/*
    @ param, starting index for sf_free_list_heads, corresponding sentinel
    returns: free block to use in malloc
*/

sf_block *find_block(int startIndex, sf_block* sentinel, int size){
    for (int i = startIndex; i < NUM_FREE_LISTS-1; i++){
        sentinel = &sf_free_list_heads[i];
        // printf("sentinel-> next, %p, sentinel: %p\n", sentinel->body.links.next, sentinel);

        if ((sentinel->body.links.next != sentinel) && ((sentinel->body.links.next)->header & BLOCK_SIZE_MASK) >= size){
            wild_check = 0;
            index_block_used = i;
            return sentinel->body.links.next;
        }
    }
    wild_check = 1;
    index_block_used = NUM_FREE_LISTS-1;
    return sf_free_list_heads[NUM_FREE_LISTS-1].body.links.next;
}
/*
    @ param, block to check adjackent blocks
    returns: 0,1,2,3
    0: both prev and next are free
    1: prev is free, next is alloc
    2: prev is alloc, next is free
    3: both prev and next are alloc
    -1: error
*/
int check_adjacent(sf_block* block){
    // int test = block->header;
    // printf("test: %d\n", test);
    int size = block->header & BLOCK_SIZE_MASK;
    // printf("size: %d\n", size);


    // PROLOGUE EDGE CASE
    if (block-2 == (sf_mem_start()+48)){
        // printf("test\n");
        // printf("next block header: %lo\n",(block+size/32)->header);
        if (((block+size/32)->header & THIS_BLOCK_ALLOCATED) == 0){
            return 2;

        // printf("test4\n");
        // CASE WHERE PREV AND NEXT ARE ALLOCATED
        } else if (((block+size/32)->header & THIS_BLOCK_ALLOCATED) == THIS_BLOCK_ALLOCATED){
            // printf("this should print\n");
            return 3;
        }
    }

    // EPILOGUE EDGE CASE
    if (block+size/32 == (sf_mem_end()-16)){
        // printf("test\n");
        if ((block->prev_footer & THIS_BLOCK_ALLOCATED) == 0)
            return 1;
        else if ((block->prev_footer & THIS_BLOCK_ALLOCATED) == THIS_BLOCK_ALLOCATED)
            return 3;
    }

    // int test = block->header & PREV_BLOCK_ALLOCATED
    // CASE WHERE PREV BLOCK FREE
    if (((block->header) & PREV_BLOCK_ALLOCATED) == 0){
        // CHECK IF NEXT BLOCK FREE
        // printf("test1\n");
        // CASE WHERE PREV AND NEXT ARE BOTH FREE, COALESCE 3 ADJACENT BLOCKS
        if (((block+size/32)->header & THIS_BLOCK_ALLOCATED) == 0){
            return 0;

        // printf("test2\n");

        // CASE WHERE NEXT IS ALLOCATED, PREV IS FREE
        } else if (((block+size/32)->header & THIS_BLOCK_ALLOCATED) == THIS_BLOCK_ALLOCATED){
            return 1;

        // printf("test3\n");
        }

    // CASE WHERE PREV BLOCK ALLOCATED
    } else if (((block->header) & PREV_BLOCK_ALLOCATED) == PREV_BLOCK_ALLOCATED){
        // CHECK IF NEXT BLOCK FREE
        // printf("test6\n");
        // CASE WHERE PREV IS ALLOCATED, NEXT IS FREE
        if (((block+size/32)->header & THIS_BLOCK_ALLOCATED) == 0){
            return 2;

        // printf("test4\n");
        // CASE WHERE PREV AND NEXT ARE ALLOCATED
        } else if (((block+size/32)->header & THIS_BLOCK_ALLOCATED) == THIS_BLOCK_ALLOCATED){
            return 3;

        // printf("test5\n");
        }

    }
    // SHOULD NEVER HAPPEN
    return -1;
}

/*
    Adds block into free_list at appropiate index
*/
void add_block_freelist(int index, sf_block *coalesce_block){

    //test
    // printf("index: %d, header: %lo\n", index, coalesce_block->header);
    //test//

    if (sf_free_list_heads[index].body.links.next == &sf_free_list_heads[index]){
            sf_free_list_heads[index].body.links.next = coalesce_block;
            coalesce_block->body.links.prev = &sf_free_list_heads[index];
            coalesce_block->body.links.next = &sf_free_list_heads[index];
        } else {
            sf_block* next_free_block = sf_free_list_heads[index].body.links.next;
            sf_free_list_heads[index].body.links.next = coalesce_block;
            coalesce_block->body.links.next = next_free_block;
            coalesce_block->body.links.prev = &sf_free_list_heads[index];
            next_free_block->body.links.prev = coalesce_block;
        }
}


/*
    Removes block from free_lista
*/
void remove_block_freelist(sf_block *coalesce_block){
    coalesce_block->body.links.prev->body.links.next = coalesce_block->body.links.next;
    coalesce_block->body.links.next->body.links.prev = coalesce_block->body.links.prev;
}

void *sf_malloc(size_t size) {

    // printf("size: %zu\n",size);
    if (size <= 0){
        return NULL;
    }
    if (sf_mem_start() == sf_mem_end()){
        sf_mem_grow();
        // void* start = sf_mem_start();
        initialize_heap(sf_mem_start());
    }

    // FIND SIZE OF BLOCK TO BE ALLOCATED //

    size_t asize; /* Adjusted size */
    int m; /* Multiple of 64 */
    // size_t padding; /* Padding to make multiple of 64 */
    // size_t extendsize; /* Amount to extend heap? */

    asize =  (size + sizeof(sf_header) + 63) & (~63); /* Header size */
    m = asize/64;
    // if (asize <= 64) {
    //     // padding = 64 - asize;type
    //     asize = 64;
    //     m = 1;
    // } else if (asize > 64){
    //     int temp = asize / 64;
    //     // printf("a %f\n",temp);
    //     asize = 64* (temp + 1);
    //     m = temp + 1;
    // }

    // printf("asize: %zu\n",asize);

    while(check_more_mem(asize) == 0){
        sf_block *old_epilogue = sf_mem_end()-16;
        sf_block *new_end = sf_mem_grow();
        sf_block *epilogue = sf_mem_end()-16;

        sf_block *wilderness = epilogue - (((old_epilogue->prev_footer) & BLOCK_SIZE_MASK)+PAGE_SZ)/32;

        if (new_end == NULL){
            sf_errno = ENOMEM;
            return NULL;
        };

        wilderness->header += PAGE_SZ;
        epilogue->prev_footer = wilderness->header;
    }

    int startIndex = find_index(m); /* Starting index of list head to look */

    sf_block* block_to_be_allocated = find_block(startIndex, &sf_free_list_heads[startIndex], asize);

    // printf("Wilderness test: %p\n", block_to_be_allocated);

    size_t check_alloc_block; /*  Mask alloc block and see if it's larger or equal to size of data to be allocated */
    check_alloc_block = block_to_be_allocated->header & BLOCK_SIZE_MASK;

    // size_t test = block_to_be_allocated->header;
    // printf("test: %zu\n", test);
    // printf("size of check_alloc_block: %zu\n", check_alloc_block);
    // What's left of the block after the asize is used
    check_alloc_block = check_alloc_block - asize; /* subtract asize from block to see if it's proper size */
    // printf("size of check_alloc_block: %zu, asize: %zu\n", check_alloc_block,asize);

    if (check_alloc_block == 0){ /* Perfect Size */
        // printf("test\n");
        // block_to_be_allocated->header
        if (index_block_used == NUM_FREE_LISTS-1){
            sf_block* epilogue = sf_mem_end()-16;
            // WILDERNESS ALLOCATED
            int prev_aloc = block_to_be_allocated->header & PREV_BLOCK_ALLOCATED;
            if (prev_aloc == PREV_BLOCK_ALLOCATED){
                // printf("wilderness header, %zu\n", wilderness->header);
                block_to_be_allocated->header = block_to_be_allocated->header | PREV_BLOCK_ALLOCATED;
            }
            block_to_be_allocated->header = block_to_be_allocated->header | THIS_BLOCK_ALLOCATED;
            epilogue->prev_footer = block_to_be_allocated->header;
            sf_free_list_heads[NUM_FREE_LISTS-1].body.links.next = &sf_free_list_heads[NUM_FREE_LISTS-1];
            sf_free_list_heads[NUM_FREE_LISTS-1].body.links.prev = &sf_free_list_heads[NUM_FREE_LISTS-1];
            index_block_used = 0;
            return block_to_be_allocated->body.payload;
        } else {
            // if ((block_to_be_allocated->header & PREV_BLOCK_ALLOCATED) == PREV_BLOCK_ALLOCATED){
            //     block_to_be_allocated->header = block_to_be_allocated->header | PREV_BLOCK_ALLOCATED;
            // }
            block_to_be_allocated->header = block_to_be_allocated->header | THIS_BLOCK_ALLOCATED;
            (block_to_be_allocated+(block_to_be_allocated->header & BLOCK_SIZE_MASK)/32)->prev_footer = block_to_be_allocated->header;
            remove_block_freelist(block_to_be_allocated);
            sf_block* next_block = block_to_be_allocated + ((block_to_be_allocated->header)&BLOCK_SIZE_MASK)/32;
            next_block->header = next_block->header | PREV_BLOCK_ALLOCATED;
            index_block_used = 0;

            return block_to_be_allocated->body.payload;
        }
    } else {
        sf_block *epilogue = sf_mem_end()-16;
        sf_block* wilderness = epilogue - (epilogue->prev_footer & BLOCK_SIZE_MASK)/32;
        if (wild_check == 1){
            /* Set the header size of the space to be allocated first to asize */
                // printf("wilderness header, %d\n", (int)wilderness->header);
            int prev_aloc = (wilderness->header) & PREV_BLOCK_ALLOCATED;
            // printf("test: %d\n", test);
            block_to_be_allocated->header = asize | THIS_BLOCK_ALLOCATED;

            if (prev_aloc == 2){
                // printf("wilderness header, %zu\n", wilderness->header);
                block_to_be_allocated->header = block_to_be_allocated->header | PREV_BLOCK_ALLOCATED;
            }
            // printf("test epilogue, wilderness: %p, %p\n\n",epilogue, wilderness);

            wilderness = block_to_be_allocated + (asize/32);

            // printf("test epilogue, wilderness: %p, %p\n\n",epilogue, wilderness);


            // printf("test epilogue, wilderness: %p, %p\n\n",epilogue, wilderness);

            // wilderness->header = check_alloc_block | THIS_BLOCK_ALLOCATED;
            wilderness->header = check_alloc_block | PREV_BLOCK_ALLOCATED;

            int ep_help = check_alloc_block & BLOCK_SIZE_MASK;

            epilogue = wilderness + (ep_help/32); /* Setting new prologue */

            // sf_block* test = sf_mem_end()-16;

            // debug("epilogue test: curr: %p, test: %p\n", epilogue, test);
            // printf("wilderness header: %zu\n", wilderness->header);
            wilderness->prev_footer = block_to_be_allocated->header;
            sf_free_list_heads[NUM_FREE_LISTS-1].body.links.next = wilderness;
            wilderness->body.links.next = (sf_free_list_heads + NUM_FREE_LISTS-1);
            wilderness->body.links.prev = (sf_free_list_heads + NUM_FREE_LISTS-1);
            epilogue->prev_footer = wilderness->header;
            wild_check = 0;
            index_block_used = 0;

            return block_to_be_allocated->body.payload;

        } else if (wild_check == 0){

            // CHANGE PREV_FOOTER OF BLOCK AFTER BLOCK_TO_BE_ALLOCATED
            sf_block* next_block = block_to_be_allocated + (block_to_be_allocated->header & BLOCK_SIZE_MASK) / 32;
            // next_block->prev_footer
            block_to_be_allocated->header = asize | THIS_BLOCK_ALLOCATED;
            if ((block_to_be_allocated->prev_footer) & THIS_BLOCK_ALLOCATED){
                block_to_be_allocated->header = block_to_be_allocated->header | PREV_BLOCK_ALLOCATED;
            }
            sf_block* new_free_block = block_to_be_allocated+asize/32;

            new_free_block->prev_footer = block_to_be_allocated->header;
            new_free_block->header = check_alloc_block | PREV_BLOCK_ALLOCATED;
            next_block->prev_footer = new_free_block->header;
            int index_to_store = find_index(check_alloc_block/64);
            add_block_freelist(index_to_store, new_free_block);
            remove_block_freelist(block_to_be_allocated);
            index_block_used = 0;

            return block_to_be_allocated->body.payload;
        }
    }
    // if (wild_check){
    // }
    // printf("start index: %d, padding: %zu\n", startIndex, padding);
    // for (int i = 0; i < NUM_FREE_LISTS; i++){

    // }
    // else if (size % 64 != 0){
    //     return NULL;
    // }

    return NULL;
}

void sf_free(void *pp) {

    if (pp == NULL){
        // printf("The pointer is invalid. Aborting...\n");
        abort();
    } else {
        sf_block* block = pp-16;

        if (((unsigned long)pp & 63) != 0){

            // printf("stupid test cause im stupid 1\n");

            // printf("The pointer is invalid. Aborting...\n");
            abort();
        } else if ((block->header & THIS_BLOCK_ALLOCATED) == 0){
            // printf("stupid test cause im stupid 2\n");

            // printf("The pointer is invalid. Aborting...\n");
            abort();
        } else if ((pp-sizeof(sf_header)) < (sf_mem_start()+56) || (((pp-16)+(block->header & BLOCK_SIZE_MASK))>(sf_mem_end()-8))){
            // printf("stupid test cause im stupid 3\n");

            // printf("The pointer is invalid. Aborting...\n");
            abort();
        } else if ((block->header & PREV_BLOCK_ALLOCATED) == 0){
            int size = block->prev_footer & BLOCK_SIZE_MASK;
            sf_block* prev_block = pp-sizeof(sf_header)-size;
            if ((prev_block->header & THIS_BLOCK_ALLOCATED) == THIS_BLOCK_ALLOCATED){
                // printf("stupid test cause im stupid 4\n");

                // printf("The pointer is invalid. Aborting...\n");
                abort();
            }
        }
    }

    sf_block* block = pp-16;
    int size = block->header & BLOCK_SIZE_MASK;

    // printf("size: %d\n",size);
    // int imgoingcrazy = block->header;s

    // printf("asdfasdfaa: %d\n",imgoingcrazy);


    /*
        TYPES OF FREE:
        0: both prev and next are free
        1: prev is free, next is alloc
        2: prev is alloc, next is free
        3: both prev and next are alloc
    */
        // printf("block->header %lo\n",(block->header));

    int type_of_free = check_adjacent(block);

    // printf("\ntype of free: %d\n\n", type_of_free);

    // sf_block* prev = block
    if (type_of_free == 0){
        // 0: both prev and next are free

        sf_block *epilogue = sf_mem_end()-16;
        sf_block* wilderness = epilogue - (epilogue->prev_footer & BLOCK_SIZE_MASK)/32;

        int prev_size = block->prev_footer & BLOCK_SIZE_MASK;
        int next_size = (block+size/32)->header & BLOCK_SIZE_MASK;
        int total_size = prev_size + size + next_size;
        int m = total_size/64;
        int index = find_index(m);


        sf_block* prev_block = block - (block->prev_footer & BLOCK_SIZE_MASK)/32;
        remove_block_freelist(prev_block);


        sf_block* coalesce_block = block - (block->prev_footer & BLOCK_SIZE_MASK)/32;
        int prev_alloc = coalesce_block->header & PREV_BLOCK_ALLOCATED;

        if (prev_alloc){
            coalesce_block->header = total_size | PREV_BLOCK_ALLOCATED;
        } else {
            coalesce_block->header = total_size;
        }


        if (block+size/32 == wilderness){

            printf("wilderness\n");
            epilogue = sf_mem_end()-16;
            epilogue->prev_footer = coalesce_block->header;
            index = NUM_FREE_LISTS-1;
            wilderness = coalesce_block;
            wilderness->body.links.next = (sf_free_list_heads + index);
            wilderness->body.links.prev = (sf_free_list_heads + index);

            sf_free_list_heads[index].body.links.next = wilderness;
            sf_free_list_heads[index].body.links.prev = wilderness;
            // remove_block_freelist(prev_block);
            return;
        } else {
            sf_block* next_block = coalesce_block + (prev_size + size)/32;
            remove_block_freelist(next_block);

            sf_block* next_next_block = coalesce_block + total_size/32;
            next_next_block->prev_footer = coalesce_block->header;

            add_block_freelist(index,coalesce_block);
               // remove_block_freelist(prev_block);
        }


    } else if (type_of_free == 1){
        // 1: prev is free, next is alloc

        sf_block* block = pp-16;

        int prev_size = block->prev_footer & BLOCK_SIZE_MASK;
        int total_size = prev_size + size;
        sf_block* coalesce_block = block - (block->prev_footer & BLOCK_SIZE_MASK)/32;
        remove_block_freelist(coalesce_block);
        int prev_alloc = coalesce_block->header & PREV_BLOCK_ALLOCATED;

        if (prev_alloc){
            coalesce_block->header = total_size | PREV_BLOCK_ALLOCATED;
        } else {
            coalesce_block->header = total_size;
        }

        sf_block* next_block = coalesce_block + total_size/32;
        next_block->prev_footer = coalesce_block->header;

        int m = total_size/64;
        int index = find_index(m);

        add_block_freelist(index,coalesce_block);

        return;
    } else if (type_of_free == 2){
        // 2: prev is alloc, next is free
        sf_block *epilogue = sf_mem_end()-16;
        sf_block* wilderness = epilogue - (epilogue->prev_footer & BLOCK_SIZE_MASK)/32;

        sf_block* block = pp-16;

        sf_block* remove_block = block + size/32;
        int next_size = (block+size/32)->header & BLOCK_SIZE_MASK;
        int total_size = size + next_size;
        // printf("total size test: %d\n",total_size);

        sf_block* coalesce_block = block;
        if ((block->header & PREV_BLOCK_ALLOCATED) == PREV_BLOCK_ALLOCATED){
            coalesce_block->header = total_size | PREV_BLOCK_ALLOCATED;
        } else
            coalesce_block->header = total_size;

        sf_block* next_block = coalesce_block + total_size/32;
        next_block->prev_footer = coalesce_block->header;

        int m = total_size/64;
        int index = find_index(m);

        if (block+size/32 == wilderness){

            // printf("wilderness\n");
            epilogue = sf_mem_end()-16;
            epilogue->prev_footer = coalesce_block->header;
            index = NUM_FREE_LISTS-1;
            wilderness = coalesce_block;
            wilderness->body.links.next = (sf_free_list_heads + index);
            wilderness->body.links.prev = (sf_free_list_heads + index);

            sf_free_list_heads[index].body.links.next = wilderness;
            sf_free_list_heads[index].body.links.prev = wilderness;
        } else {
            remove_block_freelist(remove_block);
            add_block_freelist(index,coalesce_block);
            return;
        }


    } else if (type_of_free == 3){
        // 3: both prev and next are alloc

        sf_block* block = pp-16;

        block->header = size | PREV_BLOCK_ALLOCATED;
        sf_block* next_block = block + size/32;
        // printf("size: %d, prev-footer: %lo", size, next_block->prev_footer);
        next_block->prev_footer = block->header;

        // printf("size: %d, prev-footer: %lo", size, next_block->prev_footer);
        next_block->header = next_block->header & BLOCK_SIZE_MASK;
        next_block->header = next_block->header | THIS_BLOCK_ALLOCATED;
        // block->header = size;
        int m = size/64;
        int index = find_index(m);

        add_block_freelist(index,block);
        return;
    }

    return;
}

void *sf_realloc(void *pp, size_t rsize) {

    if (pp == NULL){
        sf_errno = EINVAL;
        return NULL;
    } else {
        sf_block* block = pp-16;

        if (((unsigned long)pp & 63) != 0){
            // printf("stupid test cause im stupid 1\n");
            sf_errno = EINVAL;
            return NULL;
        } else if ((block->header & THIS_BLOCK_ALLOCATED) == 0){
            // printf("stupid test cause im stupid 2\n");
            sf_errno = EINVAL;
            return NULL;
        } else if ((pp-sizeof(sf_header)) < (sf_mem_start()+56) || (((pp-16)+(block->header & BLOCK_SIZE_MASK))>(sf_mem_end()-8))){
            // printf("stupid test cause im stupid 3\n");
            sf_errno = EINVAL;
            return NULL;
        } else if ((block->header & PREV_BLOCK_ALLOCATED) == 0){
            int size = block->prev_footer & BLOCK_SIZE_MASK;
            sf_block* prev_block = pp-sizeof(sf_header)-size;
            if ((prev_block->header & THIS_BLOCK_ALLOCATED) == THIS_BLOCK_ALLOCATED){
                // printf("stupid test cause im stupid 4\n");
                sf_errno = EINVAL;
                return NULL;
            }
        }
    }

    if (rsize == 0){
        sf_free(pp);
        return NULL;
    }

    size_t asize = (rsize + sizeof(sf_header) + 63) & (~63); /* Header size */
    // m = asize/64;
    sf_block *block = pp-16;

    // current size
    int csize = block->header & BLOCK_SIZE_MASK;

    if (asize > csize){
        void* larger_block = sf_malloc(rsize);
        if (larger_block == NULL){
            return NULL;
        }
        memcpy(larger_block, pp, csize-sizeof(sf_header));
        sf_free(pp);
        return larger_block;
    } else if (asize < csize){
        sf_block* block_to_be_split = pp-16;
        int old_size = block_to_be_split->header & BLOCK_SIZE_MASK;
        if ((old_size-rsize-8) < 64){
            return pp;
        } else {
            sf_block* new_block = block_to_be_split + asize/32;
            new_block->header = (old_size-asize) | PREV_BLOCK_ALLOCATED;
            new_block->header = new_block->header | THIS_BLOCK_ALLOCATED;
            sf_free(new_block->body.payload);
            if ((block_to_be_split->header & PREV_BLOCK_ALLOCATED) == PREV_BLOCK_ALLOCATED){
                block_to_be_split->header = asize | PREV_BLOCK_ALLOCATED;
            } else {
                block_to_be_split->header = asize;
            }
            block_to_be_split->header = block_to_be_split->header | THIS_BLOCK_ALLOCATED;
            new_block->prev_footer = block_to_be_split->header;
            return block_to_be_split->body.payload;
        }
    } else if (asize == csize){
        return pp;
    }

    return NULL;
}

/*
    @ param, int casted align size to test if is a power of 2
    returns 0, indicating not power of 2, or 1 indicating power of 2

*/
int power_check(int align){
    /*
        Two scenarios where we exit the loop, if we reached end and power/2 == 1
        or if (power % 2 != 0)
    */
    int break_loop = 0;
    int power = align;

    if (power % 2 != 0){
        // printf("test1 \n");
        return 0;
    }

    while (break_loop == 0){

        power = power/2;
        // printf("power: %d\n", power);
        if ((power != 1 && power % 2 != 0)){
            return 0;
        } else if (power == 1){
            break_loop = 1;
        }
    }

    return 1;
}

void *sf_memalign(size_t size, size_t align) {

    if (size == 0){
        return NULL;
    }
    // int power_check = 1;
    // int a = size;
    int check = align;
    // printf("size of size: %d, size_t: %lu, size of align: %d, size_t: %lu\n",a, size, b, align);

    // Check if the requested align is at least as large as minimum block size (2)
    // In terms of size_t, that's (2) * 32 = 64 bytes

    if (align < 64){
        sf_errno = EINVAL;
        return NULL;
    } else {
        int power = power_check(check);
        // printf("power test: %d\n",power);
        if (power == 0){
            sf_errno = EINVAL;
            return NULL;
        }
    }

    /* Three cases:
        1: Immediately use payload (User-defined aligned)), Free afterwards
        2: Payload that is after the initial block, and uses rest of allocated block: Free previous unlikely
        3: Payload that is after the initial block, but doesn't use the rest of the block
    */

    size_t asize;
    size_t big_block_size;
    size_t align_block_size = (size + sizeof(sf_header) + 63) & (~63);

    // size_t align_block_size;


    asize = size + align + 64 + sizeof(sf_header);

    void *payload = sf_malloc(asize);
    // unsigned int test = payload

    sf_block* big_block = payload-16;

    int left_to_aligned;
    if ((unsigned long)payload % align != 0){
        left_to_aligned = align-((unsigned long)big_block->body.payload % align);
    } else {
        left_to_aligned = 0;
    }

    int blocks_before = (left_to_aligned + 63) & (~63);

    big_block_size = big_block->header & BLOCK_SIZE_MASK;

    int blocks_after = big_block_size - (align_block_size + blocks_before);

    // printf("\nasize: %lu, big_block_size: %lu, left to aligned: %d\n\n",asize,big_block_size,left_to_aligned);

    /* STEPS:
        1) Find the next block that satisfies alignment request
        2) profit?
    */

    /*
        If payload % align == 0, we can immediately allocate the memory at the given payload,
        then split afterwards assuming no splinters
    */
    if (left_to_aligned == 0){

        sf_block* next_block = big_block + align_block_size/32;
        next_block->header = (big_block_size - align_block_size) | THIS_BLOCK_ALLOCATED;
        next_block->header = next_block->header | PREV_BLOCK_ALLOCATED;

        if ((big_block->header & PREV_BLOCK_ALLOCATED) == PREV_BLOCK_ALLOCATED){
            big_block->header = align_block_size | PREV_BLOCK_ALLOCATED;
            big_block->header = big_block->header | THIS_BLOCK_ALLOCATED;
        } else {
            big_block->header = align_block_size | THIS_BLOCK_ALLOCATED;
        }

        next_block->prev_footer = big_block->header;
        sf_free(next_block->body.payload);
        return big_block->body.payload;

    } else {
        void* aligned_payload = big_block->body.payload + left_to_aligned;
        // int test = ((((unsigned long)aligned_payload) & (align-1)) == 0);
        // printf("test: %d\nblocks_before: %d\n", test,blocks_before);
        if (blocks_before > 0){
            sf_block* memalign_block = big_block + blocks_before/32;
            sf_block* next_block = memalign_block + align_block_size/32;

            memalign_block->header = align_block_size | THIS_BLOCK_ALLOCATED;
            memalign_block->header = memalign_block->header | PREV_BLOCK_ALLOCATED;

            if ((big_block->header & PREV_BLOCK_ALLOCATED) == PREV_BLOCK_ALLOCATED){
                big_block->header = blocks_before | PREV_BLOCK_ALLOCATED;
                big_block->header = big_block->header | THIS_BLOCK_ALLOCATED;
            } else {
                big_block->header = blocks_before | THIS_BLOCK_ALLOCATED;
            }

            next_block->prev_footer = memalign_block->header;
            next_block->header = next_block->header | PREV_BLOCK_ALLOCATED;

            sf_free(big_block->body.payload);
            memalign_block->prev_footer = big_block->header;

            if (blocks_after > 0){
                memalign_block->header = align_block_size | THIS_BLOCK_ALLOCATED;

                sf_block* next_block = memalign_block + align_block_size/32;
                sf_block* next_next_block = next_block + (big_block_size - (blocks_before + align_block_size))/32;

                next_block->prev_footer = memalign_block->header;
                next_block->header = (big_block_size - (blocks_before + align_block_size)) | THIS_BLOCK_ALLOCATED;
                next_block->header = next_block->header | PREV_BLOCK_ALLOCATED;

                next_next_block->prev_footer = next_block->header;
                next_next_block->header = next_next_block->header | PREV_BLOCK_ALLOCATED;
                sf_free(next_block->body.payload);

                return aligned_payload;
            } else {
                sf_block* next_block = memalign_block + (big_block_size - blocks_before)/32;
                next_block->prev_footer = memalign_block->header;
                next_block->header = next_block->header | PREV_BLOCK_ALLOCATED;

                return aligned_payload;
            }
            // printf("big block: %p\nmemalign block: %p\nnext_block: %p\n\n", big_block, memalign_block, next_block);
        }

    }
    return NULL;
}
