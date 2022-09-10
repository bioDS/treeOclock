#ifndef UNLABELLEDRNNI_H_
#define UNLABELLEDRNNI_H_

#include "rnni.h"


typedef Tree Unlabelled_Tree;

typedef Tree_List Unlabelled_Tree_Array;

Tree_List unlabelled_rnni_neighbourhood(Tree *input_tree);

int unlabelled_nni_move(Unlabelled_Tree * input_tree, long rank, int child_moves_up);



#endif
