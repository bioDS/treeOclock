// Functions for unlabelled RNNI

#include "unlabelled_rnni.h"


int unlabelled_nni_move(Unlabelled_Tree * tree, long rank, int child_moves_up) {
    return nni_move(tree, rank, child_moves_up);
}


int unlabelled_rank_move(Unlabelled_Tree * tree, long rank){
    return rank_move(tree, rank);
}


// Compute Unlabelled_Tree_Array of all unlabelled RNNI neighbours
// Note that some trees might appear multiple times in the output array!
Unlabelled_Tree_Array unlabelled_rnni_neighbourhood(Unlabelled_Tree *tree){
    long num_leaves = tree->num_leaves;
    long max_nh_size = 2 * (num_leaves - 1);

    Unlabelled_Tree_Array neighbour_array = empty_unlabelled_tree_array(max_nh_size, num_leaves);
    long index = 0; //index to the currently last element in neighbour_array

    //Deep copy input tree to get neighbouring trees
    Unlabelled_Tree * neighbour = new_unlabelled_tree_copy(tree);
    Tree* next_neighbour_array;
    next_neighbour_array = &neighbour_array.trees[index];

    for (long r=0; r<num_leaves-2; r++){
        // Check if we can do rank move:
        if (tree->node_array[r].parent != r+1){
            unlabelled_rank_move(neighbour, r);
        } else{ // otherwise, we can perform an NNI move
            unlabelled_nni_move(neighbour, r, 0);
            // Add neighbour to neighbour_array:
            copy_unlabelled_tree(next_neighbour_array, neighbour);
            index ++;
            next_neighbour_array = &neighbour_array.trees[index];
            copy_unlabelled_tree(neighbour, tree);

            unlabelled_nni_move(neighbour, r, 1);
        }
        // Add neighbour to neighbour_array:
        copy_unlabelled_tree(next_neighbour_array, neighbour);
        index ++;
        next_neighbour_array = &neighbour_array.trees[index];
        copy_unlabelled_tree(neighbour, tree);
    }
    neighbour_array.num_trees = index;
    free_tree(neighbour);
    return(neighbour_array);
}
