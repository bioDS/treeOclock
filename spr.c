/*Implementations for ranked SPR treespaces (HSPR and RSPR)*/

#include "spr.h"

// ranked SPR move pruning the child with index child_moving of the node at
// position r of the node_array reattachment as sibling of the node at position
// new_sibling in node_array
int spr_move(Tree* tree, long r, long new_sibling, int child_moving) {
    if (new_sibling > r || tree->node_array[new_sibling].parent < r) {
        // HSPR move only possible if edge for re-attachment covers rank r
        printf(
            "Error. No SPR move possible. Destination edge does not cover rank "
            "r.\n");
        return EXIT_FAILURE;
    }
    long old_parent = tree->node_array[r].parent;
    long new_parent = tree->node_array[new_sibling].parent;
    long old_sibling = tree->node_array[r].children[1 - child_moving];

    // update part of tree where subtree has been pruned
    for (int i = 0; i <= 1; i++) {
        if (tree->node_array[old_parent].children[i] == r) {
            tree->node_array[old_sibling].parent = old_parent;
            tree->node_array[old_parent].children[i] = old_sibling;
        }
    }

    // update part of tree where subtree gets re-attached
    for (int i = 0; i <= 1; i++) {
        if (tree->node_array[new_parent].children[i] == new_sibling) {
            tree->node_array[r].parent = new_parent;
            tree->node_array[new_parent].children[i] = r;
        }
    }
    tree->node_array[new_sibling].parent = r;
    tree->node_array[r].children[1 - child_moving] = new_sibling;
    return EXIT_SUCCESS;
}

// Compute Tree_Array of all spr_neighbours
// If horizontal = FALSE, returns RSPR neighbourhood (including rank moves),
// otherwise HSPR neighbouhood (without rank moves)
Tree_Array all_spr_neighbourhood(Tree* tree, int horizontal) {
    long num_leaves = tree->num_leaves;
    long max_nh_size = 2 * num_leaves * (num_leaves - 1);

    // Initialise array of neighbours
    Tree_Array neighbour_array = get_empty_tree_array(max_nh_size, num_leaves);
    long index = 0;  // index to the currently last element in neighbour_array

    // Deep copy input tree to get neighbouring trees
    Tree* neighbour = new_tree_copy(tree);
    Tree* next_neighbour_array;
    next_neighbour_array = &neighbour_array.trees[index];

    // Loop through all possible ranks on which moves can happen ('ranks' here
    // means position in node array, where the first n entries are leaves)
    for (long r = num_leaves; r < 2 * num_leaves - 2; r++) {
        // Check if we can do rank move:
        if (horizontal == FALSE && r < 2 * num_leaves - 2 &&
            tree->node_array[r].parent != r + 1) {
            rank_move(neighbour, r);
            copy_tree(next_neighbour_array, neighbour);
            index++;
            next_neighbour_array = &neighbour_array.trees[index];
            copy_tree(neighbour, tree);
        }
        for (long new_sibling = 0; new_sibling < r; new_sibling++) {
            if (tree->node_array[new_sibling].parent > r) {
                // Two SPR moves, moving either of the children of the node of
                // rank r
                for (int child = 0; child < 2; child++) {
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
    return (neighbour_array);
}

Tree_Array spr_neighbourhood(Tree* tree) {
    return all_spr_neighbourhood(tree, FALSE);
}

Tree_Array hspr_neighbourhood(Tree* tree) {
    return all_spr_neighbourhood(tree, TRUE);
}
