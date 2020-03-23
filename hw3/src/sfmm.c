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
sf_block *prologue;
sf_block *wilderness;
sf_block *epilogue;

void initialize_heap(void* start)  {
    prologue = start + 48;
    prologue -> header = 64;
    prologue -> header |= THIS_BLOCK_ALLOCATED;
    prologue -> header |= PREV_BLOCK_ALLOCATED;

    wilderness = prologue + 2;
    wilderness -> prev_footer = prologue -> header;
    wilderness -> header = 3968 | 2;

    epilogue = wilderness + 124;
    epilogue -> prev_footer = wilderness -> header;
    epilogue -> header = 1;

    for (int i = 0; i < NUM_FREE_LISTS; i++){
        if (i == 9){
            sf_free_list_heads[i].body.links.next = wilderness;
            wilderness->body.links.next = (sf_free_list_heads + 9);
            wilderness->body.links.prev = (sf_free_list_heads + 9);
            break;
        }
        sf_free_list_heads[i].body.links.next = &sf_free_list_heads[i];
        sf_free_list_heads[i].body.links.prev = &sf_free_list_heads[i];
    }
}

int check_more_mem(int size_of_block){
    // Start after prologue to see if we have blocks that will fit the requested malloc
    sf_block *where = prologue + 2;
    while (where < epilogue){
        // printf("Where: %p, epilogue: %p\n", where,epilogue);
        if (((where->header) & THIS_BLOCK_ALLOCATED) == THIS_BLOCK_ALLOCATED){
            // printf("current header: %ld\n",whaere->header);
            where += (where->header & BLOCK_SIZE_MASK)/32;
        } else {
            if (size_of_block > (where->header & BLOCK_SIZE_MASK)){
                where += (where->header & BLOCK_SIZE_MASK)/32;
            } else if (size_of_block <= (where->header & BLOCK_SIZE_MASK)){
                return 1;
            }
        }

    }
    return 0;
}

// int find_index(int m){
//     if (m == 1){
//         return 0;
//     } else if (m == 2){
//         return 1;
//     } else if (m == 3){
//         return 2;
//     } else if (m > 3 && m <= 5){
//         return 3;
//     } else if (m > 5 && m <= 8){
//         return 4;
//     } else if (m > 8 && m <= 13){
//         return 5;
//     } else if (m > 13 && m <= 21){
//         return 6;
//     } else if (m > 21 && m <= 34){
//         return 7;
//     } else if (m > 34){
//         return 8;
//     }
//     return -1;
// }

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

sf_block *find_block(int startIndex, sf_block* sentinel){
    if (startIndex < 9){
        if (sentinel->body.links.next != sentinel){
            wild_check = 0;
            return sentinel->body.links.next;
        } else {
            // printf("test Recursion\n");
            find_block(startIndex+1, &sf_free_list_heads[startIndex+1]);
        }
    } else {
        // printf("test for startIndex\n");
        wild_check = 1;
        return sf_free_list_heads[9].body.links.next;
    }
        return sf_free_list_heads[9].body.links.next;
}

void *sf_malloc(size_t size) {

    printf("size: %zu\n",size);
    if (size <= 0){
        return NULL;
    }
    if (sf_mem_start() == sf_mem_end()){
        sf_mem_grow();
        initialize_heap(sf_mem_start());
    }

    // FIND SIZE OF BLOCK TO BE ALLOCATED //

    size_t asize; /* Adjusted size */
    int m; /* Multiple of 64 */
    // size_t padding; /* Padding to make multiple of 64 */
    // size_t extendsize; /* Amount to extend heap? */

    asize = size + 8; /* Header size */

    if (asize <= 64) {
        // padding = 64 - asize;
        asize = 64;
        m = 1;
    } else if (asize > 64){
        int temp = asize / 64;
        asize = 64* (temp + 1);
        m = temp + 1;
    }

    while(check_more_mem(asize) == 0){
        sf_block *new_end = sf_mem_grow();
        if (new_end == NULL){
            sf_errno = ENOMEM;
            return NULL;
        };

        wilderness->header += 4096;
        epilogue = wilderness + (4096/32);
        epilogue->prev_footer = wilderness->header;
    }

    int startIndex = find_index(m); /* Starting index of list head to look */

    sf_block* block_to_be_allocated = find_block(startIndex, &sf_free_list_heads[startIndex]);

    // printf("Wilderness test: %p\n", block_to_be_allocated);

    size_t check_alloc_block; /*  Mask alloc block and see if it's larger or equal to size of data to be allocated */
    check_alloc_block = block_to_be_allocated->header & BLOCK_SIZE_MASK;

    // size_t test = block_to_be_allocated->header;
    // printf("test: %zu\n", test);
    // printf("size of check_alloc_block: %zu\n", check_alloc_block);
    // What's left of the block after the asize is used
    check_alloc_block = check_alloc_block-asize; /* subtract asize from block to see if it's proper size */
    // printf("size of check_alloc_block: %zu, asize: %zu\n", check_alloc_block,asize);

    if (check_alloc_block == 0){ /* Perfect Size */
        // block_to_be_allocated->header
    } else {
        if (wild_check == 1){
            /* Set the header size of the space to be allocated first to asize */
                // printf("wilderness header, %d\n", (int)wilderness->header);
            int test = (wilderness->header) & PREV_BLOCK_ALLOCATED;
            // printf("test: %d\n", test);
            block_to_be_allocated->header = asize | THIS_BLOCK_ALLOCATED;

            if (test == 2){
                // printf("wilderness header, %zu\n", wilderness->header);
                block_to_be_allocated->header = block_to_be_allocated->header | PREV_BLOCK_ALLOCATED;
            }
            // printf("test epilogue, wilderness: %p, %p\n\n",epilogue, wilderness);

            wilderness = block_to_be_allocated + (asize/32);

            // printf("test epilogue, wilderness: %p, %p\n\n",epilogue, wilderness);

            epilogue = wilderness + (check_alloc_block/32); /* Setting new prologue */

            // printf("test epilogue, wilderness: %p, %p\n\n",epilogue, wilderness);

            // wilderness->header = check_alloc_block | THIS_BLOCK_ALLOCATED;
            wilderness->header = check_alloc_block | PREV_BLOCK_ALLOCATED;
            printf("wilderness header: %zu\n", wilderness->header);
            wilderness->prev_footer = block_to_be_allocated->header;
            sf_free_list_heads[9].body.links.next = wilderness;
            wilderness->body.links.next = (sf_free_list_heads+ 9);
            wilderness->body.links.prev = (sf_free_list_heads + 9);
            epilogue->prev_footer = wilderness->header;
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
    return;
}

void *sf_realloc(void *pp, size_t rsize) {
    return NULL;
}

void *sf_memalign(size_t size, size_t align) {
    return NULL;
}
