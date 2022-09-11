/*Basic functions for ranked trees*/

#include "tree.h"


// create empty node
Node empty_node(){
    Node new_node;
    new_node.parent = -1;
    new_node.children[0] = -1;
    new_node.children[1] = -1;
    new_node.time = -1;
    return new_node;
}


// create empty tree on num_leaves leaves
Tree* empty_tree(long num_leaves){
    long num_nodes = 2 * num_leaves - 1;
    Tree* new_tree = malloc(sizeof(Tree));
    new_tree->tree = calloc(num_nodes, sizeof(Node));
    new_tree->num_leaves = num_leaves;
    for (long i = 0; i < num_nodes; i++){
        new_tree->tree[i] = empty_node();
    }
    return new_tree;
}


// copy to_copy_tree into tree
void copy_tree(Tree* tree, Tree* to_copy_tree){
    long num_nodes = 2 * tree->num_leaves - 1;
    for (long i = 0; i < num_nodes; i++){
        tree->tree[i] = to_copy_tree->tree[i];
    }
}


// create new copy of tree
Tree* new_tree_copy(Tree* tree){
    long num_leaves = tree->num_leaves;
    Tree* tree_copy = empty_tree(num_leaves);
    copy_tree(tree_copy, tree);
    return tree_copy;
}


// create an empty Tree_List
Tree_List empty_tree_array(long num_trees, long num_leaves){
    Tree_List tree_array;
    tree_array.num_trees = num_trees;
    tree_array.trees = calloc(num_trees, sizeof(Tree));
    for (long i = 0; i < num_trees; i++){
        tree_array.trees[i].num_leaves = num_leaves;
        tree_array.trees[i].tree = calloc(2 * num_leaves - 1, sizeof(Node));
    }
    return tree_array;
}


// print parents and children for each node in tree
void print_tree(Tree* tree){
    long num_leaves = tree->num_leaves;
    long num_nodes = 2 * num_leaves - 1;
    for (long rank = 0; rank < num_nodes; rank++){
        printf("Node at rank %ld: parent %ld, children %ld, %ld\n", rank, tree->tree[rank].parent, tree->tree[rank].children[0], tree->tree[rank].children[1]);
    }
}


// Check whether two trees have the same (ranked) tree topology
int same_topology(Tree* tree1, Tree* tree2){
    long num_leaves = tree1->num_leaves;
    long num_nodes = 2 * num_leaves - 1;
    for(long i = num_leaves; i < num_nodes; i++){
        if (tree1->tree[i].parent != tree2->tree[i].parent){
            return FALSE;
        }
    }
    return TRUE;
}


// Check whether two trees are identical
int same_tree(Tree* tree1, Tree* tree2){
    long num_nodes = 2 * tree1->num_leaves - 1;
    for (long i = 0; i < num_nodes; i++){
        if (tree1->tree[i].parent != tree2->tree[i].parent){
            return FALSE;
        }
    }
    return TRUE;
}


// find rank of mrca of nodes with positions node1 and node2 in tree
long mrca(Tree* input_tree, long node1, long node2){
    long rank1 = node1;
    long rank2 = node2;
    while (rank1 != rank2){
        if (rank1 < rank2){
            rank1 = input_tree->tree[rank1].parent;
            if (rank1 == -1){
                printf("Cannot find mrca, reached root.");
                return -1;
            }
        } else{
            rank2 = input_tree->tree[rank2].parent;
            if (rank2 == -1){
                printf("Cannot find mrca, reached root.");
                return -1;
            }
        }
    }
    return rank1;
}


// return array with rank(mrca_{tree1}(C_i)) at position i where C_i is the cluster induced by node of rank i in tree2
long* mrca_array(Tree* tree1, Tree* tree2){
    long num_leaves = tree1->num_leaves;
    long num_nodes = 2 * num_leaves - 1;
    long *mrca_array = calloc(num_nodes, sizeof(long));

    for (long i = num_leaves; i < num_nodes; i++){
        // find nodes (child0, child1) in tree1 that are mrcas of the clusters induced by children of node of rank i in tree2
        long child0;
        if (tree2->tree[i].children[0] < num_leaves){
            child0 = tree2->tree[i].children[0];
        } else{
            child0 = mrca_array[tree2->tree[i].children[0]];
        }

        long child1;
        if (tree2->tree[i].children[1] < num_leaves){
            child1 = tree2->tree[i].children[1];
        } else{
            child1 = mrca_array[tree2->tree[i].children[1]];
        }

        long current_mrca = mrca(tree1, child0, child1);
        mrca_array[i] = current_mrca;
    }
    return mrca_array;
}


// Compute differences of ranks of mrcas of all clusters of tree2 btw tree1 and tree2
// Also add ranks of parents of leaves if include_leaf_parents == 0
long mrca_differences(Tree* tree1, Tree* tree2, int include_leaf_parents){
    long sum = 0;
    long num_leaves = tree2->num_leaves;
    long num_nodes = 2 * num_leaves - 1;
    // First iterate through leaves
    if (include_leaf_parents == TRUE){
        for (long i = 0; i < num_leaves; i++){
            sum += abs(tree1->tree[i].parent - tree2->tree[i].parent);
        }
    }
    long* mrcas = mrca_array(tree1, tree2);
    for (long i = num_leaves; i < num_nodes; i++){
        sum += mrcas[i] - i;
    }
    return sum;
}


// return matrix cluster*leaves -- 0 if leaf is not in cluster, 1 if it is in cluster
long ** get_clusters(Tree* tree){
    long num_leaves = tree->num_leaves;
    long ** clusters = calloc(num_leaves - 1, sizeof(long *));
    for (long i = 0; i < num_leaves - 1; i++){
        clusters[i] = calloc(num_leaves, sizeof(long));
    }

    for (long i = 0; i < num_leaves ; i++){
        for (long j = 0; j < num_leaves - 1; j++){
            clusters[j][i] = 0; //initialise all entries to be 0
        }
        // fill clusters
        long j = i;
        while (tree->tree[j].parent != -1){
            j = tree->tree[j].parent;
            clusters[j - num_leaves][i] = 1;
        }
        clusters[num_leaves - 2][i] = 1;
    }
    return clusters;
}


// Computes sum of symmetric differences of clusters of tree1 and tree2 for all ranks i=1,..,n-1
long sum_symmetric_cluster_diff(Tree* tree1, Tree* tree2){
    long num_leaves = tree1->num_leaves;

    long ** clusters_t1 = get_clusters(tree1);
    long ** clusters_t2 = get_clusters(tree2);

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
    long ** clusters_t1 = get_clusters(tree1);
    long ** clusters_t2 = get_clusters(tree2);

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
