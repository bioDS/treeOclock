#ifndef RNNI_H_
#define RNNI_H_

#include "tree.h"

/* Path: sequence of RNNI moves in matrix:
Each row moves[i] is one move
moves[i][0]:
    lower rank of interval on which move is performed
moves[i][1]:
    0 -> rank move
    1 -> nni move where children[0] moves up (becomes child of node at rank
    path[i][0]+1)
    2 -> nni move where children[1] moves up (becomes child of node at
    rank path[i][0]+1)
*/
typedef struct Path {
    long** moves;
    long length;
} Path;

// NNI move on edge [r,r+1] moving children[0] of r up to be child of r+1
int nni_move(Tree* tree, long r, int child_moves_up);
// rank move swapping ranks of nodes r and r+1
int rank_move(Tree* tree, long r);
// length moves moving all nodes between lowest_moving_node and k up by length
// move so that all those nodes are above k in returned tree
int move_up(Tree* tree, long lowest_moving_node, long k);

// returns all rnni neighbours of tree
Tree_Array rnni_neighbourhood(Tree* tree);
// returns all trees resulting from rank moves on tree
Tree_Array rank_neighbourhood(Tree* tree);
// returns one neighbour drawn uniformly from one-neighbourhood
void uniform_neighbour(Tree* input_tree);

// performs (unique) RNNI move on tree that decreases the rank of the most
// recent common ancestor of node1 and node2 by one
int decrease_mrca(Tree* tree, long node1, long node2);

// computes a Path encoding all moves done on the FindPath path from start_tree
// to dest_tree
Path findpath_moves(Tree* start_tree, Tree* dest_tree);
long rnni_distance(Tree* start_tree, Tree* dest_tree);
// Returns all trees along FindPath path from start_tree to dest_tree
Tree_Array findpath(Tree* start_tree, Tree* dest_tree);

#endif
