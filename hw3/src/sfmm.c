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

int find_index(int m){
    if (m == 1){
        return 0;
    } else if (m == 2){
        return 1;
    } else if (m == 3){
        return 2;
    } else if (m > 3 && m <= 5){
        return 3;
    } else if (m > 5 && m <= 8){
        return 4;
    } else if (m > 8 && m <= 13){
        return 5;
    } else if (m > 13 && m <= 21){
        return 6;
    } else if (m > 21 && m <= 34){
        return 7;
    } else if (m > 34){
        return 8;
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
    // sf_block *heapTrackm ==;
    printf("size: %zu\n",size);
    if (size == 0){
        return NULL;
    }
    if (sf_mem_start() == sf_mem_end()){
        sf_mem_grow();
        // TEST
        // void* start = sf_mem_start();
        // void* end = sf_mem_end();
        // printf("start: %p\nend: %p\n",start, end);
        // // TEST
        // sf_mem_grow();
        // start = sf_mem_start();
        // end = sf_mem_end();
        // printf("start: %p\nend: %p\n",start, end);

        initialize_heap(sf_mem_start());

    }
    // printf("showing heap \n");
    // sf_show_heap();

    // printf("segfault? \n");
    // sf_show_free_lists();
    // sf_show_heap();

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
    int startIndex = find_index(m); /* Starting index of list head to look */

    sf_block* block_to_be_allocated = find_block(startIndex, &sf_free_list_heads[startIndex]);
    // sf_show_heap();
    // sf_show_heap();s
    // printf("wilderness: %p\n", sf_free_list_heads[9].body.links.next);
    // // sf_show_free_lists();
    // printf("Wilderness test: %p\n", block_to_be_allocated);

    size_t check_alloc_block; /*  Mask alloc block and see if it's larger or equal to size of data to be allocated */
    check_alloc_block = block_to_be_allocated->header & BLOCK_SIZE_MASK;

    // size_t test = block_to_be_allocated->header;
    // printf("test: %zu\n", test);
    // printf("size of check_alloc_block: %zu\n", check_alloc_block);
    // What's left of the block after the asize is used
    check_alloc_block = check_alloc_block-asize; /* subtract asize from block to see if it's proper size */
    printf("size of check_alloc_block: %zu, asize: %zu\n", check_alloc_block,asize);
    // sf_show_blocks();
    if (check_alloc_block == 0){ /* Perfect Size */

    } else {
        if (wild_check == 1){
            /* Set the header size of the space to be allocated first to asize */
                // printf("wilderness header, %d\n", (int)wilderness->header);
            int test = (wilderness->header) & PREV_BLOCK_ALLOCATED;
            // printf("test: %d\n", test);
            block_to_be_allocated->header = asize | THIS_BLOCK_ALLOCATED;

            if (test == 2){
                printf("wilderness header, %zu\n", wilderness->header);
                block_to_be_allocated->header = block_to_be_allocated->header | PREV_BLOCK_ALLOCATED;
            }
            printf("test epilogue, wilderness: %p, %p\n\n",epilogue, wilderness);

            wilderness = block_to_be_allocated + (asize/32);

            printf("test epilogue, wilderness: %p, %p\n\n",epilogue, wilderness);

            epilogue = wilderness + (check_alloc_block/32); /* Setting new prologue */

            printf("test epilogue, wilderness: %p, %p\n\n",epilogue, wilderness);

            // wilderness->header = check_alloc_block | THIS_BLOCK_ALLOCATED;
            wilderness->header = check_alloc_block | PREV_BLOCK_ALLOCATED;
            printf("wilderness header: %zu\n", wilderness->header);
            wilderness->prev_footer = block_to_be_allocated->header;
            sf_free_list_heads[9].body.links.next = wilderness;
            wilderness->body.links.next = (sf_free_list_heads+ 9);
            wilderness->body.links.prev = (sf_free_list_heads + 9);
            epilogue->prev_footer = wilderness->header;
            sf_show_heap();
            return block_to_be_allocated;
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
