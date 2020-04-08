--- hw2/dev_repo/basecode/hw2/src/main.c

+++ hw2/repos/pezeng/hw2/src/main.c

@@ -2,7 +2,28 @@

  * DO NOT MODIFY THE CONTENTS OF THIS FILE.

  * IT WILL BE REPLACED DURING GRADING

  */

-#include <stdlib.h>

+

+#ifdef LINUX

+// #include <stdlib.h>

+// printf("test\n");

+#endif

+#include <stdio.h>

+#include <sys/types.h>

+#include "hash.h"

+#include "patchlevel.h"

+

+#include <ctype.h>

+#include <sys/types.h>

+#include <sys/stat.h>

+#include <sys/param.h>

+#include <stdio.h>

+#ifdef  BSD

+#include <strings.h>

+#else

+#include <string.h>

+#endif

+

+#include "customize.h"

 

 extern int vtree_main(int argc, char *argv[]);

 
