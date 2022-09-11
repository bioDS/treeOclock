#ifndef UNLABELLEDRNNI_H_
#define UNLABELLEDRNNI_H_

#include "unlabelled_tree.h"


int unlabelled_nni_move(Unlabelled_Tree * input_tree, long rank, int child_moves_up);
int unlabelled_rank_move(Unlabelled_Tree * tree, long rank);

Unlabelled_Tree_Array unlabelled_rnni_neighbourhood(Tree *input_tree);


#endif
