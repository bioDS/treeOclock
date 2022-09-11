#ifndef SPR_H_
#define SPR_H_

#include "rnni.h"
#include "queue.h"

int spr_move(Tree * input_tree, long r, long new_sibling, int child_moving);
int unlabelled_spr_move(Tree * input_tree, long r, long new_parent, int child_moving, int new_child_index);

Tree_Array all_spr_neighbourhood(Tree *input_tree, int horizontal);
Tree_Array spr_neighbourhood(Tree* input_tree);
Tree_Array hspr_neighbourhood(Tree* input_tree);

#endif
