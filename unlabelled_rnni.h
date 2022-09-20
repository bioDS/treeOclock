#ifndef UNLABELLEDRNNI_H_
#define UNLABELLEDRNNI_H_

#include "unlabelled_tree.h"

// NNI move on unlabelled tree on edge [rank, rank+1], moving the child of rank
// with index child_moves_up up, i.e. that node has rank+1 as parent after move
int unlabelled_nni_move(Unlabelled_Tree* input_tree,
                        long rank,
                        int child_moves_up);
// rank move on interval [rank, rank+1] of tree
int unlabelled_rank_move(Unlabelled_Tree* tree, long rank);

// Compute unlabelled RNNI neighbours
Unlabelled_Tree_Array unlabelled_rnni_neighbourhood(Tree* input_tree);

#endif
