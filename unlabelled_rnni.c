// Functions for unlabelled RNNI
// Author: Lena Collienne

#include "unlabelled_rnni.h"


int unlabelled_nni_move(Unlabelled_Tree * tree, long rank, int child_moves_up) {
    return nni_move(tree, rank, child_moves_up);
}


int unlabelled_rank_move(Unlabelled_Tree * tree, long rank){
    return rank_move(tree, rank);
}


// Compute Unlabelled_Tree_Array of all unlabelled RNNI neighbours
// Note that some trees might appear multiple times in the output list!
Unlabelled_Tree_Array unlabelled_rnni_neighbourhood(Unlabelled_Tree *tree){
    long num_leaves = tree->num_leaves;
    long max_nh_size = 2 * (num_leaves - 1);

    Unlabelled_Tree_Array neighbour_list = empty_unlabelled_tree_array(max_nh_size, num_leaves);
    long index = 0; //index to the currently last element in neighbour_list

    //Deep copy input tree to get neighbouring trees
    Unlabelled_Tree * neighbour = new_unlabelled_tree_copy(tree);
    // Loop through all possible ranks on which moves can happen ('ranks' here means position in node list)
    for (long r=0; r<num_leaves-2; r++){
        // Check if we can do rank move:
        if (tree->node_array[r].parent != r+1){
            unlabelled_rank_move(neighbour, r);
            // always reset neighbour to be tree after every move
            for (long i = 0; i < num_leaves - 1; i++){
                neighbour->node_array[i] = tree->node_array[i];
            }
        } else{ // otherwise, we can perform an NNI move
            unlabelled_nni_move(neighbour, r, 0);
            // Add neighbour to neighbour_list:
            // deep copy neighbour to path
            for (long i = 0; i < num_leaves - 1; i++){
                neighbour_list.trees[index].node_array[i] = neighbour->node_array[i];
            }
            index++;
            // always reset neighbour to be tree after every move
            for (long i = 0; i < num_leaves - 1; i++){
                neighbour->node_array[i] = tree->node_array[i];
            }
            unlabelled_nni_move(neighbour, r, 1);
        }
        // Add neighbour to neighbour_list:
        // deep copy neighbour to path
        for (long i = 0; i < num_leaves - 1; i++){
            neighbour_list.trees[index].node_array[i] = neighbour->node_array[i];
        }
        index++;
        // always reset neighbour to be tree after every move
        for (long i = 0; i < num_leaves - 1; i++){
            neighbour->node_array[i] = tree->node_array[i];
        }
    }
    neighbour_list.num_trees = index;
    // printf("number of neighbours: %ld\n", index);
    free(neighbour);
    return(neighbour_list);
}
