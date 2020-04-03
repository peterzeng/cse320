#include <stdio.h>
#include "sfmm.h"

// find_index(int a){};
extern int find_index();
int main(int argc, char const *argv[]) {


    sf_mem_init();

    char* a = sf_malloc(12);
    char* b = sf_malloc(102);
    char* c = sf_malloc(500);
    char* d = sf_malloc(2131);
    sf_free(b);
    char* e = sf_malloc(3434);
    sf_realloc(c, 10);
    char* f = sf_malloc(600);
    sf_free(e);
    char* g = sf_malloc(4096);
    sf_realloc(a, 2222);


    char* h = sf_malloc(23);
    sf_memalign(3464, 128);
    char* i = sf_malloc(453);
    sf_free(d);
    sf_memalign(200, 1024);
    sf_realloc(f, 353);
    sf_free(g);
    sf_free(h);
    sf_free(i);
    sf_malloc(100);
    sf_malloc(57728-8);
    sf_realloc(a, 1000);
    sf_malloc(12999);
    char *k = sf_malloc(3096);
    char *l = sf_malloc(23);
    sf_free(k);
    sf_free(l);
    char * m= sf_malloc(10000);
    sf_show_heap();
    sf_realloc(m, 20000);
    sf_show_heap();
    char *n = sf_memalign(2000, 4096);
    sf_show_heap();
    sf_free(n);
    char *o=sf_malloc(9000);
    sf_realloc(o, 12);
    sf_malloc(0);
    sf_memalign(200, 8192);
    sf_malloc(0);

    char * string = sf_malloc(56);
    for(int i =0;i < 56; i++){
        *string = 'a';
    }

    sf_free(string);
    sf_malloc(56);

    char * string2 = sf_malloc(248);
    for(int i =0;i < 248; i++){
        *string2 = 'a';
    }
    sf_malloc(12);
    // printf("%p%p%p%p%p%p%p%p%p%p%p\n",a,c,d,e,f,g,h,i,k,l,n);
    // printf("%p%p%p%p%p%p\n",a,c,d,e,f,g);


    // printf("%p%p%p%p%p%p%p%p\n",a,c,d,e,f,g,h,i);

    sf_show_heap();

    sf_mem_fini();

    return EXIT_SUCCESS;
}