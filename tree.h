#ifndef TREE_H_
#define TREE_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "queue.h"

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define TRUE 1
#define FALSE 0

// A node has one parent (except for root) and two children (except for leaves)
// Time is 0 for leaves and unique long > 0 for internal nodes; for ranked trees
// times are ranging from 1 to n-1
typedef struct Node {
    long parent;
    long children[2];
    long time;
} Node;

// node_array has length 2 * num_leaves - 1
// first num_leaves elements are leaves
// last num_leaves - 1 elements are internal nodes
typedef struct Tree {
    Node* node_array;
    long num_leaves;
} Tree;

// array of num_trees trees
typedef struct Tree_Array {
    Tree* trees;
    long num_trees;
} Tree_Array;

Node get_empty_node();

Tree* get_empty_tree(long num_leaves);
void free_tree(Tree* tree);

// copy source_tree to dest_tree
void copy_tree(Tree* dest_tree, Tree* source_tree);
Tree* new_tree_copy(Tree* tree);

Tree_Array get_empty_tree_array(long num_trees, long num_leaves);
void free_tree_array(Tree_Array tree_array);

// print parent, children, and time for all nodes in tree
void print_tree(Tree* tree);

// check if topologies (unlabelled ranked trees) are isomorphic
int same_topology(Tree* tree1, Tree* tree2);
// check if trees are identical
int same_tree(Tree* tree1, Tree* tree2);

// return rank of most recent common ancestor (lowest/least common ancestor) of
// nodes node1 and node2 in input_tree
long mrca(Tree* input_tree, long node1, long node2);

#endif
