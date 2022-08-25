// Functions for unlabelled RNNI
// Author: Lena Collienne

#include "unlabelled_rnni.h"


// NNI move on edge bounded by rank rank_in_list and rank_in_list + 1, moving child_stays (index) of the lower node up
int unlabelled_nni_move(Tree * input_tree, long rank_in_list, int child_moves_up){
    if (input_tree->tree == NULL){
        printf("Error. No RNNI move possible. Given tree doesn't exist.\n");
    } else{
        if(input_tree->tree[rank_in_list].parent != rank_in_list + 1){
            printf("Can't do an NNI - interval [%ld, %ld] is not an edge!\n", rank_in_list, rank_in_list + 1);
            return 1;
        }
        int child_moved_up;
        for (int i = 0; i < 2; i++){
            if (input_tree->tree[rank_in_list+1].children[i] != rank_in_list){ //find the child of the node of rank_in_list k+1 that is not the node of rank_in_list k
                // update parent/children relations to get nni neighbour
                input_tree->tree[input_tree->tree[rank_in_list+1].children[i]].parent = rank_in_list; //update parents
                input_tree->tree[input_tree->tree[rank_in_list].children[child_moves_up]].parent = rank_in_list+1;
                child_moved_up = input_tree->tree[rank_in_list].children[child_moves_up];
                input_tree->tree[rank_in_list].children[child_moves_up] = input_tree->tree[rank_in_list+1].children[i]; //update children
                input_tree->tree[rank_in_list+1].children[i] = child_moved_up;
            }
        }
    }
    return 0;
}


// Compute Tree_List of all UNLABELLED RNNI neighbours
// Note that some trees might appear multiple times in the output list!
Tree_List unlabelled_rnni_neighbourhood(Tree *input_tree){
    long num_leaves = input_tree->num_leaves;

    // Initialise list of neighbours
    Tree_List neighbour_list; // output list of neighbours
    neighbour_list.num_trees = 2 * (num_leaves - 1); // upper bound on max number of neighbours at most 2*(n-1) if every interval has NNI moves
    neighbour_list.trees = malloc(neighbour_list.num_trees * sizeof(Tree));
    for (long i = 0; i < neighbour_list.num_trees; i++){
        neighbour_list.trees[i].num_leaves = num_leaves;
        neighbour_list.trees[i].tree = malloc((num_leaves - 1) * sizeof(Node));
    }
    long index = 0; //index to the currently last element in neighbour_list

    //Deep copy input tree to get neighbouring trees
    Tree * neighbour = malloc(sizeof(Node*) + 3 * sizeof(long));
    neighbour->num_leaves = num_leaves;
    neighbour->tree = malloc((num_leaves - 1) * sizeof(Node)); // deep copy start tree
    for (long i = 0; i < num_leaves - 1; i++){
        neighbour->tree[i] = input_tree->tree[i];
    }
    // Loop through all possible ranks on which moves can happen ('ranks' here means position in node list, where the first n entries are leaves)
    for (long r=0; r<num_leaves-2; r++){
        // Check if we can do rank move:
        if (input_tree->tree[r].parent != r+1){
            rank_move(neighbour, r);
        } else{ // otherwise, we can perform an NNI move
            nni_move(neighbour, r, 0);
            // Add neighbour to neighbour_list:
            // deep copy neighbour to path
            for (long i = 0; i < num_leaves - 1; i++){
                neighbour_list.trees[index].tree[i] = neighbour->tree[i];
            }
            index++;
            // always reset neighbour to be input_tree after every move
            for (long i = 0; i < num_leaves - 1; i++){
                neighbour->tree[i] = input_tree->tree[i];
            }
            nni_move(neighbour, r, 1);
        }
        // Add neighbour to neighbour_list:
        // deep copy neighbour to path
        for (long i = 0; i < num_leaves - 1; i++){
            neighbour_list.trees[index].tree[i] = neighbour->tree[i];
        }
        index++;
        // always reset neighbour to be input_tree after every move
        for (long i = 0; i < num_leaves - 1; i++){
            neighbour->tree[i] = input_tree->tree[i];
        }
    }
    neighbour_list.num_trees = index;
    // printf("number of neighbours: %ld\n", index);
    free(neighbour);
    return(neighbour_list);
}
