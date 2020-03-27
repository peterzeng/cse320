#include <stdio.h>
#include "sfmm.h"

// find_index(int a){};
extern int find_index();
int main(int argc, char const *argv[]) {


    sf_mem_init();
    // sf_memalign(50, 65);
    // sf_memalign(50, 70);
    sf_memalign(400, 64);
    // sf_memalign(50, 0);

    sf_show_heap();

    sf_mem_fini();

    return EXIT_SUCCESS;
}

