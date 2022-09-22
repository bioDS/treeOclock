/*Efficient implementation of FINDPATH on ranked trees*/

#include "rnni.h"

// NNI move on edge bounded by nodes at position r and r + 1
// moves child_moves_up (index -- 0 or 1) of the lower node up
// i.e. tree.node_array[r].children[child_moves_up] has parent of rank r+1 after
// move
int nni_move(Tree* tree, long r, int child_moves_up) {
    Node* upper_node;
    upper_node = &tree->node_array[r + 1];
    Node* lower_node;
    lower_node = &tree->node_array[r];
    if (lower_node->parent != r + 1) {
        printf("Can't do an NNI - interval [%ld, %ld] is not an edge!\n", r,
               r + 1);
        return EXIT_FAILURE;
    }
    int child_moved_up;
    for (int i = 0; i < 2; i++) {
        // find the child of the node of rank r+1 that is not the node of rank r
        if (upper_node->children[i] != r) {
            // update parent/children relations to get nni neighbour
            tree->node_array[upper_node->children[i]].parent = r;
            tree->node_array[lower_node->children[child_moves_up]].parent =
                r + 1;
            child_moved_up = lower_node->children[child_moves_up];
            lower_node->children[child_moves_up] = upper_node->children[i];
            upper_node->children[i] = child_moved_up;
        }
    }
    return EXIT_SUCCESS;
}

// Make a rank move on tree between nodes of rank and rank + 1 (if possible)
int rank_move(Tree* tree, long r) {
    if (tree->node_array[r].parent == r + 1) {
        printf(
            "Error. No rank move possible. The interval [%ld,%ld] is an "
            "edge!\n",
            r, r + 1);
        return EXIT_FAILURE;
    }
    Node* upper_node;
    upper_node = &tree->node_array[r + 1];
    Node* lower_node;
    lower_node = &tree->node_array[r];

    // update parents of nodes that swap ranks
    long upper_parent;
    upper_parent = upper_node->parent;
    upper_node->parent = lower_node->parent;
    lower_node->parent = upper_parent;

    for (int i = 0; i < 2; i++) {
        // update children of nodes that swap ranks
        long upper_child = upper_node->children[i];
        upper_node->children[i] = lower_node->children[i];
        lower_node->children[i] = upper_child;
        // update parents of children of nodes that swap ranks
        tree->node_array[upper_node->children[i]].parent++;
        tree->node_array[lower_node->children[i]].parent--;
    }
    for (int i = 0; i < 2; i++) {
        // update children of parents of nodes that swap rank
        if (upper_node->parent == lower_node->parent) {
            break;
        }
        if (tree->node_array[upper_node->parent].children[i] == r) {
            tree->node_array[upper_node->parent].children[i]++;
        }
        if (tree->node_array[lower_node->parent].children[i] == r + 1) {
            tree->node_array[lower_node->parent].children[i]--;
        }
    }
    return EXIT_SUCCESS;
}

// Use length moves to move up internal nodes between lowest_moving_node
// (including) and k (excluding) in tree.node_array in the end there are no
// nodes with rank less than k in the tree these are length moves that move
// nodes up -- see pseudocode FindPath^+ in DCT paper
int move_up(Tree* tree, long lowest_moving_node, long k) {
    long num_nodes = 2 * tree->num_leaves - 1;
    long num_moves = 0;  // counter for the number of moves that are necessary
    long highest_moving_node = lowest_moving_node;
    // Find highest j that needs to be moved up -- maximum is reached at root!
    while (tree->node_array[highest_moving_node + 1].time <= k &&
           highest_moving_node + 1 <= num_nodes - 1) {
        highest_moving_node++;
    }
    long num_moving_nodes = highest_moving_node - lowest_moving_node;
    // number of nodes that will need to be moved
    // Find the uppermost node that needs to move up
    while (tree->node_array[highest_moving_node + 1].time <=
               k + num_moving_nodes &&
           highest_moving_node + 1 <= num_nodes - 1) {
        highest_moving_node++;
        num_moving_nodes++;
    }
    // Update times of nodes (moving_node) between i and highest_moving_node to
    // k+moving_node-i
    for (long moving_node = lowest_moving_node;
         moving_node <= highest_moving_node;
         moving_node++) {  // Do all required length moves
        num_moves += k + moving_node - lowest_moving_node -
                     tree->node_array[moving_node].time;
        tree->node_array[moving_node].time =
            k + moving_node - lowest_moving_node;
    }
    return num_moves;
}

// Compute Tree_Array of all RNNI neighbours
Tree_Array rnni_neighbourhood(Tree* tree) {
    long num_leaves = tree->num_leaves;
    long num_nodes = 2 * num_leaves - 1;
    long max_nh_size = 2 * (num_leaves - 1);

    Tree_Array neighbour_array = get_empty_tree_array(max_nh_size, num_leaves);
    long index = 0;  // index to the currently last element in neighbour_array

    // Deep copy input tree to get neighbouring trees
    Tree* neighbour = new_tree_copy(tree);
    Tree* next_neighbour_array;
    next_neighbour_array = &neighbour_array.trees[index];

    // Loop through all possible ranks on which moves can happen
    // 'ranks' here means position in node array, where the first n entries are
    // leaves
    for (long r = num_leaves; r < num_nodes - 1; r++) {
        if (tree->node_array[r].parent != r + 1) {
            // no edge -> rank move:
            rank_move(neighbour, r);
            copy_tree(next_neighbour_array, neighbour);
            index++;
            next_neighbour_array = &neighbour_array.trees[index];
            copy_tree(neighbour, tree);
        } else {
            // edge -> 2 NNI moves
            for (long child_moves_up = 0; child_moves_up < 2;
                 child_moves_up++) {
                nni_move(neighbour, r, child_moves_up);
                copy_tree(next_neighbour_array, neighbour);
                index++;
                next_neighbour_array = &neighbour_array.trees[index];
                copy_tree(neighbour, tree);
            }
        }
    }
    neighbour_array.num_trees = index;
    free_tree(neighbour);
    return (neighbour_array);
}

// Compute Tree_Array of all rank neighbours
Tree_Array rank_neighbourhood(Tree* tree) {
    long num_leaves = tree->num_leaves;
    long max_nh_size = num_leaves - 1;

    Tree_Array neighbour_array = get_empty_tree_array(max_nh_size, num_leaves);
    Tree* neighbour = new_tree_copy(tree);

    // index to the currently last element in neighbour_array
    long index = 0;
    Tree* next_neighbour_array_tree;
    next_neighbour_array_tree = &neighbour_array.trees[index];
    for (long r = num_leaves; r < 2 * num_leaves - 2; r++) {
        // Check if we can do rank move:
        if (tree->node_array[r].parent != r + 1) {
            rank_move(neighbour, r);
            copy_tree(next_neighbour_array_tree, neighbour);
            index++;
            // reset neighbour & update pointer to next neighbour in array
            copy_tree(neighbour, tree);
            next_neighbour_array_tree = &neighbour_array.trees[index];
        }
    }
    neighbour_array.num_trees = index;
    free_tree(neighbour);
    return (neighbour_array);
}

// Perform a random RNNI move (at uniform) on tree
int uniform_neighbour(Tree* tree) {
    long num_leaves = tree->num_leaves;
    long num_nodes = 2 * num_leaves - 1;
    long num_moves = 0;
    long max_nh_size = 2 * (num_leaves - 1);

    // moves are saved in matrix -- ith move in move_array[i]
    // move_array[i][0] -- rank of lower node of edge for move
    // move_array[i][1] -- 0 for rank move
    // 1 for NNI move with children[0] moves up
    // 2 for NNI move with children[1] moves up
    int** move_array = calloc(max_nh_size, sizeof(int*));
    for (long i = 0; i < max_nh_size; i++) {
        move_array[i][0] = -1;  // lower node of edge for move
        move_array[i][1] = -1;  // rank vs nni move
    }
    // Fill move array
    for (long i = num_leaves; i < num_nodes; i++) {
        if (tree->node_array[i].parent == i + 1) {
            move_array[num_moves][0] = i;
            move_array[num_moves][1] = 1;  // NNI move 0
            move_array[num_moves + 1][0] = i;
            move_array[num_moves + 1][1] = 2;  // NNI move 1
            num_moves += 2;
        } else {
            move_array[num_moves][0] = i;
            move_array[num_moves][1] = 0;  // rank move is 0
            num_moves += 1;
        }
    }

    // Pick random move
    long r = rand() % (num_moves - 1);
    if (move_array[r][1] == 0) {
        rank_move(tree, move_array[r][0]);
    } else if (move_array[r][1] == 1) {
        nni_move(tree, move_array[r][0], 0);
    } else {
        nni_move(tree, move_array[r][0], 1);
    }
    // free move_array
    free(move_array);
    return EXIT_SUCCESS;
}

// decrease the mrca of node1 and node2 in tree by a (unique) RNNI move
// returns 0 if rank move was done
// returns 1 if NNI move moving children[0] up
// returns 2 if NNI move moving children[1] up
int decrease_mrca(Tree* tree, long node1, long node2) {
    // return value:
    int move_type;
    long current_mrca = mrca(tree, node1, node2);
    // deep copy tree
    Tree* neighbour = new_tree_copy(tree);
    if (neighbour->node_array[current_mrca - 1].parent == current_mrca) {
        // we try both possible NNI move and see which one decreases the rank of
        // the mrca
        move_type = 1;
        nni_move(neighbour, current_mrca - 1, 0);
        if (mrca(neighbour, node1, node2) >= current_mrca) {
            // we did not decrease the rank of the mrca by this nni move, so we
            // need to do the other one but first we need to reset neighbour to
            // tree:
            copy_tree(neighbour, tree);
            nni_move(neighbour, current_mrca - 1, 1);
            move_type = 2;
        }
    } else {  // otherwise, we make a rank move
        rank_move(neighbour, current_mrca - 1);
        move_type = 0;
    }
    // now update tree to become neighbour
    copy_tree(tree, neighbour);
    free_tree(neighbour);
    return move_type;
}

// FINDPATH. returns a shortest RNNI path in matrix representation:
// each row of path is move
// path[i][0]: rank of lower node bounding the interval of move i
// path[i][1]:
// 0 -> rank move
// 1 -> nni where children[0] moves up (becomes child of node at rank
// path[i][0]+1) 2 -> nni where children[1] moves up (becomes child of node at
// rank path[i][0]+1)
Path findpath_moves(Tree* start_tree, Tree* dest_tree) {
    long num_leaves = start_tree->num_leaves;
    long num_nodes = 2 * num_leaves - 1;
    long max_dist = ((num_leaves - 1) * (num_leaves - 2)) / 2;
    Path path;
    path.moves = malloc((max_dist + 1) * sizeof(long*));
    for (long i = 0; i < max_dist + 1; i++) {
        path.moves[i] = malloc(2 * sizeof(long));
        path.moves[i][0] = 0;
        path.moves[i][1] = 0;
    }

    if (start_tree->num_leaves != dest_tree->num_leaves) {
        printf("Error. The input trees have different numbers of leaves.\n");
        return path;
    }
    long path_index =
        0;  // next position on path that we want to fill with a tree pointer
    long current_mrca;  // rank of the mrca that needs to be moved down
    Tree* current_tree = new_tree_copy(start_tree);
    // loop through internal nodes, construct cluster of node at position i in
    // iteration i
    for (long i = num_leaves; i < num_nodes; i++) {
        current_mrca = mrca(current_tree, dest_tree->node_array[i].children[0],
                            dest_tree->node_array[i].children[1]);
        // decreases current_mrca until it becomes i
        while (current_mrca != i) {
            path.moves[path_index][0] = current_mrca - 1;
            path.moves[path_index][1] = decrease_mrca(
                current_tree, dest_tree->node_array[i].children[0],
                dest_tree->node_array[i].children[1]);
            path_index++;
            current_mrca--;
        }
    }
    free_tree(current_tree);
    path.length = path_index;
    return path;
}

// FINDPATH without saving the path -- returns only the distance
long rnni_distance(Tree* start_tree, Tree* dest_tree) {
    long num_leaves = start_tree->num_leaves;
    long num_nodes = 2 * num_leaves - 1;
    long path_length = 0;
    if (dest_tree->num_leaves != start_tree->num_leaves) {
        printf("Error. The input trees have different numbers of leaves.\n");
        return EXIT_FAILURE;
    }
    long current_mrca_rank;  // rank of the mrca that needs to be moved down
    Tree* current_tree = new_tree_copy(start_tree);
    // loop through internal nodes, construct cluster of node at position i in
    // iteration i
    for (long i = num_leaves; i < num_nodes; i++) {
        // if needed: length moves moving all nodes up that shouldn't be below
        // node i in dest_tree (this cannot happen in RNNI)
        if (current_tree->node_array[i].time < dest_tree->node_array[i].time) {
            path_length +=
                move_up(current_tree, i, dest_tree->node_array[i].time);
        }
        // find mrca of children of currently considered node (i) -> current
        // mrca
        current_mrca_rank =
            mrca(current_tree, dest_tree->node_array[i].children[0],
                 dest_tree->node_array[i].children[1]);
        Node* current_mrca;
        current_mrca = &current_tree->node_array[current_mrca_rank];
        Node* node_below_current_mrca;  // node with rank one less than
                                        // current_mrca
        node_below_current_mrca =
            &current_tree->node_array[current_mrca_rank - 1];
        // decrease time of current_mrca until it reaches the time it has in
        // dest_tree
        while (current_mrca->time != dest_tree->node_array[i].time) {
            // first length moves (if needed) to decrease time of current_mrca
            if (node_below_current_mrca->time < current_mrca->time - 1) {
                // check if current_mrca needs to move past
                // node_below_current_mrca if so, we need to move current_mrca
                // down to node_below_current_mrca and then do RNNI moves
                if (node_below_current_mrca->time + 1 >
                    dest_tree->node_array[i].time) {
                    path_length += current_mrca->time -
                                   (node_below_current_mrca->time + 1);
                    current_mrca->time = node_below_current_mrca->time + 1;
                } else {
                    // in this case we move the node i to its final position
                    path_length +=
                        current_mrca->time - dest_tree->node_array[i].time;
                    current_mrca->time = dest_tree->node_array[i].time;
                    break;
                }
            }
            // now one RNNI move
            decrease_mrca(current_tree, dest_tree->node_array[i].children[0],
                          dest_tree->node_array[i].children[1]);
            current_mrca_rank--;
            current_mrca = &current_tree->node_array[current_mrca_rank];
            path_length++;
        }
    }
    free_tree(current_tree);
    return path_length;
}

// returns the FINDPATH path between two given given trees as Tree_Array
// (i) runs findpath and (ii) translates path matrix to actual trees on path
Tree_Array findpath(Tree* start_tree, Tree* dest_tree) {
    long num_leaves = start_tree->num_leaves;
    Path fp = findpath_moves(start_tree, dest_tree);

    Tree_Array findpath_array = get_empty_tree_array(fp.length + 1, num_leaves);
    Tree* next_findpath_tree;

    Tree* current_tree;
    current_tree = new_tree_copy(start_tree);

    // create actual path by doing moves starting at start_tree
    // decoding the information in the matrix returned from fp
    for (long i = 0; i < fp.length; i++) {
        next_findpath_tree = &findpath_array.trees[i];
        copy_tree(next_findpath_tree, current_tree);
        if (fp.moves[i][1] == 0) {
            rank_move(current_tree, fp.moves[i][0]);
        } else {
            nni_move(current_tree, fp.moves[i][0], fp.moves[i][1] - 1);
        }
    }
    // add last tree
    next_findpath_tree = &findpath_array.trees[fp.length];
    copy_tree(next_findpath_tree, current_tree);

    for (int i = 0; i < fp.length + 1; i++) {
        free(fp.moves[i]);
    }
    free(fp.moves);
    free_tree(current_tree);
    return findpath_array;
}
