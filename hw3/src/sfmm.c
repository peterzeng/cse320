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

void initialize_heap(sf_block* start)  {
    sf_block *prologue = start + 48;
    prologue -> header = 64;
    prologue -> header |= 1;
    prologue -> header |= 2;

    sf_block *wilderness = prologue + 64;
    wilderness -> prev_footer = prologue -> header;
    wilderness -> header = 3968 | 2;

    sf_block *epilogue = wilderness + 3968;
    epilogue -> prev_footer = wilderness -> header;
    epilogue -> header = 1;

    for (int i = 0; i < NUM_FREE_LISTS; i++){
        if (i == 9){
            sf_free_list_heads[i].body.links.next = wilderness;
            break;
        }
        sf_free_list_heads[i].body.links.next = &sf_free_list_heads[i];
        sf_free_list_heads[i].body.links.prev = &sf_free_list_heads[i];
    }
}

void *sf_malloc(size_t size) {
    // sf_block *heapTrack;
    if (size == 0){
        return NULL;
    }
    if (sf_mem_start() == sf_mem_end()){
        sf_mem_grow();
        initialize_heap(sf_mem_start());


    }
    size_t asize; /* Adjusted size */
    int m; /* Multiple of 64 */
    size_t padding; /* Padding to make multiple of 64 */
    size_t extendsize; /* Amount to extend heap? */

    asize = size + 8; /* Header size */

    if (asize <= 64) {
        padding = 64 - asize;
        asize = 64;
        m = 1;
    } else if (asize > 64){
        int temp = asize / 64;
        asize = 64* (temp + 1);
        m = temp + 1;
    }

    for (int i = 0; i < NUM_FREE_LISTS; i++){

    }
    // else if (size % 64 != 0){
    //     return NULLasdas;
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
