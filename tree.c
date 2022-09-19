/*Basic functions for ranked trees*/

#include "tree.h"

// create empty node
Node empty_node() {
    Node new_node;
    new_node.parent = -1;
    new_node.children[0] = -1;
    new_node.children[1] = -1;
    new_node.time = -1;
    return new_node;
}

// create empty tree on num_leaves leaves
Tree* empty_tree(long num_leaves) {
    long num_nodes = 2 * num_leaves - 1;
    Tree* new_tree = malloc(sizeof(Tree));
    new_tree->node_array = calloc(num_nodes, sizeof(Node));
    new_tree->num_leaves = num_leaves;
    for (long i = 0; i < num_nodes; i++) {
        new_tree->node_array[i] = empty_node();
    }
    return new_tree;
}

void free_tree(Tree* tree) {
    free(tree->node_array);
    free(tree);
}

// copy to_copy_tree into tree
void copy_tree(Tree* tree, Tree* to_copy_tree) {
    long num_nodes = 2 * tree->num_leaves - 1;
    for (long i = 0; i < num_nodes; i++) {
        tree->node_array[i] = to_copy_tree->node_array[i];
    }
}

// create new copy of tree
Tree* new_tree_copy(Tree* tree) {
    long num_leaves = tree->num_leaves;
    Tree* tree_copy = empty_tree(num_leaves);
    copy_tree(tree_copy, tree);
    return tree_copy;
}

// create an empty Tree_Array
Tree_Array empty_tree_array(long num_trees, long num_leaves) {
    Tree_Array tree_array;
    tree_array.num_trees = num_trees;
    tree_array.trees = calloc(num_trees, sizeof(Tree));
    for (long i = 0; i < num_trees; i++) {
        tree_array.trees[i].num_leaves = num_leaves;
        tree_array.trees[i].node_array =
            calloc(2 * num_leaves - 1, sizeof(Node));
    }
    return tree_array;
}

void free_tree_array(Tree_Array tree_array) {
    for (long i = 0; i < tree_array.num_trees; i++) {
        free(tree_array.trees[i].node_array);
    }
    free(tree_array.trees);
}

// print parents and children for each node in tree
void print_tree(Tree* tree) {
    long num_leaves = tree->num_leaves;
    long num_nodes = 2 * num_leaves - 1;
    for (long rank = 0; rank < num_nodes; rank++) {
        printf("Node at rank %ld: parent %ld, children %ld, %ld\n", rank,
               tree->node_array[rank].parent,
               tree->node_array[rank].children[0],
               tree->node_array[rank].children[1]);
    }
}

// Check whether two trees have the same (ranked) tree topology
int same_topology(Tree* tree1, Tree* tree2) {
    long num_leaves = tree1->num_leaves;
    long num_nodes = 2 * num_leaves - 1;
    for (long i = num_leaves; i < num_nodes; i++) {
        if (tree1->node_array[i].parent != tree2->node_array[i].parent) {
            return FALSE;
        }
    }
    return TRUE;
}

// Check whether two trees are identical
int same_tree(Tree* tree1, Tree* tree2) {
    long num_nodes = 2 * tree1->num_leaves - 1;
    for (long i = 0; i < num_nodes; i++) {
        if (tree1->node_array[i].parent != tree2->node_array[i].parent) {
            return FALSE;
        }
    }
    return TRUE;
}

// find rank of mrca of nodes with positions node1 and node2 in tree
long mrca(Tree* input_tree, long node1, long node2) {
    long rank1 = node1;
    long rank2 = node2;
    while (rank1 != rank2) {
        if (rank1 < rank2) {
            rank1 = input_tree->node_array[rank1].parent;
            if (rank1 == -1) {
                printf("Cannot find mrca, reached root.");
                return -1;
            }
        } else {
            rank2 = input_tree->node_array[rank2].parent;
            if (rank2 == -1) {
                printf("Cannot find mrca, reached root.");
                return -1;
            }
        }
    }
    return rank1;
}
