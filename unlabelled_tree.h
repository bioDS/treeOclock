#ifndef UNLABELLEDTREE_H_
#define UNLABELLEDTREE_H_

#include "rnni.h"

// unlabelled trees use the same struct as Trees, but their node_array only
// contains num_leaves-1 internal nodes, as leaves have no labels
typedef Tree Unlabelled_Tree;
// Array of unlabelled trees
typedef Tree_Array Unlabelled_Tree_Array;

// copy source to already existing dest
void copy_unlabelled_tree(Unlabelled_Tree* dest, Unlabelled_Tree* source);
// get new copy of tree
Unlabelled_Tree* new_unlabelled_tree_copy(Unlabelled_Tree* tree);
// Create empty array of num_trees unlabelled trees on num_leaves leaves
Unlabelled_Tree_Array get_empty_unlabelled_tree_array(long num_trees,
                                                      long num_leaves);

#endif
