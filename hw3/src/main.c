#include <stdio.h>
#include "sfmm.h"

// find_index(int a){};

int main(int argc, char const *argv[]) {


    sf_mem_init();
sf_malloc(4000);
    sf_malloc(5000);
    void* a = sf_malloc(235);
    void* b = sf_malloc(354);
    void* c = sf_malloc(542);
    sf_malloc(2500);
    sf_malloc(2);
    sf_free(c);
    sf_free(a);
    sf_free(b);
    sf_show_heap();

    sf_mem_fini();

    return EXIT_SUCCESS;
}

