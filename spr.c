/*Implementations for ranked SPR treespaces (HSPR and RSPR)*/

#include "spr.h"


// ranked SPR move pruning the child with index child_moving of the node at position r of the node_array
// reattachment as sibling of the node at position new_sibling in node_array
int spr_move(Tree * input_tree, long r, long new_sibling, int child_moving){
    if (new_sibling > r || input_tree->node_array[new_sibling].parent < r){
        printf("Error. No SPR move possible. Destination edge does not cover rank r.\n");
        return EXIT_FAILURE;
    }
    long old_parent = input_tree->node_array[r].parent;
    long new_parent = input_tree->node_array[new_sibling].parent;
    long old_sibling = input_tree->node_array[r].children[1-child_moving];

    // update part of tree where subtree has been pruned
    for (int i=0; i<=1; i++){
        if (input_tree->node_array[old_parent].children[i] == r){
            input_tree->node_array[old_sibling].parent = old_parent;
            input_tree->node_array[old_parent].children[i] = old_sibling;
        }
    }

    // update part of tree where subtree gets re-attached
    for (int i=0; i<=1; i++){
        if (input_tree->node_array[new_parent].children[i] == new_sibling){
            input_tree->node_array[r].parent = new_parent;
            input_tree->node_array[new_parent].children[i] = r;
        }
    }
    input_tree->node_array[new_sibling].parent = r;
    input_tree->node_array[r].children[1-child_moving] = new_sibling;
    return EXIT_SUCCESS;
}


// unlabelled ranked SPR move pruning the child with index child_moving of node at position r of the node_array
// re-attaching as children[new_child] of tree at position new_parent of node_array
int unlabelled_spr_move(Tree * input_tree, long r, long new_parent, int child_moving, int new_child_index){
    if (r > new_parent || input_tree->node_array[new_parent].children[new_child_index] > r){
        printf("Error. No unlabelled SPR move possible\n");
        return EXIT_FAILURE;
    }

    // update part of tree where we cut subtree
    long old_parent = input_tree->node_array[r].parent;
    long old_sibling = input_tree->node_array[r].children[1-child_moving];
    for (int i=0; i<=1; i++){ // find out which child of old_parent r is
        if (input_tree->node_array[old_parent].children[i] == r){
            if (old_sibling != -1){
                input_tree->node_array[old_sibling].parent = old_parent;
            }
            input_tree->node_array[old_parent].children[i] = old_sibling;
        }
    }

    // update part of tree where subtree gets re-attached
    long new_child = input_tree->node_array[new_parent].children[new_child_index];
    input_tree->node_array[new_parent].children[new_child_index] = r;
    input_tree->node_array[r].children[1-child_moving] = new_child;
    input_tree->node_array[r].parent = new_parent;
    if (new_child != -1){
        input_tree->node_array[new_child].parent = r;
    }
    return EXIT_SUCCESS;
}



// Compute Tree_Array of all spr_neighbours
// If horizontal = FALSE, returns RSPR neighbourhood (including rank moves), otherwise HSPR neighbouhood (without rank moves)
Tree_Array all_spr_neighbourhood(Tree* tree, int horizontal){
    long num_leaves = tree->num_leaves;
    long max_nh_size = 2 * num_leaves * (num_leaves - 1);

    // Initialise array of neighbours
    Tree_Array neighbour_array = empty_tree_array(max_nh_size, num_leaves);
    long index = 0; //index to the currently last element in neighbour_array

    //Deep copy input tree to get neighbouring trees
    Tree * neighbour = new_tree_copy(tree);
    Tree* next_neighbour_array;
    next_neighbour_array = &neighbour_array.trees[index];

    // Loop through all possible ranks on which moves can happen ('ranks' here means position in node array, where the first n entries are leaves)
    for (long r=num_leaves; r<2* num_leaves-2; r++){
        // Check if we can do rank move:
        if (horizontal == FALSE && r < 2*num_leaves - 2 && tree->node_array[r].parent != r+1){
            rank_move(neighbour, r);
            copy_tree(next_neighbour_array, neighbour);
            index++;
            next_neighbour_array = &neighbour_array.trees[index];
            copy_tree(neighbour, tree);

        }
        for (long new_sibling=0; new_sibling<r; new_sibling++){
            if (tree->node_array[new_sibling].parent > r){
                // Two SPR moves, moving either of the children of the node of rank r
                for (int child = 0; child < 2; child++){
                    spr_move(neighbour, r, new_sibling, child);
                    copy_tree(next_neighbour_array, neighbour);
                    index++;
                    next_neighbour_array = &neighbour_array.trees[index];
                    copy_tree(neighbour, tree);
                }
            }
        }
    }
    neighbour_array.num_trees = index;
    free(neighbour);
    return(neighbour_array);
}


Tree_Array spr_neighbourhood(Tree* input_tree){
    return all_spr_neighbourhood(input_tree, FALSE);
}


Tree_Array hspr_neighbourhood(Tree* input_tree){
    return all_spr_neighbourhood(input_tree, TRUE);
}
