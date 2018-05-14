# Ternary Search Tree (tst)

A ternary search tree is a binary search tree (bst) with an additional
node pointer.

There is a relative dearth of information available about ternary trees,
and specifially, proper node-rotation when a node is deleted from the tree.
The only examples for a node-delete found are simple deletes that leave the
tree dirty by leaving a node, with or withoutsiblings, but having no valid
middle (or equal) pointer.

With a binary search tree each node contains a *left* and *right* node-pointer
so a binary choice controls traversal. Either a *greater than* or *less than*
choice. As the result of a comparison between the node-data and a reference
either the left or right node-pointer is used to further descend.

A ternary search tree adds a third (or middle) node. While you can still use
the `left-middle-right` notation, ternary trees use a `low-equal-high` pointer
verbiage. With each node holding a *key ID*, the node pointers for the this
code uses a `lokid`, `eqkid`, and `hikid` pointer naming convention. If the
difference between a refernence and the node key is negative (lower than),
the `lokid` node is traversed, if they are equal, the `eqkid` node is followed
or `hikid` is followed.

In addition to the `node->key`, the node used in this example adds a
*reference count* (a `node->refcnt`) to the node data to track the number of
occurrances for each word it holds. So for example, if using the tree to track
the words in an editor buffer (where there may be multiple occurrences of 'the'
or other common words), the node holding 'the' is not deleted, upon delete,
until no other occurrences remain (i.e. the `node->refcnt` is zero).

Each individual node has the following form:
```
                              o
                              |-key
                              |-refcnt
                  ------------+------------
                  |lokid      |eqkid      |hikid
                  o           o           o
```

The string data (a pointer to a word, or a copy of the word itself) is stored
in an additional special node following the node containing the last
character (`node->key`) in the search path for the word, cast to type
(node *) and stored as the `node->eqkid` pointer. Further, since this is
the *node after the last character*, its key is the
*nul-character* (decimal `0`) just as you would expect when ending a string.
Thus, the 'key's for each of the nodes that make up the search path of a word,
are the letters in the word with the final node having a key `0` with either
a pointer to the string (if stored in an external data structure) or an
allocated copy of the string itself if the string is to be stored in the
tree. (as in holding the words for an edit buffer, where the location/address
for the string changes with each keypress). In either case, the traversal down
nodes to the final node will have a form similar to the following for the word
"cat":
```
                              o
                              |-c
                              |-0
                  ------------+------------
                  |lokid      |eqkid      |hikid
                  o           o           o
                              |-a
                              |-0
                          ----+----
                          |   |   |    note: any of the lokid or hikid nodes
                              o              can also have pointers to nodes
                              |-t            for words that "cat" or "ca" is
                              |-0            a partial prefix to.
                          ----+----
                          |   |   |
                              o
                              |-0
                              |-1    <== the refcnt is only relevant to the final node
                          ----+----
                          |   |   |
                        NULL  o  NULL
                            "cat"
```

The ternary tree has the same O(n) efficiency for insert and search as does
a bst. The delete is only slightly worse due to the proper deletion of the
chain of unique nodes that make a word and proper rotation to eliminate the
final node containing siblings. Lookup times associated with loading the
entire `/usr/share/dict/words` file and searching range between
`0.000002 - 0.000014` sec. However, the *prefix search* ability offered by
the ternary search tree sets it apart from virtually all other data
stuctures. While Tri/Radix trees can perform as well, their memory
requirements are often 20 times more than a ternary tree.

The benefit of a ternary tree for prefix searching of text lies in its
ability to quickly traverse a tree of any size finding the node containing
the last character in the wanted prefix. An in-order traversal of that node
identifies all strings in the tree containing the prefix.
