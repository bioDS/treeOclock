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

typedef struct Node {
    long parent;
    long children[2];
    long time;
} Node;

typedef struct Tree {
    Node* node_array;
    long num_leaves;
    long root_time;
    long sos_d;
} Tree;

typedef struct Tree_Array {
    Tree* trees;
    long num_trees;
    long max_root_time;
} Tree_Array;

Node empty_node();

Tree* empty_tree(long num_leaves);
void free_tree(Tree* tree);

void copy_tree(Tree* tree, Tree* to_copy_tree);
Tree* new_tree_copy(Tree* tree);

Tree_Array empty_tree_array(long num_trees, long num_leaves);
void free_tree_array(Tree_Array tree_array);

void print_tree(Tree* tree);

int same_topology(Tree* tree1, Tree* tree2);
int same_tree(Tree* tree1, Tree* tree2);

long mrca(Tree* input_tree, long node1, long node2);

#endif
