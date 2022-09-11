#ifndef RNNI_H_
#define RNNI_H_

# include "tree.h"


typedef struct Path{
  long ** moves;
  long length;
} Path;

int nni_move(Tree * input_tree, long rank_in_list, int child_moves_up);
int rank_move(Tree * input_tree, long rank_in_list);
int move_up(Tree * itree, long i, long k);

Tree_Array rnni_neighbourhood(Tree *input_tree);
Tree_Array rank_neighbourhood(Tree *input_tree);
int uniform_neighbour(Tree * input_tree);

int decrease_mrca(Tree* tree, long node1, long node2);

long shortest_rank_path(Tree* tree1, Tree* tree2);
Path findpath(Tree *start_tree, Tree *dest_tree);
long rnni_distance(Tree *start_tree, Tree *dest_tree);
Tree_Array return_findpath(Tree *start_tree, Tree *dest_tree);
long random_walk(Tree * tree, long k);

int first_iteration(Tree_Array* treelist, long node1, long node2, long r);
long sos(Tree_Array* treelist, Tree* focal_tree);

#endif
