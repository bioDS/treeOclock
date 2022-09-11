/*Efficient implementation of FINDPATH on ranked trees*/

#include "rnni.h"


// NNI move on edge bounded by rank and rank + 1, moving child_moves_up (index) of the lower node up
int nni_move(Tree* tree, long rank, int child_moves_up){
    Node* upper_node;
    upper_node = &tree->node_array[rank + 1];
    Node* lower_node;
    lower_node = &tree->node_array[rank];
    if(lower_node->parent != rank + 1){
        printf("Can't do an NNI - interval [%ld, %ld] is not an edge!\n", rank, rank + 1);
        return EXIT_FAILURE;
    }
    int child_moved_up;
    for (int i = 0; i < 2; i++){
        if (upper_node->children[i] != rank){ //find the child of the node of rank k+1 that is not the node of rank k
            // update parent/children relations to get nni neighbour
            tree->node_array[upper_node->children[i]].parent = rank;
            tree->node_array[lower_node->children[child_moves_up]].parent = rank+1;
            child_moved_up = lower_node->children[child_moves_up];
            lower_node->children[child_moves_up] = upper_node->children[i];
            upper_node->children[i] = child_moved_up;
        }
    }
    return EXIT_SUCCESS;
}


// Make a rank move on tree between nodes of rank and rank + 1 (if possible)
int rank_move(Tree* tree, long rank){
    if (tree->node_array[rank].parent == rank + 1){
        printf("Error. No rank move possible. The interval [%ld,%ld] is an edge!\n", rank, rank + 1);
        return EXIT_FAILURE;
    }
    Node* upper_node;
    upper_node = &tree->node_array[rank + 1];
    Node* lower_node;
    lower_node = &tree->node_array[rank];

    // update parents of nodes that swap ranks
    long upper_parent;
    upper_parent = upper_node->parent;
    upper_node->parent = lower_node->parent;
    lower_node->parent = upper_parent;


    for (int i = 0; i < 2; i++){
        // update children of nodes that swap ranks
        long upper_child = upper_node->children[i];
        upper_node->children[i] = lower_node->children[i];
        lower_node->children[i] = upper_child;
        // update parents of children of nodes that swap ranks
        tree->node_array[upper_node->children[i]].parent ++; 
        tree->node_array[lower_node->children[i]].parent --;
    }
    for (int i = 0; i < 2; i ++){
        // update children of parents of nodes that swap rank
        if (upper_node->parent == lower_node->parent){
            break;
        }
        if (tree->node_array[upper_node->parent].children[i] == rank){
            tree->node_array[upper_node->parent].children[i] ++;
        }
        if (tree->node_array[lower_node->parent].children[i] == rank + 1){
            tree->node_array[lower_node->parent].children[i] --;
        }
    }
    return EXIT_SUCCESS;
}


// decrease the mrca of node1 and node2 in tree by a (unique) RNNI move
// returns 0 if rank move was done
// returns 1 if NNI move moving children[0] up
// returns 2 if NNI move moving children[1] up
int decrease_mrca(Tree* tree, long node1, long node2){
    // return value:
    int move_type;
    long current_mrca = mrca(tree, node1, node2);
    // deep copy tree
    Tree* neighbour = new_tree_copy(tree);
    if (neighbour->node_array[current_mrca-1].parent == current_mrca){
        // we try both possible NNI move and see which one decreases the rank of the mrca
        move_type = 1;
        nni_move(neighbour, current_mrca-1, 0);
        if(mrca(neighbour,node1,node2) >= current_mrca){
            // we did not decrease the rank of the mrca by this nni move, so we need to do the other one
            // but first we need to reset neighbour to tree:
            copy_tree(neighbour, tree);
            nni_move(neighbour, current_mrca-1, 1);
            move_type = 2;
        }
    } else{ // otherwise, we make a rank move
        rank_move(neighbour, current_mrca - 1);
        move_type = 0;
    }
    // now update tree to become neighbour
    copy_tree(tree, neighbour);
    free_tree(neighbour);
    return move_type;
}


// Move up internal nodes that are at position >i in node list so that there are no nodes with rank less than k in the tree at the end (i.e. length moves that move nodes up -- see pseudocode FindPath^+)
int move_up(Tree* tree, long lowest_moving_node, long k){
    long num_nodes = 2 * tree->num_leaves - 1;
    long num_moves = 0; // counter for the number of moves that are necessary
    long highest_moving_node = lowest_moving_node;
    // Find highest j that needs to be moved up -- maximum is reached at root!
    while (tree->node_array[highest_moving_node+1].time <= k && highest_moving_node+1 <= num_nodes - 1){
        highest_moving_node ++;
    }
    long num_moving_nodes = highest_moving_node - lowest_moving_node;
    // number of nodes that will need to be moved
    // Find the uppermost node that needs to move up
    while (tree->node_array[highest_moving_node+1].time <= k+num_moving_nodes && highest_moving_node+1 <= num_nodes - 1){
        highest_moving_node++;
        num_moving_nodes++;
    }
    // Update times of nodes (moving_node) between i and highest_moving_node to k+moving_node-i
    for (long moving_node = lowest_moving_node; moving_node <= highest_moving_node; moving_node++){ // Do all required length moves
        num_moves += k + moving_node - lowest_moving_node - tree->node_array[moving_node].time;
        tree->node_array[moving_node].time = k + moving_node - lowest_moving_node;
    }
    return num_moves;
}


// Compute Tree_Array of all RNNI neighbours
Tree_Array rnni_neighbourhood(Tree* tree){
    long num_leaves = tree->num_leaves;
    long num_nodes = 2 * num_leaves - 1;
    long max_nh_size = 2 * (num_leaves  - 1);

    Tree_Array neighbour_array = empty_tree_array(max_nh_size, num_leaves);
    long index = 0; //index to the currently last element in neighbour_array

    //Deep copy input tree to get neighbouring trees
    Tree* neighbour = new_tree_copy(tree);
    Tree* next_neighbour_array;
    next_neighbour_array = &neighbour_array.trees[index];

    // Loop through all possible ranks on which moves can happen ('ranks' here means position in node list, where the first n entries are leaves)
    for (long r=num_leaves; r < num_nodes - 1; r++){
        if (tree->node_array[r].parent != r+1){
            // rank move:
            rank_move(neighbour, r);
            copy_tree(next_neighbour_array, neighbour);
            index++;
            next_neighbour_array = &neighbour_array.trees[index];
            copy_tree(neighbour, tree);
        } else{
            // 2 NNI moves
            for (long child_moves_up=0; child_moves_up<2; child_moves_up ++){
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
    return(neighbour_array);
}


// Compute Tree_Array of all rank neighbours
Tree_Array rank_neighbourhood(Tree* tree){
    long num_leaves = tree->num_leaves;
    long max_nh_size = num_leaves - 1;

    Tree_Array neighbour_array = empty_tree_array(max_nh_size, num_leaves);
    Tree* neighbour = new_tree_copy(tree);

    //index to the currently last element in neighbour_array
    long index = 0;
    Tree* next_neighbour_array_tree;
    next_neighbour_array_tree = &neighbour_array.trees[index];
    for (long r=num_leaves; r < 2 * num_leaves-2; r++){
        // Check if we can do rank move:
        if (tree->node_array[r].parent != r+1){
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
    return(neighbour_array);
}


// Perform a random RNNI move (at uniform) on tree
int uniform_neighbour(Tree* tree){
    long num_leaves = tree->num_leaves;
    long num_nodes = 2 * num_leaves - 1;
    long num_moves = 0;
    long max_nh_size = 2 * (num_leaves - 1);

    // moves are saved in matrix -- ith move in move_list[i]
    // move_list[i][0] -- rank of lower node of edge for move
    // move_list[i][1] -- 0 for rank move
    // 1 for NNI move with children[0] moves up
    // 2 for NNI move with children[1] moves up
    int ** move_list = calloc(max_nh_size, sizeof(int*));
    for (long i = 0; i < max_nh_size; i++){
        move_list[i][0] = -1; // lower node of edge for move
        move_list[i][1] = -1; // rank vs nni move
    }
    // Fill move list
    for (long i = num_leaves; i < num_nodes; i++){
        if (tree->node_array[i].parent == i+1){
            move_list[num_moves][0] = i;
            move_list[num_moves][1] = 1; // NNI move 0
            move_list[num_moves + 1][0] = i;
            move_list[num_moves + 1][1] = 2; // NNI move 1
            num_moves += 2;
        } else{
            move_list[num_moves][0] = i;
            move_list[num_moves][1] = 0; // rank move is 0
            num_moves += 1;
        }
    }

    // Pick random move
    long r = rand() % (num_moves-1);
    if (move_list[r][1] == 0){
        rank_move(tree, move_list[r][0]);
    } else if (move_list[r][1] == 1){
        nni_move(tree, move_list[r][0], 0);
    } else{
        nni_move(tree, move_list[r][0], 1);
    }
    // free move_list
    free(move_list);
    return EXIT_SUCCESS;
}


// FINDPATH. returns a shortest RNNI path in matrix representation:
// each row of path is move
// path[i][1]: rank of lower node bounding the interval of move i
// path[i][1]: 0,1,2: rank move, nni where children[0] stays, nni where children[1] stays
Path findpath(Tree* start_tree, Tree* dest_tree){
    long num_leaves = start_tree->num_leaves;
    long num_nodes = 2 * num_leaves - 1;
    long max_dist = ((num_leaves - 1) * (num_leaves - 2))/2;
    Path path;
    path.moves = malloc((max_dist + 1) * sizeof(long*));
    for (long i = 0; i < max_dist + 1; i++){
        path.moves[i] = malloc(2 * sizeof(long));
        path.moves[i][0] = 0;
        path.moves[i][1] = 0;
    }

    if (start_tree->num_leaves != dest_tree->num_leaves){
        printf("Error. The input trees have different numbers of leaves.\n");
    } else{
        long path_index = 0; // next position on path that we want to fill with a tree pointer
        long current_mrca; //rank of the mrca that needs to be moved down
        Tree* current_tree = new_tree_copy(start_tree);
        for (long i = num_leaves; i < num_nodes; i++){
            current_mrca = mrca(current_tree, dest_tree->node_array[i].children[0], dest_tree->node_array[i].children[1]);
            // move current_mrca down
            while(current_mrca != i){
                path.moves[path_index][0] = current_mrca-1;
                path.moves[path_index][1] = decrease_mrca(current_tree, dest_tree->node_array[i].children[0], dest_tree->node_array[i].children[1]);
                path_index++;
                current_mrca--;
            }
        }
        free_tree(current_tree);
        path.length = path_index;
    }
    return path;
}


// FINDPATH without saving the path -- returns only the distance
long rnni_distance(Tree* start_tree, Tree* dest_tree){
    long num_leaves = start_tree->num_leaves;
    long num_nodes = 2 * num_leaves - 1;
    long path_length = 0;
    if (dest_tree->node_array == NULL){
        printf("Error. Destination tree doesn't exist.\n");
        return EXIT_FAILURE;
    }
    long current_mrca_rank; //rank of the mrca that needs to be moved down
    Tree* current_tree = new_tree_copy(start_tree);
    for (long i = num_leaves; i < num_nodes; i++){
        if (current_tree->node_array[i].time < dest_tree->node_array[i].time){
            path_length += move_up(current_tree, i, dest_tree->node_array[i].time);
        }
        // we now need to find the current MRCA and decrease its time in the tree
        current_mrca_rank = mrca(current_tree, dest_tree->node_array[i].children[0], dest_tree->node_array[i].children[1]);
        Node* current_mrca;
        current_mrca = &current_tree->node_array[current_mrca_rank];
        while(current_mrca->time != dest_tree->node_array[i].time){
            if (current_tree->node_array[current_mrca_rank-1].time < current_mrca->time - 1){
                // First length moves (if necessary)
                if( current_tree->node_array[current_mrca_rank-1].time + 1 > dest_tree->node_array[i].time){
                    path_length += current_mrca->time - (current_tree->node_array[current_mrca_rank-1].time + 1);
                    current_mrca->time = current_tree->node_array[current_mrca_rank-1].time + 1;
                } else{
                    // in this case we move the node i to its final position
                    path_length += current_mrca->time - dest_tree->node_array[i].time;
                    current_mrca->time = dest_tree->node_array[i].time;
                    break;
                }
            }
            // now RNNI move
            decrease_mrca(current_tree, dest_tree->node_array[i].children[0], dest_tree->node_array[i].children[1]);
            current_mrca_rank--;
            current_mrca = &current_tree->node_array[current_mrca_rank];
            path_length++;
        }
    }
    // TODO: Add free_tree function
    free_tree(current_tree);
    return path_length;
}


// returns the FINDPATH path between two given given trees as Tree_Array -- runs findpath and translates path matrix to actual trees on path
Tree_Array return_findpath(Tree* start_tree, Tree* dest_tree){
    long num_leaves = start_tree->num_leaves;
    Path fp = findpath(start_tree, dest_tree);

    Tree_Array findpath_list = empty_tree_array(fp.length + 1, num_leaves);
    Tree* next_findpath_tree;

    Tree* current_tree;
    current_tree = new_tree_copy(start_tree);

    // create actual path by doing moves starting at start_tree with the information in the matrix returned form fp above
    for (long i = 0; i < fp.length; i++){
        next_findpath_tree = &findpath_list.trees[i];
        copy_tree(next_findpath_tree, current_tree);
        if (fp.moves[i][1] == 0){
            rank_move(current_tree, fp.moves[i][0]);
        }
        else{
            nni_move(current_tree, fp.moves[i][0], fp.moves[i][1]-1);
        }
    }
    // add last tree
    next_findpath_tree = &findpath_list.trees[fp.length];
    copy_tree(next_findpath_tree, current_tree);

    for (int i = 0; i < fp.length + 1; i++){
        free(fp.moves[i]);
    }
    free(fp.moves);
    free_tree(current_tree);
    return findpath_list;
}


// Perform a series of k random RNNI moves to receive a random walk in RNNI, starting at input tree
long random_walk(Tree* tree, long k){
    Tree* current_tree = new_tree_copy(tree);
    for (long i = 0; i < k; i++){
        uniform_neighbour(current_tree);
    }
    long distance = rnni_distance(current_tree, tree);
    free_tree(current_tree);
    return(distance);
}


// TODO: Do we need this function here?
// perform one iteration of FP on every tree in tree_list, such that resulting tree has mrca of i and j at position r
// Note that this may change every tree in treelist
int first_iteration(Tree_Array* tree_array, long node1, long node2, long r){
    for(long i = 0; i < tree_array->num_trees; i++){
        while(mrca(&tree_array->trees[i], node1, node2) > r){
            decrease_mrca(&tree_array->trees[i], node1, node2);
        }
    }
    return 0;
}

// TODO: Do we need this function here?
long sos(Tree_Array* tree_array, Tree* focal_tree){
    // compute sum of squared distances for all tree in tree_array to focal_tree
    long sos = 0;
    for(long i = 0; i < tree_array->num_trees; i++){
        sos += rnni_distance(&tree_array->trees[i], focal_tree);
    }
    return sos;
}
