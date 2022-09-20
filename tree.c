/*Basic functions for ranked trees*/

#include "tree.h"

// create empty node
Node get_empty_node() {
    Node new_node;
    new_node.parent = -1;
    new_node.children[0] = -1;
    new_node.children[1] = -1;
    new_node.time = -1;
    return new_node;
}

// create empty tree on num_leaves leaves
Tree* get_empty_tree(long num_leaves) {
    long num_nodes = 2 * num_leaves - 1;
    Tree* new_tree = malloc(sizeof(Tree));
    new_tree->node_array = calloc(num_nodes, sizeof(Node));
    new_tree->num_leaves = num_leaves;
    for (long i = 0; i < num_nodes; i++) {
        new_tree->node_array[i] = get_empty_node();
    }
    return new_tree;
}

// free memory
void free_tree(Tree* tree) {
    free(tree->node_array);
    free(tree);
}

// copy source_tree to dest_tree
void copy_tree(Tree* dest_tree, Tree* source_tree) {
    long num_nodes = 2 * source_tree->num_leaves - 1;
    for (long i = 0; i < num_nodes; i++) {
        dest_tree->node_array[i] = source_tree->node_array[i];
    }
}

// create new copy of tree
Tree* new_tree_copy(Tree* tree) {
    long num_leaves = tree->num_leaves;
    Tree* tree_copy = get_empty_tree(num_leaves);
    copy_tree(tree_copy, tree);
    return tree_copy;
}

// create an empty Tree_Array
Tree_Array get_empty_tree_array(long num_trees, long num_leaves) {
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

// free memory
void free_tree_array(Tree_Array tree_array) {
    for (long i = 0; i < tree_array.num_trees; i++) {
        free(tree_array.trees[i].node_array);
    }
    free(tree_array.trees);
}

// print parents, children, and time for each node in tree
void print_tree(Tree* tree) {
    long num_leaves = tree->num_leaves;
    long num_nodes = 2 * num_leaves - 1;
    for (long pos = 0; pos < num_nodes; pos++) {
        printf(
            "Node at position %ld: parent %ld, children %ld, %ld, time %ld\n",
            pos, tree->node_array[pos].parent,
            tree->node_array[pos].children[0],
            tree->node_array[pos].children[1], tree->node_array[pos].time);
    }
}

// Check whether two trees have the same (ranked) tree topology (unlabelled
// ranked tree)
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

// Check whether two trees are isomorphic
int same_tree(Tree* tree1, Tree* tree2) {
    long num_nodes = 2 * tree1->num_leaves - 1;
    for (long i = 0; i < num_nodes; i++) {
        if (tree1->node_array[i].parent != tree2->node_array[i].parent) {
            return FALSE;
        }
    }
    return TRUE;
}

// find rank (position in node_array) of most recent common ancestor of nodes
// node1 and node2 in tree
long mrca(Tree* input_tree, long node1, long node2) {
    long rank1 = node1;
    long rank2 = node2;
    // loop through ancestors (bottom-up) of the two nodes until ancestor of
    // both is found
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
