#ifndef EXPLORINGRNNI_H_
#define EXPLORINGRNNI_H_

#include "rnni.h"

long random_walk(Tree* tree, long k);
int first_iteration_fp(Tree_Array* tree_array, long node1, long node2, long r);
long sos(Tree_Array* tree_array, Tree* focal_tree);

long* mrca_array(Tree* tree1, Tree* tree2);
long mrca_differences(Tree* current_tree,
                      Tree* dest_tree,
                      int include_leaf_parents);

long** get_clusters(Tree* tree);
long sum_symmetric_cluster_diff(Tree* tree1, Tree* tree2);
long symmetric_cluster_diff(Tree* tree1, Tree* tree2, long k);

#endif
