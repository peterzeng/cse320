#include <stdio.h>
#include <stdlib.h>

#include "const.h"
#include "debug.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

int main(int argc, char **argv)
{

    // char test=' ';

    // for(int i = 0; i < argc; i++){
    //     printf("%s\n", *(argv+i));
    //     // while (test!='\0'){

    //     // }
    // }
    // int ret = validargs(argc, argv);
    // printf("%d\n", ret);

// TESTING
// TESTING

    if(validargs(argc, argv))
        USAGE(*argv, EXIT_FAILURE);
    debug("Options: 0x%x", global_options);
    if(global_options & 1)
        USAGE(*argv, EXIT_SUCCESS);

    if (global_options == 4){
        int bytes = decompress(stdin, stdout);
        if (bytes == EOF){
            return EXIT_FAILURE;
        } else {
            return EXIT_SUCCESS;
        }
    } else if ((global_options & 2) == 2){
        int blocksize = global_options >> 16;
        int bytes = compress(stdin, stdout, blocksize);
        if (bytes == EOF){
            return EXIT_FAILURE;
        } else {
            return EXIT_SUCCESS;
        }
    }

    return EXIT_SUCCESS;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
