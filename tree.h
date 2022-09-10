#ifndef TREE_H_
#define TREE_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "queue.h"

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define TRUE 1
#define FALSE 0

typedef struct Node{
  long parent;
  long children[2];
  long time;
} Node;

typedef struct Tree{
  Node * tree;
  long num_leaves;
  long root_time;
  long sos_d;
} Tree;

typedef struct Tree_List{
  Tree * trees;
  long num_trees;
  long max_root_time;
} Tree_List;

typedef struct Path{
  long ** moves;
  long length;
} Path;


Node empty_node();
Tree* empty_tree(long num_leaves);
Tree* deep_copy(Tree* tree);

int same_topology(Tree* tree1, Tree* tree2);
int same_tree(Tree* tree1, Tree* tree2);

long mrca(Tree * input_tree, long node1, long node2);
long* mrca_list(Tree* tree1, Tree* tree2);
long mrca_differences(Tree* current_tree, Tree* dest_tree, int include_leaf_parents);

long sum_symmetric_cluster_diff(Tree* tree1, Tree* tree2);
long symmetric_cluster_diff(Tree* tree1, Tree* tree2, long k);

#endif
