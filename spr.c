/*Implementations for ranked SPR treespaces (HSPR and RSPR)*/

#include "spr.h"


// ranked SPR move pruning the child with index child_moving of the node at position r of the node_list.
// The subtree gets re-attached as sibling of the node at position new_sibling of the node_list.
// r and new_sibling are the positions in the node_list rather than actual ranks.
int spr_move(Tree * input_tree, long r, long new_sibling, int child_moving){
    if (input_tree->tree == NULL){
        printf("Error. No SPR move possible. Given tree doesn't exist.\n");
        return 1;
    }
    if (new_sibling > r || input_tree->tree[new_sibling].parent < r){
        printf("Error. No SPR move possible. Destination edge does not cover rank r.\n");
        return 1;
    }
    long old_parent = input_tree->tree[r].parent;
    long new_parent = input_tree->tree[new_sibling].parent;
    long old_sibling = input_tree->tree[r].children[1-child_moving];

    // update part of tree where subtree has been pruned
    for (int i=0; i<=1; i++){
        if (input_tree->tree[old_parent].children[i] == r){
            input_tree->tree[old_sibling].parent = old_parent;
            input_tree->tree[old_parent].children[i] = old_sibling;
        }
    }

    // update part of tree where subtree gets re-attached
    for (int i=0; i<=1; i++){
        if (input_tree->tree[new_parent].children[i] == new_sibling){
            input_tree->tree[r].parent = new_parent;
            input_tree->tree[new_parent].children[i] = r;
        }
    }
    input_tree->tree[new_sibling].parent = r;
    input_tree->tree[r].children[1-child_moving] = new_sibling;
    return 0;
}


// unlabelled ranked SPR move pruning the child with index child_moving of the node at position r of the node_list.
// The subtree gets re-attached as children[new_child] of the node at position new_parent of the node_list.
// Note that r and new_sibling are the positions in the node_list rather than actual ranks.
int unlabelled_spr_move(Tree * input_tree, long r, long new_parent, int child_moving, int new_child_index){
        return 1;
    }
    if (r > new_parent || input_tree->tree[new_parent].children[new_child_index] > r){
        return 1;
    }

    // update part of tree where we cut subtree
    long old_parent = input_tree->tree[r].parent;
    long old_sibling = input_tree->tree[r].children[1-child_moving];
    for (int i=0; i<=1; i++){ // find out which child of old_parent r is
        if (input_tree->tree[old_parent].children[i] == r){
            if (old_sibling != -1){
                input_tree->tree[old_sibling].parent = old_parent;
            }
            input_tree->tree[old_parent].children[i] = old_sibling;
        }
    }

    // update part of tree where subtree gets re-attached
    long new_child = input_tree->tree[new_parent].children[new_child_index];
    input_tree->tree[new_parent].children[new_child_index] = r;
    input_tree->tree[r].children[1-child_moving] = new_child;
    input_tree->tree[r].parent = new_parent;
    if (new_child != -1){
        input_tree->tree[new_child].parent = r;
    }
    return 0;
}



// Compute Tree_List of all spr_neighbours
// If horizontal = 1, we compute the rankedSPR neighbourhood(including rank moves), otherwise the hspr neighbouhood (without rank moves)
Tree_List all_spr_neighbourhood(Tree *input_tree, int horizontal){
    long num_leaves = input_tree->num_leaves;

    // Initialise list of neighbours
    Tree_List neighbour_list; // output list of neighbours
    neighbour_list.num_trees = 2 * num_leaves * (num_leaves - 1); //max number of neighbours (quadratic in number of ranks + at most linear number of rank moves)
    neighbour_list.trees = malloc(neighbour_list.num_trees * sizeof(Tree));
    for (long i = 0; i < neighbour_list.num_trees; i++){
        neighbour_list.trees[i].num_leaves = num_leaves;
        neighbour_list.trees[i].tree = malloc((2* num_leaves - 1) * sizeof(Node));
    }
    long index = 0; //index to the currently last element in neighbour_list

    //Deep copy input tree to get neighbouring trees
    Tree * neighbour = malloc(sizeof(Node*) + 3 * sizeof(long));
    neighbour->num_leaves = num_leaves;
    neighbour->tree = malloc((2 * num_leaves - 1) * sizeof(Node)); // deep copy start tree
    for (long i = 0; i < 2 * num_leaves - 1; i++){
        neighbour->tree[i] = input_tree->tree[i];
    }

    // Loop through all possible ranks on which moves can happen ('ranks' here means position in node list, where the first n entries are leaves)
    for (long r=num_leaves; r<2* num_leaves-2; r++){
        // Check if we can do rank move:
        if (horizontal == 1 && r < 2*num_leaves - 2 && input_tree->tree[r].parent != r+1){
            rank_move(neighbour, r);

            // Add neighbour to neighbour_list:
            // deep copy neighbour to path
            for (long i = 0; i < 2 * num_leaves - 1; i++){
                neighbour_list.trees[index].tree[i] = neighbour->tree[i];
            }
            index++;
            // always reset neighbour to be input_tree after every move
            for (long i = 0; i < 2 * num_leaves - 1; i++){
                neighbour->tree[i] = input_tree->tree[i];
            } 

        }
        for (long new_sibling=0; new_sibling<r; new_sibling++){
            if (input_tree->tree[new_sibling].parent > r){
                // Two SPR moves, moving either of the children of the node of rank r
                spr_move(neighbour, r, new_sibling, 0);
                // Add neighbour to neighbour_list:
                // deep copy neighbour to path
                for (long i = 0; i < 2 * num_leaves - 1; i++){
                    neighbour_list.trees[index].tree[i] = neighbour->tree[i];
                }
                index++;
                // always reset neighbour to be input_tree after every move
                for (long i = 0; i < 2 * num_leaves - 1; i++){
                    neighbour->tree[i] = input_tree->tree[i];
                }

                spr_move(neighbour, r, new_sibling, 1);
                // Add neighbour to neighbour_list:
                // deep copy neighbour to path
                for (long i = 0; i < 2 * num_leaves - 1; i++){
                    neighbour_list.trees[index].tree[i] = neighbour->tree[i];
                }
                index++;
                // always reset neighbour to be input_tree after every move
                for (long i = 0; i < 2 * num_leaves - 1; i++){
                    neighbour->tree[i] = input_tree->tree[i];
                }
            }

        }
    }
    neighbour_list.num_trees = index;
    free(neighbour);
    return(neighbour_list);
}

Tree_List spr_neighbourhood(Tree* input_tree){
    return all_spr_neighbourhood(input_tree, 1);
}

Tree_List hspr_neighbourhood(Tree* input_tree){
    return all_spr_neighbourhood(input_tree, 0);
}

// Compute Tree_List of all UNLABELLED spr neighbours
// Note that some trees might appear multiple times in the output list!
// If horizontal = 1, we compute the unlabelled rankedSPR neighbourhood(including rank moves), otherwise the unlabelled hspr neighbouhood (without rank moves)
Tree_List unlabelled_spr_neighbourhood(Tree *input_tree, int horizontal){
    long num_leaves = input_tree->num_leaves;

    // Initialise list of neighbours
    Tree_List neighbour_list; // output list of neighbours
    neighbour_list.num_trees = 2 * num_leaves * (num_leaves - 1); //rough upper bound on max number of neighbours (quadratic in number of ranks + at most linear number of rank moves)
    // printf("upper bound for number of trees: %ld\n", neighbour_list.num_trees);
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
        if (horizontal == 1 && input_tree->tree[r].parent != r+1){
            rank_move(neighbour, r);
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
        for (long new_parent=r+1; new_parent<num_leaves-1; new_parent++){
            // perform an SPR move putting node of rank r between new_parent and its children[0]
            for (int i = 0; i <= 1; i++){
                for (int j = 0; j <=1; j++){
                    unlabelled_spr_move(neighbour, r, new_parent, i, j);
                    // Add neighbour to neighbour_list (deep copy)
                    for (long k = 0; k < num_leaves - 1; k++){
                        neighbour_list.trees[index].tree[k] = neighbour->tree[k];
                    }
                    index++;
                    // always reset neighbour to be input_tree after every move
                    for (long k = 0; k < num_leaves - 1; k++){
                        neighbour->tree[k] = input_tree->tree[k];
                    }
                }
            }
        }
    }
    neighbour_list.num_trees = index;
    free(neighbour);
    return(neighbour_list);
}

