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

// return array with rank(mrca_{tree1}(C_i)) at position i where C_i is the
// cluster induced by node of rank i in tree2
long* mrca_array(Tree* tree1, Tree* tree2) {
    long num_leaves = tree1->num_leaves;
    long num_nodes = 2 * num_leaves - 1;
    long* mrca_array = calloc(num_nodes, sizeof(long));

    for (long i = num_leaves; i < num_nodes; i++) {
        // find nodes (child0, child1) in tree1 that are mrcas of the clusters
        // induced by children of node of rank i in tree2
        long child0;
        if (tree2->node_array[i].children[0] < num_leaves) {
            child0 = tree2->node_array[i].children[0];
        } else {
            child0 = mrca_array[tree2->node_array[i].children[0]];
        }

        long child1;
        if (tree2->node_array[i].children[1] < num_leaves) {
            child1 = tree2->node_array[i].children[1];
        } else {
            child1 = mrca_array[tree2->node_array[i].children[1]];
        }

        long current_mrca = mrca(tree1, child0, child1);
        mrca_array[i] = current_mrca;
    }
    return mrca_array;
}

// Compute differences of ranks of mrcas of all clusters of tree2 btw tree1 and
// tree2 Also add ranks of parents of leaves if include_leaf_parents == 0
long mrca_differences(Tree* tree1, Tree* tree2, int include_leaf_parents) {
    long sum = 0;
    long num_leaves = tree2->num_leaves;
    long num_nodes = 2 * num_leaves - 1;
    // First iterate through leaves
    if (include_leaf_parents == TRUE) {
        for (long i = 0; i < num_leaves; i++) {
            sum +=
                abs(tree1->node_array[i].parent - tree2->node_array[i].parent);
        }
    }
    long* mrcas = mrca_array(tree1, tree2);
    for (long i = num_leaves; i < num_nodes; i++) {
        sum += mrcas[i] - i;
    }
    return sum;
}

// return matrix cluster*leaves -- 0 if leaf is not in cluster, 1 if it is in
// cluster
long** get_clusters(Tree* tree) {
    long num_leaves = tree->num_leaves;
    long** clusters = calloc(num_leaves - 1, sizeof(long*));
    for (long i = 0; i < num_leaves - 1; i++) {
        clusters[i] = calloc(num_leaves, sizeof(long));
    }

    for (long i = 0; i < num_leaves; i++) {
        for (long j = 0; j < num_leaves - 1; j++) {
            clusters[j][i] = 0;  // initialise all entries to be 0
        }
        // fill clusters
        long j = i;
        while (tree->node_array[j].parent != -1) {
            j = tree->node_array[j].parent;
            clusters[j - num_leaves][i] = 1;
        }
        clusters[num_leaves - 2][i] = 1;
    }
    return clusters;
}

// Computes sum of symmetric differences of clusters of tree1 and tree2 for all
// ranks i=1,..,n-1
long sum_symmetric_cluster_diff(Tree* tree1, Tree* tree2) {
    long num_leaves = tree1->num_leaves;

    long** clusters_t1 = get_clusters(tree1);
    long** clusters_t2 = get_clusters(tree2);

    // Now compute symmetric difference between clusters (i.e. all columns that
    // have a 1 in cluster_t1 or cluster_t2 and a 0 in the other matrix)
    long symm_diff = 0;
    for (long i = 0; i < num_leaves - 1; i++) {
        for (long j = 0; j < num_leaves; j++) {
            if (clusters_t1[i][j] + clusters_t2[i][j] == 1) {
                symm_diff++;
            }
        }
    }
    free(clusters_t1);
    free(clusters_t2);
    return symm_diff;
}

// Compute symmetric difference of clusters induced by nodes of rank k in tree1
// and tree2
long symmetric_cluster_diff(Tree* tree1, Tree* tree2, long k) {
    long num_leaves = tree1->num_leaves;
    long** clusters_t1 = get_clusters(tree1);
    long** clusters_t2 = get_clusters(tree2);

    // now count the number of entries that are a 1 in either cluster_t1 or
    // cluster_t2 in row k-num_leaves
    long output = 0;
    for (long i = 0; i < num_leaves; i++) {
        if (clusters_t1[k - num_leaves][i] + clusters_t2[k - num_leaves][i] ==
            1) {
            output++;
        }
    }
    for (long i = 0; i < num_leaves - 1; i++) {
        free(clusters_t1[i]);
        free(clusters_t2[i]);
    }
    free(clusters_t1);
    free(clusters_t2);
    return output;
}