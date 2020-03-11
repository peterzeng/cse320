--- hw1/dev_repo/solution/src/sequitur.c

+++ hw1/repos/pezeng/hw1/src/sequitur.c

@@ -29,35 +29,35 @@

  */

 static void join_symbols(SYMBOL *this, SYMBOL *next) {

     debug("Join %s%lu%s and %s%lu%s",

-	  IS_RULE_HEAD(this) ? "[" : "<", SYMBOL_INDEX(this), IS_RULE_HEAD(this) ? "]" : ">",

-	  IS_RULE_HEAD(next) ? "[" : "<", SYMBOL_INDEX(next), IS_RULE_HEAD(next) ? "]" : ">");

+      IS_RULE_HEAD(this) ? "[" : "<", SYMBOL_INDEX(this), IS_RULE_HEAD(this) ? "]" : ">",

+      IS_RULE_HEAD(next) ? "[" : "<", SYMBOL_INDEX(next), IS_RULE_HEAD(next) ? "]" : ">");

     if(this->next) {

-	// We will be assigning to this->next, which will destroy any digram

-	// that starts at this.  So that is what we have to delete from the table.

-	digram_delete(this);

-

-	// We have to have special-case treatment of triples, which are occurrences

-	// of the same three symbols in a row.  These violations of "no repeated digrams"

-	// are allowed because creating a rule to replace two of the three symbols

-	// would result in a rule that is only used once, thus violating the "rule utility"

-	// constraint.  However, the digram hash table will only be able to record

-	// one of the two overlapping digrams.  In the present situation case,

-	// the deletion we have just performed has deleted the only instance of this

-	// digram in the hash table, but there will still be one left once we join

-	// the remaining nodes.  So we need to detect this and put the digram back

-	// into the table.

-	//

-	// There are two cases (the ^ shows which is getting deleted):

-	//    abbbc  ==> abbc   (handle this first)

-	//      ^

-	//    abbbc  ==> abbc   (then check for this one)

+    // We will be assigning to this->next, which will destroy any digram

+    // that starts at this.  So that is what we have to delete from the table.

+    digram_delete(this);

+

+    // We have to have special-case treatment of triples, which are occurrences

+    // of the same three symbols in a row.  These violations of "no repeated digrams"

+    // are allowed because creating a rule to replace two of the three symbols

+    // would result in a rule that is only used once, thus violating the "rule utility"

+    // constraint.  However, the digram hash table will only be able to record

+    // one of the two overlapping digrams.  In the present situation case,

+    // the deletion we have just performed has deleted the only instance of this

+    // digram in the hash table, but there will still be one left once we join

+    // the remaining nodes.  So we need to detect this and put the digram back

+    // into the table.

+    //

+    // There are two cases (the ^ shows which is getting deleted):

+    //    abbbc  ==> abbc   (handle this first)

+    //      ^

+    //    abbbc  ==> abbc   (then check for this one)

         //     ^

-	if(next->prev && next->next &&

-	   next->value == next->prev->value && next->value == next->next->value)

-	    digram_put(next);

-	if(this->prev && this->next &&

-	   this->value == this->prev->value && this->value == this->next->value)

-	    digram_put(this);

+    if(next->prev && next->next &&

+       next->value == next->prev->value && next->value == next->next->value)

+        digram_put(next);

+    if(this->prev && this->next &&

+       this->value == this->prev->value && this->value == this->next->value)

+        digram_put(this);

     }

     this->next = next;

     next->prev = this;

@@ -72,7 +72,7 @@

  */

 void insert_after(SYMBOL *this, SYMBOL *next) {

     debug("Insert symbol <%lu> after %s%lu%s", SYMBOL_INDEX(next),

-	  IS_RULE_HEAD(this) ? "[" : "<", SYMBOL_INDEX(this), IS_RULE_HEAD(this) ? "]" : ">");

+      IS_RULE_HEAD(this) ? "[" : "<", SYMBOL_INDEX(this), IS_RULE_HEAD(this) ? "]" : ">");

     join_symbols(next, this->next);

     join_symbols(this, next);

 }

@@ -86,8 +86,8 @@

 static void delete_symbol(SYMBOL *this) {

     debug("Delete symbol <%lu> (value=%d)", SYMBOL_INDEX(this), this->value);

     if(IS_RULE_HEAD(this)) {

-	fprintf(stderr, "Attempting to delete a rule sentinel!\n");

-	abort();

+    fprintf(stderr, "Attempting to delete a rule sentinel!\n");

+    abort();

     }

     // Splice the symbol out, deleting the digram headed by the neighbor to the left.

     join_symbols(this->prev, this->next);

@@ -98,7 +98,7 @@

     // If the deleted node is a nonterminal, decrement the reference count of

     // the associated rule.

     if(IS_NONTERMINAL(this))

-	unref_rule(this->rule);

+    unref_rule(this->rule);

 

     // Recycle the deleted symbol for re-use.

     recycle_symbol(this);

@@ -112,10 +112,10 @@

 static void expand_instance(SYMBOL *this) {

     SYMBOL *rule = this->rule;

     debug("Expand last instance of underutilized rule [%lu] for %d",

-	   SYMBOL_INDEX(rule), rule->value);

+       SYMBOL_INDEX(rule), rule->value);

     if(rule->refcnt != 1) {

-	fprintf(stderr, "Attempting to delete a rule with multiple references!\n");

-	abort();

+    fprintf(stderr, "Attempting to delete a rule with multiple references!\n");

+    abort();

     }

     SYMBOL *left = this->prev;

     SYMBOL *right = this->next;

@@ -148,7 +148,7 @@

  */

 static void replace_digram(SYMBOL *this, SYMBOL *rule) {

     debug("Replace digram <%lu> using rule [%lu] for %d",

-	  SYMBOL_INDEX(this), SYMBOL_INDEX(rule), rule->value);

+      SYMBOL_INDEX(this), SYMBOL_INDEX(rule), rule->value);

     SYMBOL *prev = this->prev;

 

     // Delete the two nodes of the digram headed by "this", handling the removal

@@ -174,7 +174,7 @@

     // have to check the digram starting at prev->next, which is still headed by the

     // nonterminal we just inserted.

     if(!check_digram(prev)) {

-	check_digram(prev->next);

+    check_digram(prev->next);

     }

 }

 

@@ -187,46 +187,46 @@

  */

 static void process_match(SYMBOL *this, SYMBOL *match) {

     debug("Process matching digrams <%lu> and <%lu>",

-	  SYMBOL_INDEX(this), SYMBOL_INDEX(match));

+      SYMBOL_INDEX(this), SYMBOL_INDEX(match));

     SYMBOL *rule = NULL;

 

     if(IS_RULE_HEAD(match->prev) && IS_RULE_HEAD(match->next->next)) {

-	// If the digram headed by match constitutes the entire right-hand side

-	// of a rule, then we don't create any new rule.  Instead we use the

-	// existing rule to replace_digram for the newly inserted digram.

-	rule = match->prev->rule;

-	replace_digram(this, match->prev->rule);

+    // If the digram headed by match constitutes the entire right-hand side

+    // of a rule, then we don't create any new rule.  Instead we use the

+    // existing rule to replace_digram for the newly inserted digram.

+    rule = match->prev->rule;

+    replace_digram(this, match->prev->rule);

     } else {

-	// Otherwise, we create a new rule.

-	// Note that only one digram is created by this rule, and the insert_after

-	// calls will only delete digrams from the hash table, but do not insert any.

-	// In fact, no digrams will be deleted during the construction of

-	// the new rule because the calls are being made in such a way that we are

-	// never overwriting any pointers that were previously non-NULL.

-	rule = new_rule(next_nonterminal_value++);

-	add_rule(rule);

-	insert_after(rule->prev, new_symbol(this->value, this->rule));

-	insert_after(rule->prev, new_symbol(this->next->value, this->next->rule));

-

-	// Now, replace the two existing instances of the right-hand side of the

-	// rule by nonterminals that refer to the rule.

-	// Note that these will potentially cause the destruction of digrams,

-	// leading to their deletion from the hash table.

-	// They will potentially also cause the creation of digrams, due to the

-	// insertion of the nonterminal symbol.

-	// However, since the nonterminal symbol is a freshly created one that

-	// did not exist before, these replacements cannot result in the creation

-	// of digrams that duplicate already existing ones.

-	replace_digram(match, rule);

-	replace_digram(this, rule);

-

-	// Insert the right-hand side of the new rule into the digram table.

-	// Note that no other rules that might have been created as a result of the

-	// two substitutions above could have the same right-hand side as the rule

-	// we are about to insert here, because, the right-hand sides of any of these

-	// other rules must contain the new nonterminal that is at the head of the

-	// current rule but not in the body of the current rule.

-	digram_put(rule->next);

+    // Otherwise, we create a new rule.

+    // Note that only one digram is created by this rule, and the insert_after

+    // calls will only delete digrams from the hash table, but do not insert any.

+    // In fact, no digrams will be deleted during the construction of

+    // the new rule because the calls are being made in such a way that we are

+    // never overwriting any pointers that were previously non-NULL.

+    rule = new_rule(next_nonterminal_value++);

+    add_rule(rule);

+    insert_after(rule->prev, new_symbol(this->value, this->rule));

+    insert_after(rule->prev, new_symbol(this->next->value, this->next->rule));

+

+    // Now, replace the two existing instances of the right-hand side of the

+    // rule by nonterminals that refer to the rule.

+    // Note that these will potentially cause the destruction of digrams,

+    // leading to their deletion from the hash table.

+    // They will potentially also cause the creation of digrams, due to the

+    // insertion of the nonterminal symbol.

+    // However, since the nonterminal symbol is a freshly created one that

+    // did not exist before, these replacements cannot result in the creation

+    // of digrams that duplicate already existing ones.

+    replace_digram(match, rule);

+    replace_digram(this, rule);

+

+    // Insert the right-hand side of the new rule into the digram table.

+    // Note that no other rules that might have been created as a result of the

+    // two substitutions above could have the same right-hand side as the rule

+    // we are about to insert here, because, the right-hand sides of any of these

+    // other rules must contain the new nonterminal that is at the head of the

+    // current rule but not in the body of the current rule.

+    digram_put(rule->next);

     }

 

     // We have now restored the "no repeated digram" property, but it might be that

@@ -248,16 +248,16 @@

 

     SYMBOL *tocheck = rule->next->rule;  // The first symbol of the just-added rule.

     if(tocheck) {

-	debug("Checking reference count for rule [%lu] => %d",

-	      SYMBOL_INDEX(tocheck), tocheck->refcnt);

-	if(tocheck->refcnt < 2) {

-	    if(tocheck->refcnt == 0) {

-		// There is at least one reference in the just-added rule.

-		fprintf(stderr, "Reference count should not be zero!\n");

-		abort();

-	    }

-	    expand_instance(rule->next);

-	}

+    debug("Checking reference count for rule [%lu] => %d",

+          SYMBOL_INDEX(tocheck), tocheck->refcnt);

+    if(tocheck->refcnt < 2) {

+        if(tocheck->refcnt == 0) {

+        // There is at least one reference in the just-added rule.

+        fprintf(stderr, "Reference count should not be zero!\n");

+        abort();

+        }

+        expand_instance(rule->next);

+    }

     }

 }

 

@@ -280,24 +280,24 @@

     // If the "digram" is actually a single symbol at the beginning or

     // end of a rule, then there is no need to do anything.

     if(IS_RULE_HEAD(this) || IS_RULE_HEAD(this->next))

-	return 0;

+    return 0;

 

     // Otherwise, look up the digram in the digram table, to see if there is

     // a matching instance.

     SYMBOL *match = digram_get(this->value, this->next->value);

     if(match == NULL) {

         // The digram did not previously exist -- insert it now.

-	digram_put(this);

-	return 0;

+    digram_put(this);

+    return 0;

     }

 

     // If the existing digram overlaps the one we are checking, then what we have

     // is a triple, like aaa.  In this case, we do not replace it because the resulting

     // rule would only be used once.

     if(match->next == this) {

-	return 0;

+    return 0;

     } else {

-	process_match(this, match);

-	return 1;

-    }

-}

+    process_match(this, match);

+    return 1;

+    }

+}