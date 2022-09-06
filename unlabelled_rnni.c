// Functions for unlabelled RNNI
// Author: Lena Collienne

#include "unlabelled_rnni.h"



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
    // Loop through all possible ranks on which moves can happen ('ranks' here means position in node list)
    for (long r=0; r<num_leaves-2; r++){
        // Check if we can do rank move:
        if (input_tree->tree[r].parent != r+1){
            rank_move(neighbour, r);
            // always reset neighbour to be input_tree after every move
            for (long i = 0; i < num_leaves - 1; i++){
                neighbour->tree[i] = input_tree->tree[i];
            }
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
