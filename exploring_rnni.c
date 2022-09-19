// Testing some ideas to do with exploring distributions in RNNI space

#include "exploring_rnni.h"

// Perform a series of k random RNNI moves to receive a random walk in RNNI,
// starting at input tree
long random_walk(Tree* tree, long k) {
    Tree* current_tree = new_tree_copy(tree);
    for (long i = 0; i < k; i++) {
        uniform_neighbour(current_tree);
    }
    long distance = rnni_distance(current_tree, tree);
    free_tree(current_tree);
    return (distance);
}

// perform one iteration of FP on every tree in tree_array, such that resulting
// tree has mrca of i and j at position r Note that this may change every tree
// in treearray
int first_iteration_fp(Tree_Array* tree_array, long node1, long node2, long r) {
    for (long i = 0; i < tree_array->num_trees; i++) {
        while (mrca(&tree_array->trees[i], node1, node2) > r) {
            decrease_mrca(&tree_array->trees[i], node1, node2);
        }
    }
    return 0;
}

long sos(Tree_Array* tree_array, Tree* focal_tree) {
    // compute sum of squared distances for all tree in tree_array to focal_tree
    long sos = 0;
    for (long i = 0; i < tree_array->num_trees; i++) {
        sos += rnni_distance(&tree_array->trees[i], focal_tree);
    }
    return sos;
}
