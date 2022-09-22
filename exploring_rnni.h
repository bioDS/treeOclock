#ifndef EXPLORINGRNNI_H_
#define EXPLORINGRNNI_H_

#include "rnni.h"

// Random walk of length k starting at `tree` choosing neighbour uniformly in
// every step
long random_walk(Tree* tree, long k);
// updates every tree in tree_array to the tree after one iteration of findpath:
// decreases the mrca of node1 and node2 in every tree until it has rank r
int first_iteration_fp(Tree_Array* tree_array, long node1, long node2, long r);
// sum of squared distances of all trees in tree_array to focal_tree
long sos(Tree_Array* tree_array, Tree* focal_tree);

// returns array with rank(mrca_{tree1}(C_i)) at position i where C_i is the
// cluster induced by node of rank i in tree2
long* mrca_array(Tree* tree1, Tree* tree2);
// Sum of differences of ranks of mrcas of all clusters of tree2 between tree1
// and tree2.
// Also add differences for parents of leaves if include_leaf_parents == 1
long mrca_differences(Tree* current_tree,
                      Tree* dest_tree,
                      int include_leaf_parents);

// return binary matrix with rows representing clusters, columns leaves:
// 0 if leaf is not in cluster, 1 if it is in cluster
long** get_clusters(Tree* tree);

// Computes sum of sizes of symmetric differences of clusters of tree1 and tree2
// for all ranks i=1,..,n-1
long sum_symmetric_cluster_diff(Tree* tree1, Tree* tree2);

// Compute symmetric difference of clusters induced by nodes of rank k in tree1
// and tree2
long symmetric_cluster_diff(Tree* tree1, Tree* tree2, long k);

#endif
