#ifndef TREE_H_
#define TREE_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "queue.h"

typedef struct Node {
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

int get_num_digits(int integer);
Tree* deep_copy(Tree* tree);
char* tree_to_string(Tree * input_tree);

int same_topology(Tree* tree1, Tree* tree2);
int same_tree(Tree* tree1, Tree* tree2);

int nni_move(Tree * input_tree, long rank_in_list, int child_moves_up);
int rank_move(Tree * input_tree, long rank_in_list);
Tree_List rnni_neighbourhood(Tree *input_tree);
Tree_List rank_neighbourhood(Tree *input_tree);
int uniform_neighbour(Tree * input_tree);

long mrca(Tree * input_tree, long node1, long node2);
long* mrca_list(Tree* tree1, Tree* tree2);
long mrca_differences(Tree* current_tree, Tree* dest_tree, int include_leaf_parents);
int decrease_mrca(Tree* tree, long node1, long node2);

long sum_symmetric_cluster_diff(Tree* tree1, Tree* tree2);
long symmetric_cluster_diff(Tree* tree1, Tree* tree2, long k);

int move_up(Tree * itree, long i, long k);

long shortest_rank_path(Tree* tree1, Tree* tree2);
Path findpath(Tree *start_tree, Tree *dest_tree);
long findpath_distance(Tree *start_tree, Tree *dest_tree);
Tree_List return_findpath(Tree *start_tree, Tree *dest_tree);
long random_walk(Tree * tree, long k);

int first_iteration(Tree_List* treelist, long node1, long node2, long r);
long sos(Tree_List* treelist, Tree* focal_tree);


#endif
