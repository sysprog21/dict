#ifndef PREFIX_SEARCH_H
#define PREFIX_SEARCH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* forward declaration of ternary search tree */
typedef struct tst_node tst_node;

/** tst_del() del copy or reference of 's' from ternary search tree.
 *  If 's' already exists in tree, decrement node->refcnt.
 *  If node->refcnt is zero after decrement, remove assoshiated nodes.
 *  If 'cpy' is non-zero, free the allocated space of string.
 *  Returns the address of 's' in tree on delete if refcnt non-zero,
 *  -1 on 's' not found in ternary search tree,
 *  otherwise returns NULL.
 */
void *tst_del(tst_node **root, const char *s, const int cpy);

/** tst_ins() inserts copy or reference of a string (pointed to by 's') into
 *  ternary search tree (TST).
 *  If the string already exists in the tree, increment 'node->refcnt', which is
 *  used for deletion. Otherwise, all nodes required for the string are inserted
 *  into 'eqkid' of the leaf node in the TST, one after the other (including the
 *  null character). When all insertions are done, assign the pointer to string
 *  to 'eqkid' of the current node.
 *  1. Copy: If 'cpy' is non-zero, duplicate the string and assign the address
 *  of the duplicate to 'eqkid'.
 *  2. Reference: If 'cpy' is zero, assign 's' to 'eqkid'. Note that 's' points
 *  to the string, which is previously stored in the memory pool.
 *
 *  Return value:
 *  tst_ins() returns a pointer to the string on successful insertion, and
 *  returns NULL on allocation failure.
 */
void *tst_ins(tst_node **root, const char *s, const int cpy);

/** tst_search() finds a given string in the ternary tree, non recursively.
 *  Returns a pointer to the string on success, and NULL otherwise.
 */
void *tst_search(const tst_node *p, const char *s);

/** tst_search_prefix() fills ptr array 'a' with words prefixed with 's'.
 *  once the node containing the first prefix matching 's' is found
 *  tst_suggest() is called to traverse the ternary_tree beginning
 *  at the node filling 'a' with pointers to all words that contain
 *  the prefix upto 'max' words updating 'n' with the number of words
 *  in 'a'. a pointer to the first node is returned on success
 *  NULL otherwise.
 */
void *tst_search_prefix(const tst_node *root,
                        const char *s,
                        char **a,
                        int *n,
                        const int max);

/** tst_traverse_fn(), traverse tree calling 'fn' on each word.
 *  prototype for 'fn' is void fn(const void *, void *). data can
 *  be NULL if unused.
 *
 *  The callback can be implemented as following:
 *
 *  // print each word.
 *  void print_word(const void *node, void *data) {
 *      printf("%s\n", tst_get_string(node));
 *  }
 *
 * Then, invoke as "tst_traverse_fn (root, print_word, NULL);"
 */
void tst_traverse_fn(const tst_node *p,
                     void(fn)(const void *, void *),
                     void *data);

/** free the ternary search tree rooted at p, data storage internal. */
void tst_free_all(tst_node *p);

/** free the ternary search tree rooted at p, data storage external. */
void tst_free(tst_node *p);

/** access functions tst_get_key(), tst_get_refcnt, & tst_get_string().
 *  provide access to struct members through opague pointers availale
 *  to program.
 */
char tst_get_key(const tst_node *node);
unsigned tst_get_refcnt(const tst_node *node);
char *tst_get_string(const tst_node *node);

#endif
