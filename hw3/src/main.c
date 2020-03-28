#include <stdio.h>
#include "sfmm.h"

// find_index(int a){};
extern int find_index();
int main(int argc, char const *argv[]) {


    sf_mem_init();


 void* y = sf_malloc(576);
    // sf_show_heap();

    sf_malloc(500);
    // sf_show_heap();

    sf_free(y);
    // sf_show_heap();

    void* test = sf_memalign(400, 64);
    void* test2 = sf_memalign(400, 512);
    void* test3 = sf_memalign(200, 1024);
    void* test4 = sf_memalign(800, 2048);
    void* test5 = sf_memalign(300, 256);

    int a = (long)test % 64;
    int b = (long)test2 % 512;
    int c = (long)test3 % 1024;
    int d = (long)test4 % 2048;
    int e = (long)test5 % 256;

    printf("%d %d %d %d %d\n",a,b,c,d,e);



    sf_show_heap();

    sf_mem_fini();

    return EXIT_SUCCESS;
}

