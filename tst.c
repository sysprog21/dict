#include "tst.h"

/** max word length to store in ternary search tree, stack size */
#define WRDMAX 128
#define STKMAX (WRDMAX * 2)

/** ternary search tree node. */
typedef struct tst_node {
    char key;               /* char key for node (null for node with string) */
    unsigned refcnt;        /* refcnt tracks occurrence of word (for delete) */
    struct tst_node *lokid; /* ternary low child pointer */
    struct tst_node *eqkid; /* ternary equal child pointer */
    struct tst_node *hikid; /* ternary high child pointer */
} tst_node;

/** struct to use for static stack to remove nodes. */
typedef struct tst_stack {
    void *data[STKMAX];
    size_t idx;
} tst_stack;

/** stack push/pop to store node pointers to delete word from tree.
 *  on delete, store all nodes from root to leaf containing word to
 *  allow word removal and reordering of tree.
 */
static void *tst_stack_push(tst_stack *s, void *node)
{
    if (s->idx >= STKMAX)
        return NULL;

    return (s->data[(s->idx)++] = node);
}

static void *tst_stack_pop(tst_stack *s)
{
    if (!s->idx)
        return NULL;

    void *node = s->data[--(s->idx)];
    s->data[s->idx] = NULL;

    return node;
}

/** delete non-referenced nodes from the stack, update 'node' to new parent.
 *  before delete the current refcnt is checked, if non-zero, occurrences
 *  of the word remain in buffer the node is not deleted, if refcnt zero,
 *  the node is deleted. if 'freeword = 1' the copy of word allocated and
 *  stored as the node->eqkid is freed, if 'freeword = 0', node->eqkid is
 *  stored elsewhere and not freed, root node updated if changed. returns
 *  NULL on success (deleted), otherwise returns the address of victim
 *  if refcnt non-zero.
 */
static void *tst_del_word(tst_stack *stk, const int freeword)
{
    tst_node **pvictim = tst_stack_pop(stk);
    tst_node *victim = *pvictim;

    if (victim->refcnt > 0) {
        /* The string is still be referenced.
         * Return the address of victim.
         */
        printf("  %s  (refcnt: %u) not removed.\n", (char *) victim->eqkid,
               victim->refcnt);
        return victim;
    }

    if (freeword) /* Free the string in CPY mode. */
        free(victim->eqkid);
    victim->eqkid = NULL;

    /* Remove unique suffix chain - victim have no children.
     * Simply remove until the first node found with children.
     */
    while (!victim->lokid && !victim->hikid && !victim->eqkid) {
        free(victim);
        *pvictim = NULL;
        pvictim = tst_stack_pop(stk);
        if (!pvictim) {
            /* Stack empty, which means reached the root of the tree. */
            return NULL;
        }
        victim = *pvictim;
    }

    /* If 'victim->qekid' isn't NULL, 'victim' represent a node of another
     * prefix string. In this case, the delete process is done and return NULL;
     */
    if (victim->eqkid)
        return NULL;

    /* If 'victim->eqkid' is NULL, 'victim' is a prefix node of deleting string.
     * The prefix alse referencing by other strings. Try to rotate victim's
     * subtrees for maintaining those strings.
     */
    if (victim->lokid && victim->hikid) {
        /* If both 'lokid' and 'hikid' are exist, try to rotate one
         * to be another's kid.
         * Because of 'victim->lokid->key' alway lower than
         * 'victim->lokid->key', the subtree can be rotated without comparison,
         * vice versa.
         * The only thing need to be aware of is the destination of the rotation
         * should have no subtree otherwise the rotation isn't available.
         * If both of the rotation are not available, the delete process is done
         * and left a node with no 'eqkid'.
         */
        if (!victim->lokid->hikid) {
            victim->lokid->hikid = victim->hikid;
            *pvictim = victim->lokid;
        } else if (!victim->hikid->lokid) {
            victim->hikid->lokid = victim->lokid;
            *pvictim = victim->hikid;
        } else /* The subtrees are non-rotatable. */
            return NULL;
    } else if (victim->lokid) {
        *pvictim = victim->lokid;
    } else if (victim->hikid) {
        *pvictim = victim->hikid;
    }

    free(victim);
    return NULL;
}

/** next_node()find out the next node based on the char from 's' for searching
 *  given string from ternary search tree. If char equal the key of current
 *  node, move forward the char ptr which 's' is pointing, for preparing
 *  next comparison. Return a ptr to ptr to next node.
 */
static tst_node **next_node(tst_node **root, const char **s)
{
    int diff = **s - (*root)->key;
    if (diff == 0) {
        (*s)++;
        root = &(*root)->eqkid;
    } else if (diff < 0)
        root = &(*root)->lokid;
    else
        root = &(*root)->hikid;
    return root;
}

/** tst_del() del copy or reference of 's' from ternary search tree.
 *  If 's' already exists in tree, decrement node->refcnt.
 *  If node->refcnt is zero after decrement, remove assoshiated nodes.
 *  If 'cpy' is non-zero, free the allocated space of string.
 *  Returns the address of 's' in tree on delete if refcnt non-zero,
 *  -1 on 's' not found in ternary search tree,
 *  otherwise returns NULL.
 */
void *tst_del(tst_node **root, const char *s, const int cpy)
{
    const char *p = s;
    tst_stack stk = {.data = {NULL}, .idx = 0};
    tst_node *curr, **pcurr;

    if (!root || !s)
        return NULL;                /* validate parameters */
    if (strlen(s) + 1 > STKMAX / 2) /* limit length to 1/2 STKMAX */
        return NULL;                /* 128 char word length is plenty */

    pcurr = root;
    while ((curr = *pcurr)) {
        tst_stack_push(&stk, pcurr); /* push ptr to node on stack for del */
        if (*p == 0 && curr->key == 0) {
            (*pcurr)->refcnt--;
            return tst_del_word(&stk, cpy);
        }
        pcurr = next_node(pcurr, &p);
    }
    return (void *) -1;
}

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
void *tst_ins(tst_node **root, const char *s, const int cpy)
{
    const char *p = s;
    tst_node *curr, **pcurr;

    if (!root || !s)
        return NULL;                /* validate parameters */
    if (strlen(s) + 1 > STKMAX / 2) /* limit length to 1/2 STKMAX */
        return NULL;                /* 128 char word length is plenty */

    pcurr = root;
    while ((curr = *pcurr)) {
        /* Returns a pointer to the string if it is already in the ternary
         * search tree (TST). */
        if (*p == '\0' && curr->key == '\0') {
            curr->refcnt++;
            return (void *) curr->eqkid; /* pointer to the string */
        }
        pcurr = next_node(pcurr, &p);
    }

    /* If 's' is not in the TST, insert the remaining chars into the tree with
     * root node 'curr'. */
    for (;;) {
        /* allocate memory for node, and fill. use calloc (or include
         * string.h and initialize w/memset) to avoid valgrind warning
         * "Conditional jump or move depends on uninitialised value(s)"
         */
        if (!(*pcurr = calloc(1, sizeof **pcurr))) {
            fprintf(stderr, "error: tst_insert(), memory exhausted.\n");
            return NULL;
        }

        /* Initializes the new node */
        curr = *pcurr;
        curr->key = *p;
        curr->refcnt = 1;
        curr->lokid = curr->hikid = curr->eqkid = NULL;

        /* When the end of the string is reached, assign the pointer to the
         * string to 'curr->eqkid'. */
        if (*p++ == '\0') {
            if (cpy) { /* allocate storage space for 's' */
                const char *eqdata = strdup(s);
                if (!eqdata)
                    return NULL;
                curr->eqkid = (tst_node *) eqdata;
                return (void *) eqdata;
            } else { /* the string is pointed to by 's' and is already stored in
                        a memory pool */
                curr->eqkid = (tst_node *) s;
                return (void *) s;
            }
        }
        pcurr = &(curr->eqkid);
    }
}

/** tst_search() finds a given string in the ternary tree, non recursively.
 *  Returns a pointer to the string on success, and NULL otherwise.
 */
void *tst_search(const tst_node *p, const char *s)
{
    const tst_node *curr = p;

    /* Loops over each character in 's' */
    while (curr) {
        int diff = *s - curr->key;
        if (diff == 0) {
            /* Found identical string */
            if (*s == '\0')
                return (void *) curr->eqkid; /* pointer to the string */
            s++;
            curr = curr->eqkid;
        } else if (diff < 0)
            curr = curr->lokid;
        else
            curr = curr->hikid;
    }
    return NULL;
}

/** fill ptr array 'a' with strings matching prefix at node 'p'.
 *  the 'a' array will hold pointers to stored strings with prefix
 *  matching the string passed to tst_matching, ending in 'c', the
 *  nchr'th char in in each matched string.
 */
void tst_suggest(const tst_node *p,
                 const char c,
                 const size_t nchr,
                 char **a,
                 int *n,
                 const int max)
{
    if (!p || *n >= max)
        return;
    tst_suggest(p->lokid, c, nchr, a, n, max);
    if (p->key)
        tst_suggest(p->eqkid, c, nchr, a, n, max);
    else if (*(((char *) p->eqkid) + nchr - 1) == c && *n < max)
        a[(*n)++] = (char *) p->eqkid;
    tst_suggest(p->hikid, c, nchr, a, n, max);
}

/** tst_search_prefix fills ptr array 'a' with words prefixed with 's'.
 *  once the node containing the first prefix matching 's' is found
 *  tst_suggest is called to travers the ternary_tree beginning
 *  at the node filling 'a' with pointers to all words that contain
 *  the prefix upto 'max' words updating 'n' with the number of word
 *  in 'a'. a pointer to the first node is returned on success
 *  NULL otherwise.
 */
void *tst_search_prefix(const tst_node *root,
                        const char *s,
                        char **a,
                        int *n,
                        const int max)
{
    const tst_node *curr = root;
    const char *start = s;

    if (!*s)
        return NULL;

    /* get length of s */
    for (; *start; start++)
        ; /* wait */
    const size_t nchr = start - s;

    start = s; /* reset start to s */
    *n = 0;    /* initialize n - 0 */

    /* Loop while we haven't hit a NULL node or returned */
    while (curr) {
        int diff = *s - curr->key; /* calculate the difference */
        if (diff == 0) {           /* handle the equal case */
            /* check if prefix number of chars reached */
            if ((size_t)(s - start) == nchr - 1) {
                /* call tst_suggest to fill a with pointer to matching words */
                tst_suggest(curr, curr->key, nchr, a, n, max);
                return (void *) curr;
            }
            if (*s == 0) /* no matching prefix found in tree */
                return (void *) curr->eqkid;

            s++;
            curr = curr->eqkid;
        } else if (diff < 0) /* handle the less than case */
            curr = curr->lokid;
        else
            curr = curr->hikid; /* handle the greater than case */
    }
    return NULL;
}

/** tst_traverse_fn(), traverse tree calling 'fn' on each word.
 *  prototype fonr 'fn' is void fn(const void *, void *). data can
 *  be NULL if unused.
 */
void tst_traverse_fn(const tst_node *p,
                     void(fn)(const void *, void *),
                     void *data)
{
    if (!p)
        return;
    tst_traverse_fn(p->lokid, fn, data);
    if (p->key)
        tst_traverse_fn(p->eqkid, fn, data);
    else
        fn(p, data);
    tst_traverse_fn(p->hikid, fn, data);
}

/** free the ternary search tree rooted at p, data storage internal. */
void tst_free_all(tst_node *p)
{
    if (!p)
        return;
    tst_free_all(p->lokid);
    if (p->key)
        tst_free_all(p->eqkid);
    tst_free_all(p->hikid);
    if (!p->key && p->refcnt > 0)
        free(p->eqkid);
    free(p);
}

/** free the ternary search tree rooted at p, data storage external. */
void tst_free(tst_node *p)
{
    if (!p)
        return;
    tst_free(p->lokid);
    if (p->key)
        tst_free(p->eqkid);
    tst_free(p->hikid);
    free(p);
}

/** access functions tst_get_key(), tst_get_refcnt, & tst_get_string().
 *  provide access to struct members through opaque pointers availale
 *  to program.
 */
char tst_get_key(const tst_node *node)
{
    return node->key;
}

unsigned tst_get_refcnt(const tst_node *node)
{
    return node->refcnt;
}

char *tst_get_string(const tst_node *node)
{
    if (node && !node->key)
        return (char *) node->eqkid;

    return NULL;
}
