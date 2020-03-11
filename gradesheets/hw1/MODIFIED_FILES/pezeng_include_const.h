--- hw1/dev_repo/solution/include/const.h

+++ hw1/repos/pezeng/hw1/include/const.h

@@ -33,6 +33,7 @@

  * to declare any arrays (or use any array brackets at all) in your own code.

  * Also, some of the tests we make on your program may rely on being able to

  * inspect the contents of these variables.

+ * inspect the contents of these variables.

  */

 

 /* Options info, set by validargs. */

@@ -57,6 +58,7 @@

  * Array, used during decompression, that maps symbol values to nonterminal symbols.

  */

 SYMBOL *rule_map[SYMBOL_VALUE_MAX];

+

 

 /*

  * Below this line are prototypes for functions that MUST occur in your program.
