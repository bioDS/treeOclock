#ifndef RNNI_H_
#define RNNI_H_

#include "tree.h"

typedef struct Path {
    long** moves;
    long length;
} Path;

int nni_move(Tree* input_tree, long r, int child_moves_up);
int rank_move(Tree* input_tree, long r);
int move_up(Tree* itree, long i, long k);

Tree_Array rnni_neighbourhood(Tree* input_tree);
Tree_Array rank_neighbourhood(Tree* input_tree);
int uniform_neighbour(Tree* input_tree);

int decrease_mrca(Tree* tree, long node1, long node2);

Path findpath_moves(Tree* start_tree, Tree* dest_tree);
long rnni_distance(Tree* start_tree, Tree* dest_tree);
Tree_Array findpath(Tree* start_tree, Tree* dest_tree);

#endif
