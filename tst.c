#include "tst.h"

/** max word length to store in ternary search tree, stack size */
#define WRDMAX 128
#define STKMAX (WRDMAX * 2)

/* This macro is used to replace some repeating pattern for rotating and
 * deleting the node on ternary search tree. It will append kid of victim on
 * suitable position, then free victim itself. Note that 'kid' should be
 * victim->lokid or victim->hikid
 */
#define del_node(parent, victim, root, kid)   \
    do {                                      \
        if (!parent) {                        \
            *root = kid;                      \
        } else {                              \
            if (victim == parent->lokid)      \
                parent->lokid = kid;          \
            else if (victim == parent->hikid) \
                parent->hikid = kid;          \
            else                              \
                parent->eqkid = kid;          \
        }                                     \
        free(victim);                         \
        victim = NULL;                        \
    } while (0)


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

/** delete current data-node and parent, update 'node' to new parent.
 *  before delete the current refcnt is checked, if non-zero, occurrences
 *  of the word remain in buffer the node is not deleted, if refcnt zero,
 *  the node is deleted. if 'freeword = 1' the copy of word allocated and
 *  stored as the node->eqkid is freed, if 'freeword = 0', node->eqkid is
 *  stored elsewhere and not freed, root node updated if changed. returns
 *  NULL on success (deleted), otherwise returns the address of victim
 *  if refcnt non-zero.
 */
static void *tst_del_word(tst_node **root,
                          tst_node *node,
                          tst_stack *stk,
                          const int freeword)
{
    tst_node *victim = node;               /* begin deletion w/victim */
    tst_node *parent = tst_stack_pop(stk); /* parent to victim */

    if (!victim->refcnt) {            /* if last occurrence */
        if (!victim->key && freeword) /* check key is nul   */
            free(victim->eqkid);      /* free string (data) */

        /* remove unique suffix chain - parent & victim nodes
         * have no children. simple remove until the first parent
         * found with children.
         */
        while (!parent->lokid && !parent->hikid && !victim->lokid &&
               !victim->hikid) {
            parent->eqkid = NULL;
            free(victim);
            victim = parent;
            parent = tst_stack_pop(stk);
            if (!parent) { /* last word & root node */
                free(victim);
                return (void *) (*root = NULL);
            }
        }

        /* check if victim is prefix for others (victim has lo/hi node).
         * if both lo & hi children, check if lokid->hikid present, if not,
         * move hikid to lokid->hikid, replace node with lokid and free node.
         * if lokid->hikid present, check hikid->lokid. If not present, then
         * move lokid to hikid->lokid, replace node with hikid free node.
         */
        if (victim->lokid && victim->hikid) { /* victim has both lokid/hikid */
            if (!victim->lokid->hikid) {      /* check for hikid in lo tree */
                /* rotate victim->hikid to victim->lokid->hikid, and
                 * rotate victim->lokid to place of victim.
                 */
                victim->lokid->hikid = victim->hikid;
                del_node(parent, victim, root, victim->lokid);
            } else if (!victim->hikid->lokid) { /* check for lokid in hi tree */
                /* opposite rotation */
                victim->hikid->lokid = victim->lokid;
                del_node(parent, victim, root, victim->hikid);
            } else /* can't rotate, return, leaving victim->eqkid NULL */
                return NULL;
        } else if (victim->lokid) { /* only lokid, replace victim with lokid */
            del_node(parent, victim, root, victim->lokid);
        } else if (victim->hikid) { /* only hikid, replace victim with hikid */
            del_node(parent, victim, root, victim->hikid);
        } else { /* victim - no children, but parent has other children */
            if (victim == parent->lokid) { /* if parent->lokid - trim */
                parent->lokid = NULL;
                free(victim);
                victim = NULL;
            } else if (victim == parent->hikid) { /* if parent->hikid - trim */
                parent->hikid = NULL;
                free(victim);
                victim = NULL;
            } else { /* victim was parent->eqkid, but parent->lo/hikid exists */
                parent->eqkid = NULL;        /* set eqkid NULL */
                free(victim);                /* free current victim */
                victim = parent;             /* set parent = victim */
                parent = tst_stack_pop(stk); /* get new parent */
                /* if both victim hi/lokid are present */
                if (victim->lokid && victim->hikid) {
                    /* same checks and rotations as above */
                    if (!victim->lokid->hikid) {
                        victim->lokid->hikid = victim->hikid;
                        del_node(parent, victim, root, victim->lokid);
                    } else if (!victim->hikid->lokid) {
                        victim->hikid->lokid = victim->lokid;
                        del_node(parent, victim, root, victim->hikid);
                    } else
                        return NULL;
                }
                /* if only lokid, rewire to parent */
                else if (victim->lokid) {
                    del_node(parent, victim, root, victim->lokid);
                }
                /* if only hikid, rewire to parent */
                else if (victim->hikid) {
                    del_node(parent, victim, root, victim->hikid);
                }
            }
        }
    } else /* node->refcnt non-zero */
        printf("  %s  (refcnt: %u) not removed.\n", (char *) node->eqkid,
               node->refcnt);

    return victim; /* return NULL on successful free, *node otherwise */
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
        if (*p == 0 && curr->key == 0) {
            (*pcurr)->refcnt--;
            return tst_del_word(root, curr, &stk, cpy);
        }
        tst_stack_push(&stk, curr); /* push node on stack for del */
        pcurr = next_node(pcurr, &p);
    }
    return (void *) -1;
}

/** tst_ins() insert copy or reference of 's' from ternary search tree.
 *  insert all nodes required for 's' in tree at eqkid node of leaf.
 *  Insert 's' at node->eqkid with node->key set to the nul-character after
 *  final node in search path.
 *  If 'cpy' is non-zero allocate storage for 's', otherwise save pointer to
 *  's'. If 's' already exists in tree, increment node->refcnt. (to be used
 *  for del). returns address of 's' in tree on successful insert , NULL on
 *  allocation failure.
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
        if (*p == 0 && curr->key == 0) {
            curr->refcnt++;
            return (void *) curr->eqkid;
        }
        pcurr = next_node(pcurr, &p);
    }

    /* if not duplicate, insert remaining chars into tree rooted at curr */
    for (;;) {
        /* allocate memory for node, and fill. use calloc (or include
         * string.h and initialize w/memset) to avoid valgrind warning
         * "Conditional jump or move depends on uninitialised value(s)"
         */
        if (!(*pcurr = calloc(1, sizeof **pcurr))) {
            fprintf(stderr, "error: tst_insert(), memory exhausted.\n");
            return NULL;
        }
        curr = *pcurr;
        curr->key = *p;
        curr->refcnt = 1;
        curr->lokid = curr->hikid = curr->eqkid = NULL;

        /* Place nodes until end of the string, at end of stign allocate
         * space for data, copy data as final eqkid, and return.
         */
        if (*p++ == 0) {
            if (cpy) { /* allocate storage for 's' */
                const char *eqdata = strdup(s);
                if (!eqdata)
                    return NULL;
                curr->eqkid = (tst_node *) eqdata;
                return (void *) eqdata;
            } else { /* save pointer to 's' (allocated elsewhere) */
                curr->eqkid = (tst_node *) s;
                return (void *) s;
            }
        }
        pcurr = &(curr->eqkid);
    }
}

/** tst_search(), non-recursive find of a string internary tree.
 *  returns pointer to 's' on success, NULL otherwise.
 */
void *tst_search(const tst_node *p, const char *s)
{
    const tst_node *curr = p;

    while (curr) {                 /* loop over each char in 's' */
        int diff = *s - curr->key; /* calculate the difference */
        if (diff == 0) {           /* handle the equal case */
            if (*s == 0)           /* if *s = curr->key = nul-char, 's' found */
                return (void *) curr->eqkid; /* return pointer to 's' */
            s++;
            curr = curr->eqkid;
        } else if (diff < 0) /* handle the less than case */
            curr = curr->lokid;
        else
            curr = curr->hikid; /* handle the greater than case */
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
    if (!p->key)
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
