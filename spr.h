#ifndef SPR_H_
#define SPR_H_

#include "tree.h"
#include "queue.h"

int spr_move(Tree * input_tree, long r, long new_sibling, int child_moving);

Tree_List all_spr_neighbourhood(Tree *input_tree, int horizontal);
Tree_List spr_neighbourhood(Tree* input_tree);
Tree_List hspr_neighbourhood(Tree* input_tree);

Tree_List rankedspr_path_mrca_cluster_diff(Tree* start_tree, Tree* dest_tree, int hspr);
Tree_List rankedspr_path_mrca_diff(Tree* start_tree, Tree* dest_tree, int hspr);
Tree_List rankedspr_path_rnni_mrca_diff(Tree* start_tree, Tree* dest_tree, int rank);
long rankedspr_path_restricting_neighbourhood(Tree* start_tree, Tree* dest_tree, int hspr);
long rankedspr_path_top_down_symm_diff(Tree* start_tree, Tree* dest_tree);
Tree_List rankedspr_path_bottom_up_hspr(Tree *start_tree, Tree *dest_tree);
long rankedspr_path_bottom_up_hspr_dist(Tree *start_tree, Tree *dest_tree);
long fp_rspr(Tree* tree1, Tree* tree2);

#endif
