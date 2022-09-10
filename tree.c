/*BAsic functions for ranked trees*/

#include "tree.h"


Node empty_node(){
    Node new_node;
    new_node.parent = -1;
    new_node.children[0] = -1;
    new_node.children[1] = -1;
    new_node.time = -1;
    return new_node;
}


Tree* empty_tree(long num_leaves){
    // initalise tree on num_leaves leaves
    Tree* new_tree = malloc(sizeof(Tree));
    new_tree->tree = calloc((2 * num_leaves - 1), sizeof(Node));
    new_tree->num_leaves = num_leaves;
    for (long i = 0; i < 2 * num_leaves - 1; i++){
        new_tree->tree[i] = empty_node();
    }
    return new_tree;
}


Tree* deep_copy(Tree* tree){
    // deep copy tree
    long num_leaves = tree->num_leaves;
    Tree* tree_copy = empty_tree(num_leaves);
    for (long i = 0; i < 2 * num_leaves - 1; i++){
        tree_copy->tree[i] = tree->tree[i];
    }
    return tree_copy;
}


void print_tree(Tree* tree){
    // print parents and children for each node in tree
    long num_leaves = tree->num_leaves;
    long num_nodes = 2 * num_leaves - 1;
    for (long rank = 0; rank < num_nodes; rank++){
        printf("Node at rank %ld: parent %ld, children %ld, %ld\n", rank, tree->tree[rank].parent, tree->tree[rank].children[0], tree->tree[rank].children[1]);
    }
}


// Check whether two trees have the same (ranked) tree topology -- return 0 if this is the case, 1 otherwise
int same_topology(Tree* tree1, Tree* tree2){
    long num_leaves = tree1->num_leaves;
    for(long i=num_leaves; i < 2 * num_leaves - 1; i++){
        if (tree1->tree[i].parent != tree2->tree[i].parent){
            return(1);
        }
    }
    return(0);
}


// Check whether two trees are identical (output 0 if this is true, 1 otherwise)
int same_tree(Tree* tree1, Tree* tree2){
    for (long i = 0; i < 2* tree1->num_leaves - 1; i++){
        if (tree1->tree[i].parent != tree2->tree[i].parent){
            return(1);
        }
    }
    return(0);
}

// find mrca of nodes with positions node1 and node2 in tree
// returns the rank of the mrca
long mrca(Tree * input_tree, long node1, long node2){
    long rank1 = node1;
    long rank2 = node2;
    while (rank1 != rank2){
        if (rank1 < rank2){
            rank1 = input_tree->tree[rank1].parent;
            if (rank1 == -1){
                return -1;
            }
        } else{
            rank2 = input_tree->tree[rank2].parent;
            if (rank2 == -1){
                return -1;
            }
        }
    }
    return rank1;
}


long* mrca_list(Tree* tree1, Tree* tree2){
    long num_leaves = tree1->num_leaves;
    long *mrca_list = malloc((2*num_leaves-1)*sizeof(long)); // at position i save rank(mrca_{tree1}(C_i)) where C_i is the cluster induced by node of rank i in tree2

    for (long i = num_leaves; i < 2*num_leaves - 1; i++){
        // iterate through the ranks of mrcas in dest_tree
        // find mrca (distinguish leaves vs. non-leaf and fill mrca_list to get mrcas of non-leafs)
        long child0;
        if (tree2->tree[i].children[0] < num_leaves){
            child0 = tree2->tree[i].children[0];
        } else{
            child0 = mrca_list[tree2->tree[i].children[0]];
        }

        long child1;
        if (tree2->tree[i].children[1] < num_leaves){
            child1 = tree2->tree[i].children[1];
        } else{
            child1 = mrca_list[tree2->tree[i].children[1]];
        }

        long current_mrca = mrca(tree1, child0, child1);
        mrca_list[i] = current_mrca;
    }
    return mrca_list;
}


long mrca_differences(Tree* current_tree, Tree* dest_tree, int include_leaf_parents){
    // Compute differences of ranks of mrcas of all cluster of dest_tree btw current_tree and dest_tree
    // Also add ranks of parents of leaves if include_leaf_parents == 0
    long sum = 0;
    long num_leaves = dest_tree->num_leaves;
    // long mrcas[2*num_leaves-1]; // at position i save rank(mrca_{current_tree}(C_i)) where C_i is the cluster induced by node of rank i in dest_tree
    // First iterate through leaves
    if (include_leaf_parents == 0){
        for (long i = 0; i < num_leaves; i++){
            sum += abs(current_tree->tree[i].parent - dest_tree->tree[i].parent);
        }
    }
    long* mrcas = mrca_list(current_tree, dest_tree);
    for (long i = num_leaves; i < 2*num_leaves-1; i++){
        sum += (mrcas[i] - i);
    }
    return sum;
}


long sum_symmetric_cluster_diff(Tree* tree1, Tree* tree2){
    // Computes sum of symmetric differences of clusters of tree1 and tree2 for all ranks i=1,..,n-1
    long num_leaves = tree1->num_leaves;

    // Get clusters for both trees:
    // create matrix cluster*leaves -- 0 if leaf is not in cluster, 1 if it is in cluster
    long ** clusters_t1 = malloc((num_leaves - 1) * sizeof(long *));
    for (long i = 0; i < num_leaves - 1; i++){
        clusters_t1[i] = malloc((num_leaves) * sizeof(long));
    }

    for (long i = 0; i < num_leaves ; i++){
        for (long j = 0; j < num_leaves - 1; j++){
            clusters_t1[j][i] = 0; //initialise all entries to be 0
        }
        long j = i;
        while (tree1->tree[j].parent != -1){
            j = tree1->tree[j].parent;
            clusters_t1[j - num_leaves][i] = 1;
        }
        clusters_t1[num_leaves - 2][i] = 1;
    }

    long ** clusters_t2 = malloc((num_leaves - 1) * sizeof(long *));
    for (long i = 0; i < num_leaves - 1; i++){
        clusters_t2[i] = malloc((num_leaves) * sizeof(long));
    }

    for (long i = 0; i < num_leaves ; i++){
        for (long j = 0; j < num_leaves - 1; j++){
            clusters_t2[j][i] = 0; //initialise all entries to be 0
        }
        long j = i;
        while (tree2->tree[j].parent != -1){
            j = tree2->tree[j].parent;
            clusters_t2[j - num_leaves][i] = 1;
        }
        clusters_t2[num_leaves - 2][i] = 1;
    }

    // Now compute symmetric difference between clusters (i.e. all columns that have a 1 in cluster_t1 or cluster_t2 and a 0 in the other matrix)
    long symm_diff = 0;
    for (long i = 0; i < num_leaves -1; i++){
        for (long j = 0; j < num_leaves; j++){
            if (clusters_t1[i][j] + clusters_t2[i][j] == 1){
                symm_diff++;
            }
        }
    }
    free(clusters_t1);
    free(clusters_t2);
    return symm_diff;
}


// Compute symmetric difference of clusters induced by nodes of rank k in tree1 and tree2
long symmetric_cluster_diff(Tree* tree1, Tree* tree2, long k){
    long num_leaves = tree1->num_leaves;
    // find clusters induced by node of rank k in both tree1 and tree2
    // Note: getting cluster matrix is copy&paste from tree_to_string function. We only need one column of this matrix here
    // create matrix cluster*leaves -- 0 if leaf is not in cluster, 1 if it is in cluster
    // first: clusters_t1 for tree1
    long ** clusters_t1 = malloc((num_leaves - 1) * sizeof(long *));
    for (long i = 0; i < num_leaves - 1; i++){
        clusters_t1[i] = malloc((num_leaves) * sizeof(long));
    }

    for (long i = 0; i < num_leaves ; i++){
        for (long j = 0; j < num_leaves - 1; j++){
            clusters_t1[j][i] = 0; //initialise all entries to be 0
        }
        long j = i;
        while (tree1->tree[j].parent != -1){
            j = tree1->tree[j].parent;
            clusters_t1[j - num_leaves][i] = 1;
        }
        clusters_t1[num_leaves - 2][i] = 1;
    }
    // same thing for tree2
    long ** clusters_t2 = malloc((num_leaves - 1) * sizeof(long *));
    for (long i = 0; i < num_leaves - 1; i++){
        clusters_t2[i] = malloc((num_leaves) * sizeof(long));
    }

    for (long i = 0; i < num_leaves ; i++){
        for (long j = 0; j < num_leaves - 1; j++){
            clusters_t2[j][i] = 0; //initialise all entries to be 0
        }
        long j = i;
        while (tree2->tree[j].parent != -1){
            j = tree2->tree[j].parent;
            clusters_t2[j - num_leaves][i] = 1;
        }
        clusters_t2[num_leaves - 2][i] = 1;
    }

    // now count the number of entries that are a 1 in either cluster_t1 or cluster_t2 in row k-num_leaves
    long output = 0;
    for (long i = 0; i < num_leaves; i++){
        if (clusters_t1[k-num_leaves][i]+clusters_t2[k-num_leaves][i]==1){
            output++;
        }
    }
    for (long i = 0; i < num_leaves - 1; i++){
        free(clusters_t1[i]);
        free(clusters_t2[i]);
    }
    free(clusters_t1);
    free(clusters_t2);
    return output;
}
