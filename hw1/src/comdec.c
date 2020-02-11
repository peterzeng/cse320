#include "const.h"
#include "sequitur.h"
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

/*
 * You may modify this file and/or move the functions contained here
 * to other source files (except for main.c) as you wish.
 *
 * IMPORTANT: You MAY NOT use any array brackets (i.e. [ and ]) and
 * you MAY NOT declare any arrays or allocate any storage with malloc().
 * The purpose of this restriction is to force you to use pointers.
 * Variables to hold the pathname of the current file or directory
 * as well as other data have been pre-declared for you in const.h.
 * You must use those variables, rather than declaring your own.
 * IF YOU VIOLATE THIS RESTRICTION, YOU WILL GET A ZERO!
 *
 * IMPORTANT: You MAY NOT use floating point arithmetic or declare
 * any "float" or "double" variables.  IF YOU VIOLATE THIS RESTRICTION,
 * YOU WILL GET A ZERO!
 */

/**
 * Main compression function.
 * Reads a sequence of bytes from a specified input stream, segments the
 * input data into blocks of a specified maximum number of bytes,
 * uses the Sequitur algorithm to compress each block of input to a list
 * of rules, and outputs the resulting compressed data transmission to a
 * specified output stream in the format detailed in the header files and
 * assignment handout.  The output stream is flushed once the transmission
 * is complete.
 *
 * The maximum number of bytes of uncompressed data represented by each
 * block of the compressed transmission is limited to the specified value
 * "bsize".  Each compressed block except for the last one represents exactly
 * "bsize" bytes of uncompressed data and the last compressed block represents
 * at most "bsize" bytes.
 *
 * @param in  The stream from which input is to be read.
 * @param out  The stream to which the block is to be written.
 * @param bsize  The maximum number of bytes read per block.
 * @return  The number of bytes written, in case of success,
 * otherwise EOF.
 */
int compress(FILE *in, FILE *out, int bsize) {
    // To be implemented.
    return EOF;
}

/**
 * Main decompression function.
 * Reads a compressed data transmission from an input stream, expands it,
 * and and writes the resulting decompressed data to an output stream.
 * The output stream is flushed once writing is complete.
 *
 * @param in  The stream from which the compressed block is to be read.
 * @param out  The stream to which the uncompressed data is to be written.
 * @return  The number of bytes written, in case of success, otherwise EOF.
 */
int decompress(FILE *in, FILE *out) {
    // To be implemented.
    return EOF;
}

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 0 if validation succeeds and -1 if validation fails.
 * Upon successful return, the selected program options will be set in the
 * global variable "global_options", where they will be accessible
 * elsewhere in the program.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 0 if validation succeeds and -1 if validation fails.
 * Refer to the homework document for the effects of this function on
 * global variables.
 * @modifies global variable "global_options" to contain a bitmap representing
 * the selected options.
 */
int stringlen(char *str){
    int counter = 0;
    int incr = 0;
    char check = *str;
    while (check != '\0'){
        counter++;
        incr++;
        check=*(str+incr);
    }
    return counter;
}

int validargs(int argc, char **argv)
{
    if (argc == 0){
        return -1;
    }
    char test = ' '; // The char being compared, initialized as ' '
    int increment = 0; // To help check the chars
    int blocksize = 0; // Is the blocksize, initialized as false
    int actualBlocksize = 0; // Well, the boolean for the actualBlocksize anyways
    int intBlocksize = 0; // The numeric integer value of blocksize.

    for (int i = 1; i < argc; i++){
        // printf("actualblocksize%d\n" ,actualBlocksize);
        // printf("test\n");
        char *arg = *(argv+i);
        // printf("%s\n", arg);
        // Checking to make sure that length of non-blocksize flags is 2
        int length = stringlen(arg);

        if (blocksize == 0){
            if (length != 2){
                printf("debug\n");
                return -1;
            }
        }

        if (actualBlocksize == 1){
            if (length < 1 || length > 4){
                return -1;
            }
        }

        // Now testing the characters themselves. The first HAS TO BE '-'
        while  (test!='\0'){
            test = *(arg+increment);
            // printf("Test: %c\n", test);s

            // Really janky code to convert the bytes to ints.
            if (actualBlocksize == 1){ // Check if it's a digit between 0 and 9'

                if (test < 48 || test > 57){
                    if (test == '\0'){
                        if (intBlocksize >= 1 && intBlocksize <= 1024){
                            intBlocksize = intBlocksize << 16;
                            global_options = global_options | intBlocksize;
                            global_options = global_options | 2;
                            return 0;
                        }
                    } else
                        return -1;
                } else {
                    int temp = test-48; // Convert to int
                    if (length-increment == 4){
                        temp = temp * 1000;
                    } else if (length-increment == 3){
                        temp = temp * 100;
                    } else if (length - increment == 2){
                        temp = temp * 10;
                    }
                    intBlocksize = intBlocksize + temp;
                    // printf("blocksize: %d\n", intBlocksize);
                }
            }

            if (increment == 0 && blocksize == 0){ // If not blocksize, and first char isn't '-', return -1
                if (test != '-'){
                    return -1;
                }
            } else if (increment == 1 && blocksize == 0){
                // Code to see which flag: ONLY POSSIBLE RN ARE -h, -c (special case), -d
                if (test == 'h'){ // We don't care if there's anything after h, and we already checked to make sure the length is 2.
                    global_options = global_options | 1;
                    return 0;
                } else if (test == 'd'){ // If decompress, no option for blocksize, and -d should be the only flag.
                    if (argc != 2){
                        return -1;
                    } else {
                        global_options = global_options | 4;
                        return 0;
                    }
                } else if (test == 'c'){
                    if (argc == 2){
                        global_options = global_options | 2;
                        return 0;
                    } else if (argc == 4){
                        blocksize = 1; // We have a potential blocksize which has an additional flag as well as the blocksize itself. 4 args
                        // printf("%d\n", argc); // debug
                    } else {
                        // printf("%d\n", argc); // debug
                        return -1;
                    }
                }

            } else if (increment == 0 && blocksize == 1 && actualBlocksize != 1){
                if (test != '-'){
                        printf("%d\n", argc); // debug
                    return -1;
                }
            } else if (increment == 1 && blocksize == 1 && actualBlocksize != 1){

                if (test != 'b'){
                    // printf("asadfasdfas%d\n", argc); // debug
                    return -1;
                } else if (test == 'b'){
                    // printf("should be right: %d\n", argc); // debug
                    actualBlocksize = 1; // Setting actualBlocksize to true
                    increment = 0;
                    break;
                }
            }

            increment++;
        }
        increment = 0;
        test = ' ';
    }
    // printf("test: %d\n", intBlocksize);

    // To be implemented.
    return -1;
}
