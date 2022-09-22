// Basic functions for unlabelled ranked trees

#include "unlabelled_tree.h"

// copy to_copy_tree into tree
void copy_unlabelled_tree(Unlabelled_Tree* dest, Unlabelled_Tree* source) {
    long num_nodes = source->num_leaves - 1;
    for (long i = 0; i < num_nodes; i++) {
        dest->node_array[i] = source->node_array[i];
    }
}

// create new copy of tree
Unlabelled_Tree* new_unlabelled_tree_copy(Unlabelled_Tree* tree) {
    long num_leaves = tree->num_leaves;
    Unlabelled_Tree* tree_copy = get_empty_tree(num_leaves);
    copy_unlabelled_tree(tree_copy, tree);
    return tree_copy;
}

// create an empty Tree_Array
Unlabelled_Tree_Array get_empty_unlabelled_tree_array(long num_trees,
                                                      long num_leaves) {
    Unlabelled_Tree_Array tree_array;
    tree_array.num_trees = num_trees;
    tree_array.trees = calloc(num_trees, sizeof(Unlabelled_Tree));
    for (long i = 0; i < num_trees; i++) {
        tree_array.trees[i].num_leaves = num_leaves;
        tree_array.trees[i].node_array = calloc(num_leaves - 1, sizeof(Node));
    }
    return tree_array;
}
