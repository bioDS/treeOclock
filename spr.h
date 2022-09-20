#ifndef SPR_H_
#define SPR_H_

#include "queue.h"
#include "rnni.h"

// SPR move on ranked tree: prune subtree rooted in the child of the node r with
// index child_moving (0 or 1); gets re-attached at rank r to become sibling of
// the node new_sibling
int spr_move(Tree* tree, long r, long new_sibling, int child_moving);

// SPR moves on unlabelled tree: pruning the child with index child_moving of
// node r; re-attaching as children[new_child] of tree at position new_parent of
// node_array
int unlabelled_spr_move(Tree* tree,
                        long r,
                        long new_parent,
                        int child_moving,
                        int new_child_index);

// Return all spr neighbours in array; if horizontal = FALSE (0), then give RSPR
// neighbourhood (including rank moves), otherwise HSPR (only SPR moves)
Tree_Array all_spr_neighbourhood(Tree* tree, int horizontal);
Tree_Array spr_neighbourhood(Tree* tree);
Tree_Array hspr_neighbourhood(Tree* tree);

#endif
