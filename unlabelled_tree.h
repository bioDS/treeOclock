#ifndef UNLABELLEDTREE_H_
#define UNLABELLEDTREE_H_

#include "rnni.h"


typedef Tree Unlabelled_Tree;
typedef Tree_Array Unlabelled_Tree_Array;


void copy_unlabelled_tree(Tree* tree, Tree* to_copy_tree);
Unlabelled_Tree* new_unlabelled_tree_copy(Tree* tree);
Unlabelled_Tree_Array empty_unlabelled_tree_array(long num_trees, long num_leaves);

#endif
