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

// TESTING
int count = 0;
void print_rules(void) {
    count++;
    printf("Number of blocks: %d\n", count);
    SYMBOL *rule = main_rule;
    SYMBOL *symbol;

    while (1) {
        symbol = rule;
        printf("Rule value: %d\n", symbol->value);

        while(1) {
            printf("Symbol value: %d\n", symbol->value);
            symbol = symbol->next;

            if (symbol == rule)
                break;
        }
        printf("\n");
        rule = rule->nextr;

        if (rule == main_rule)
            break;
    }

    // printf("rule-map_valuen%d\n", (*(rule_map+256))->value);
    // printf("rule-map_valuen%d\n", (*(rule_map+257))->value);

}


void expand_rules(SYMBOL* rule, FILE *out){
    // TESTING
    // printf("test value: %d\n", check->value);
    SYMBOL* symbol = rule->next;
    while (symbol != rule){
        // printf("symbol value: %d\n", symbol->value);
        if (symbol->value < FIRST_NONTERMINAL){
            fputc(symbol->value, out);
            // expand_rules(rule->next, out);
            symbol = symbol->next;
            // rule = rule->next;
        } else {
            expand_rules(*(rule_map+(symbol->value)), out);
            symbol = symbol->next;
        }
    }

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
    FILE *start = in;

    int bytes_written = 0;

    // If start of transmission
    int start_transmission = 0;

    // If start of block
    int start_block = 0;

    // If End block
    int end_block = 0;
    // UTF to be translated
    int final_utf = 0;

    // Num of bytes in string
    // int num_bytes = 0;

    // New symbol ( as opposed to finding the rest of an old symbol / starts at true)
    int new_symbol_check = 1;

    // bytes left to check in symbol, if new_symbol_check == 0
    int bytes_left = 0;

    // If we're looking for a new rule, only after we find rd 0x85
    int new_rule_check = 0;

    // The current byte we're checking
    int check = 0;

    // The current head of the rule that symbols are added to
    SYMBOL* sentinel;

    // The pointer to the symbol that was last made
    // SYMBOL* last_symbol;

    // The pointer to the newest symbol made
    SYMBOL* new;

    // printf("test: %d", check);
    // CONVERTING TO UTF-8 TO VALUES
    while (check != EOF){

        if (end_block){
            // printf("test\n");
            // print_rules();
            expand_rules(main_rule,out);
            // fflush(out);

            // printf("\n");
            check = fgetc(start);

            // printf("\n\ncheck: %d\n", check);
            if (check == 0x82){
                // REACHED END TRANSMISSION
                // printf("REACHED END OF TRANSMISSION\n");
                // bytes_written++;
                fflush(out);
                break;
            } else if (check == 0x83){
                start_block = 1;
                end_block = 0;
                new_rule_check = 1;
                new_symbol_check = 1;
                continue;
            } else {
                return EOF;
            }
        }
// THIS LOGIC PREVENTS CALLING fgetc() AN INACCURATE NUMBER OF TIMES
        if (!start_transmission || !start_block){
            check = fgetc(start);
        }

// SHOULD BE WORKING

        if (!start_transmission){
            if (check == 0x81){
                // printf("Should start\n");
                start_transmission = 1;
                new_rule_check = 1;
                // bytes_written++;
                continue;
            } else if (check == EOF) {
                return EOF;
            }
            continue;
        }

// SHOULD BE WORKING


// CAN HAVE ONE OR MORE BLOCKS
// CAN ONLY BE A NEW BLOCK IF IT'S A NEW SYMBOL
// ONC NEW BLOCK LOOK FOR NEW RULE: n_r_c == 1

        if (!start_block){
            if (check == 0x83 && new_rule_check){
                // printf("START OF BLOCK\n");
                start_block = 1;
                new_rule_check = 1;
                // bytes_written++;
                continue;
            } else if (check == EOF){
                return EOF;
            }
            continue;
        }

// EVERY BLOCK WE NEED TO INITIALIZE SYMBOLS AND RULES
        if (start_block){
            // printf("reinitialized\n");
            init_rules();
            init_symbols();

// ESSENTIALLY GO UNTIL WE REACH END OF BLOCK
            while (!((check == 0x84) & new_symbol_check)){
                check = fgetc(start);
                if (check == 0x84 && new_symbol_check){
                    // REACHED END OF BLOCK
                    // printf("REACHED END OFF BLOCK\n");
                    start_block = 0;
                    end_block = 1;
                    // bytes_written++;
                    break;
                }
                // printf("check: %d\n", check);
                if (new_symbol_check){
                    // printf("new_symbo_check, check %d %d\n",new_symbol_check, check);
                    if (check == 0x85){
                        // IF NEW RULE DELIMITER, WE NEED NEW RULE OBVIOUSLY
                        new_rule_check = 1;
                        bytes_written++;

                        // // IN ORDER TO GET NEXT BYTE, WE CALL FGETC
                        // check = fgetc(start);
                        // // WE GOT THE FIRST BYTE OF A RULE SO new_symbol_check = 0
                        // new_symbol_check = 0;
                        // bytes_left = 1;


                    } else if ((check & 0b11110000) == 0b11110000){
                        new_symbol_check = 0;
                        bytes_left = 3;
                        final_utf = check & 0b00000111;
                        // check = fgetc(start);

                    } else if ((check & 0b11100000) == 0b11100000){
                        new_symbol_check = 0;
                        bytes_left = 2;
                        final_utf = check & 0b00001111;
                        // check = fgetc(start);

                    } else if ((check & 0b11000000) == 0b11000000){
                        // printf("2 bytes\n");
                        new_symbol_check = 0;
                        bytes_left = 1;
                        final_utf = check & 0b00011111;
                        // check = fgetc(start);

                    } else if ((check & 0b10000000) != 0b10000000) {
                        final_utf = check & 0b01111111;
                        // printf("1 byte value: %d\n",final_utf);
                        new = new_symbol(final_utf, sentinel);

                        // printf("Sentinel: %d\n", sentinel->value);
                        // printf("Last symbol: %d\n", sentinel->prev->value);

                        sentinel->prev->next = new;
                        new->prev = sentinel->prev;

                        sentinel->prev = new;
                        new->next = sentinel;

                        bytes_written++;
                        // check = fgetc(start);
                    } else {
                        return EOF;
                    }

                } else if (!new_symbol_check){
                    if (bytes_left == 1){
                        check = check & 0b00111111;
                        final_utf = final_utf << 6;
                        final_utf = final_utf | check;
                        // printf("test: %d \n",final_utf);
                        // printf("nrc, finalutf: %d %d \n", new_rule_check, final_utf);

                        // WE'RE ADDING A NEW RULE
                        if ((new_rule_check) && (final_utf >= FIRST_NONTERMINAL)){
                            // printf("ADDING NEW RULE\n");
                            // printf("nrc%d\n",FIRST_NONTERMINAL);
                            sentinel = new_rule(final_utf);
                            // printf("New rule value: %d\n", final_utf);
                            add_rule(sentinel);
                            // printf("Main rule value: %d\n", main_rule->value);

                            *(rule_map+final_utf) = sentinel;
                            // printf("last_symbol: %p\n", last_symbol);

                            new_rule_check = 0;
                            new_symbol_check = 1;
                            bytes_written++;

                        } else if (!new_rule_check){
                            // printf("does this not work? \n");

                            new = new_symbol(final_utf, sentinel);

                            sentinel->prev->next = new;
                            new->prev = sentinel->prev;

                            sentinel->prev = new;
                            new->next = sentinel;
                            new_symbol_check = 1;
                            bytes_written++;
                        };


                    } else if (bytes_left > 1 && bytes_left <= 3){
                        check = check & 0b00111111;
                        final_utf = final_utf << 6;
                        final_utf = final_utf | check;
                        for (int i = 0; i < bytes_left-1; i++){
                            check = fgetc(start);
                            check = check & 0b00111111;
                            final_utf = final_utf << 6;
                            final_utf = final_utf | check;
                        }
                        // if (final_utf < )
                        // new_symbol(final_utf)
                        // printf("check value: %d\n",final_utf);

                       if ((new_rule_check) && (final_utf >= FIRST_NONTERMINAL)){
                            // printf("nrc%d\n",FIRST_NONTERMINAL);
                            // printf("ADDING NEW RULE\n");
                            sentinel = new_rule(final_utf);
                            add_rule(sentinel);
                            *(rule_map+final_utf) = sentinel;

                            // printf("last_symbol: %p\n", last_symbol);
                            new_rule_check = 0;

                            new_symbol_check = 1;
                            bytes_written++;

                        } else if (!new_rule_check){
                            // printf("does this not work? \n");

                            new = new_symbol(final_utf, sentinel);
                            sentinel->prev->next = new;
                            new->prev = sentinel->prev;

                            sentinel->prev = new;
                            new->next = sentinel;
                            new_symbol_check = 1;
                            bytes_written++;
                        };
                    } else {
                        // ERROR
                        return EOF;
                    }
                }
            }

        }
    }
    return bytes_written;
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
