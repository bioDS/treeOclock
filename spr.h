#ifndef SPR_H_
#define SPR_H_

#include "rnni.h"
#include "queue.h"

int spr_move(Tree * input_tree, long r, long new_sibling, int child_moving);
int unlabelled_spr_move(Tree * input_tree, long r, long new_parent, int child_moving, int new_child_index);

Tree_List all_spr_neighbourhood(Tree *input_tree, int horizontal);
Tree_List spr_neighbourhood(Tree* input_tree);
Tree_List hspr_neighbourhood(Tree* input_tree);
Tree_List unlabelled_spr_neighbourhood(Tree* input_tree, int horizontal);

#endif
