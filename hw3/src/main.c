#include <stdio.h>
#include "sfmm.h"

// find_index(int a){};
extern int find_index();
int main(int argc, char const *argv[]) {

    // int a = find_index(30);
    // int b = find_index(1);
    // int c = find_index(45);
    // printf("a: %d, b: %d, c: %d\n", a,b,c);
    sf_mem_init();

    // double* ptr = sf_malloc(sizeof(double));

    // *ptr = 320320320e-320;

    // *ptr = 30;
    // printf("%f\n", *ptr);



    // sf_malloc(140);
    // sf_malloc(4200);
    // sf_malloc(sizeof(double));
    // sf_free(ptr)
    sf_malloc(140);
    sf_malloc(4200);
    sf_malloc(sizeof(double));;
    sf_show_heap();

    sf_mem_fini();

    return EXIT_SUCCESS;
}
