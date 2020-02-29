/*
 * DO NOT MODIFY THE CONTENTS OF THIS FILE.
 * IT WILL BE REPLACED DURING GRADING
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include "hash.h"
#include "patchlevel.h"

#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <stdio.h>
#ifdef  BSD
#include <strings.h>
#else
#include <string.h>
#endif

#include "customize.h"

extern int vtree_main(int argc, char *argv[]);

int main(int argc, char *argv[]) {
    return vtree_main(argc, argv);
}
