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
    if (input_tree->tree == NULL){
        // printf("Error. No SPR move possible. Given tree doesn't exist.\n");
        return 1;
    }
    if (r > new_parent || input_tree->tree[new_parent].children[new_child_index] > r){
        // printf("Error. No SPR move possible, as destination edge does not cover rank r.\n");
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
            // printf("sibling %ld\n", new_sibling);
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
    // printf("number of neighbours: %ld\n", index);
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
    // printf("number of neighbours: %ld\n", index);
    free(neighbour);
    return(neighbour_list);
}



Tree_List rankedspr_path_mrca_cluster_diff(Tree* start_tree, Tree* dest_tree, int hspr){
    // compute a path between start_tree and dest_tree (approximation for shortest path)
    // this approach uses tree search by optimising the sum of mrca differences + symmetric cluster differences
    long num_leaves = start_tree->num_leaves;

    // Initialise output path
    Tree_List path; // output: list of trees on FP path
    path.num_trees = 0.5 * (num_leaves-1) * (num_leaves-2) + 1; //diameter of rankedspr is less than quadratic
    path.trees = malloc(path.num_trees * sizeof(Tree));
    for (long i = 0; i < path.num_trees; i++){
        path.trees[i].num_leaves = num_leaves;
        path.trees[i].tree = malloc((2* num_leaves - 1) * sizeof(Node));
    }
    // current path index (i.e. current path length)
    long index = 0;

    //Deep copy start tree to get new tree to be added to path iteratively
    Tree* current_tree = malloc(sizeof(Node*) + 3 * sizeof(long));
    current_tree->num_leaves = num_leaves;
    current_tree->tree = malloc((2 * num_leaves - 1) * sizeof(Node)); // deep copy start tree
    for (long i = 0; i < 2 * num_leaves - 1; i++){
        current_tree->tree[i] = start_tree->tree[i];
    }
    // Add the first tree to output path
    for (long j = 0; j < 2 * num_leaves - 1; j++){
        path.trees[index].tree[j] = current_tree->tree[j];
    }
    index+=1;

    // long diff = sum_symmetric_cluster_diff(current_tree, dest_tree);
    long diff = mrca_differences(current_tree, dest_tree, TRUE) + sum_symmetric_cluster_diff(current_tree, dest_tree);
    while (diff > 0){
        Tree_List neighbours = all_spr_neighbourhood(current_tree, hspr);
        for (long i = 0; i < neighbours.num_trees; i++){
            Tree* neighbour_pointer;
            neighbour_pointer = &neighbours.trees[i];
            long new_diff =  mrca_differences(neighbour_pointer, dest_tree, TRUE) + sum_symmetric_cluster_diff(neighbour_pointer, dest_tree);
            // long new_diff = sum_symmetric_cluster_diff(neighbour_pointer, dest_tree);
            // printf("mrca_diff: %ld\n", new_mrca_diff);
            if (new_diff < diff){
                diff = new_diff;
                // update current_tree and add it to path list
                for (long j = 0; j < 2 * num_leaves - 1; j++){
                    current_tree->tree[j] = neighbour_pointer->tree[j];
                }
            }
        }
        for (long j = 0; j < 2 * num_leaves - 1; j++){
            path.trees[index].tree[j] = current_tree->tree[j];
        }
        index += 1;
    }
    free(current_tree);
    path.num_trees = index;
    return(path);
}


Tree_List rankedspr_path_mrca_diff(Tree* start_tree, Tree* dest_tree, int hspr){
    // compute a path between start_tree and dest_tree (approximation for shortest path)
    // this approach uses tree search by optimising the sum of mrca differences
    long num_leaves = start_tree->num_leaves;

    // Initialise output path
    Tree_List path; // output: list of trees on FP path
    path.num_trees = 0.5 * (num_leaves-1) * (num_leaves-2) + 1; //diameter of rankedspr is less than quadratic
    path.trees = malloc(path.num_trees * sizeof(Tree));
    for (long i = 0; i < path.num_trees; i++){
        path.trees[i].num_leaves = num_leaves;
        path.trees[i].tree = malloc((2* num_leaves - 1) * sizeof(Node));
    }
    // current path index (i.e. current path length)
    long index = 0;

    //Deep copy start tree to get new tree to be added to path iteratively
    Tree* current_tree = malloc(sizeof(Node*) + 3 * sizeof(long));
    current_tree->num_leaves = num_leaves;
    current_tree->tree = malloc((2 * num_leaves - 1) * sizeof(Node)); // deep copy start tree
    for (long i = 0; i < 2 * num_leaves - 1; i++){
        current_tree->tree[i] = start_tree->tree[i];
    }
    // Add the first tree to output path
    for (long j = 0; j < 2 * num_leaves - 1; j++){
        path.trees[index].tree[j] = current_tree->tree[j];
    }
    index+=1;

    long mrca_diff = mrca_differences(current_tree, dest_tree, FALSE);
    while (mrca_diff > 0){
        Tree_List neighbours = all_spr_neighbourhood(current_tree, hspr);
        for (long i = 0; i < neighbours.num_trees; i++){
            Tree* neighbour_pointer;
            neighbour_pointer = &neighbours.trees[i];
            long new_mrca_diff =  mrca_differences(neighbour_pointer, dest_tree, FALSE);
            // printf("mrca_diff: %ld\n", new_mrca_diff);
            if (new_mrca_diff < mrca_diff){
                mrca_diff = new_mrca_diff;
                // update current_tree and add it to path list
                for (long j = 0; j < 2 * num_leaves - 1; j++){
                    current_tree->tree[j] = neighbour_pointer->tree[j];
                }
            }
        }
        for (long j = 0; j < 2 * num_leaves - 1; j++){
            path.trees[index].tree[j] = current_tree->tree[j];
        }
        index += 1;
    }
    free(current_tree);
    path.num_trees = index;
    return(path);
}


Tree_List rankedspr_path_rnni_mrca_diff(Tree* start_tree, Tree* dest_tree, int rank){
    // approximate the beginning of a shortest RPSR path between start_tree and dest_tree that consists of RNNI moves only 
    // We only do an RNNI move if it does not increase the rank difference of any mrcas or parents of leaves when considering clusters in dest_tree
    long num_leaves = start_tree->num_leaves;

    // Initialise output path
    Tree_List path; // output: list of trees on FP path
    path.num_trees = 0.5 * (num_leaves-1) * (num_leaves-2) + 3; //diameter of rankedspr is less than quadratic
    path.trees = malloc(path.num_trees * sizeof(Tree));
    for (long i = 0; i < path.num_trees; i++){
        path.trees[i].num_leaves = num_leaves;
        path.trees[i].tree = malloc((2* num_leaves - 1) * sizeof(Node));
    }
    // current path index (i.e. current path length)
    long index = 0;

    //Deep copy start tree to get new tree to be added to path iteratively
    Tree* current_tree = malloc(sizeof(Node*) + 3 * sizeof(long));
    current_tree->num_leaves = num_leaves;
    current_tree->tree = malloc((2 * num_leaves - 1) * sizeof(Node)); // deep copy start tree
    for (long i = 0; i < 2 * num_leaves - 1; i++){
        current_tree->tree[i] = start_tree->tree[i];
    }
    // Add the first tree to output path
    for (long j = 0; j < 2 * num_leaves - 1; j++){
        path.trees[index].tree[j] = current_tree->tree[j];
    }
    index+=1;

    int change = 0; //indicates whether we could improve the mrca in the previous iteration (0: yes, 1: no); this will turn to 1 once we are done with RNNI moves (on a path to dest_tree we assume that only HSPR moves follow)
    while (change == 0){
        Tree_List neighbours;
        if (rank == 0){
            neighbours = rank_neighbourhood(current_tree);
        } else{
            neighbours = rnni_neighbourhood(current_tree);
        }

        if (neighbours.num_trees == 0){
            break;
        }

        long * current_mrcas = mrca_array(current_tree, dest_tree); // get mrca list for current tree to be able to compare mrcas to neighbours

        for (long i = 0; i < neighbours.num_trees; i++){
            change = 0;
            Tree* neighbour_pointer;
            neighbour_pointer = &neighbours.trees[i];
            long *neighbour_mrcas = mrca_array(neighbour_pointer, dest_tree);
            // test for every tree in one neighbourhood if the rank difference of a parent of a leaf or an mrca gets worse
            for (long j = 0; j < 2 * num_leaves - 1; j++){
                if (j >= num_leaves){
                    // printf("j : %ld, current_mrcas[j]: %ld, neighbour_mrcas[j]: %ld\n", j, current_mrcas[j], neighbour_mrcas[j]);
                }
                if (j < num_leaves){
                    // printf("current tree parent: %ld, neighbour parent: %ld, dest_tree parent: %ld rank: %ld\n", current_tree->tree[j].parent, neighbour_pointer->tree[j].parent, dest_tree->tree[j].parent, j);
                }
                if (j < num_leaves && abs(dest_tree->tree[j].parent-current_tree->tree[j].parent) < abs(dest_tree->tree[j].parent-neighbour_pointer->tree[j].parent)){
                    // if there is a leaf whose parent gets moved further away from where it is in the destination tree, compared to current_tree, then this neighbour is not chosen for our path
                    change = 1;
                    break;
                }
                else if (j >= num_leaves && abs((j-num_leaves) - current_mrcas[j])< abs((j-num_leaves) - neighbour_mrcas[j])){
                    // check if any of the mrcas have bigger rank difference in neighbour&dest_tree than in current_tree&dest_tree
                    // if they do, then we do not add this neighbour to the shortest path
                    change = 1;
                    break;
                }
            }
            if (change == 0){
                // We only get here if the neighbour is an improvement over current_tree
                // deep copy neighbouring tree to become current_tree
                for (long i = 0; i < 2 * num_leaves - 1; i++){
                    current_tree->tree[i] = neighbour_pointer->tree[i];
                }
                for (long j = 0; j < 2 * num_leaves - 1; j++){
                    path.trees[index].tree[j] = current_tree->tree[j];
                }
                index += 1;
                break; // no need to look at further neighbours
            }
        }
        if (same_tree(current_tree, dest_tree)==0){
            free(current_tree);
            path.num_trees = index;
            return(path);
        }
    }
    free(current_tree);
    path.num_trees = index;
    return(path);
}


// returns length of the path computed by tree search in neighbourhoods (BFS), restricting neighbourhoods to use a bottom up approach like FP, and only moves involving nodes of the currently considered cluster in dest_tree
// more detailed description of this algorithm can be found in git repo rankedSPR_paper.
// uses RSPR space if hspr = 1, otherwise HSPR space (in which case no rank move is possible)
long rankedspr_path_restricting_neighbourhood(Tree* start_tree, Tree* dest_tree, int hspr){
    long output = 0; //length of the path that is being computed in this function
    // compute a path between start_tree and dest_tree (approximation for shortest path)
    // this approach uses tree search by only considering a few specific neighbours to the current tree
    long num_leaves = start_tree->num_leaves;
    // array containing at position i the number of trees in i-neighbourhood whose neighbours have already been added to the queue -- needed to derive the distance in the end.
    long* visited_at_distance = calloc(num_leaves * num_leaves, sizeof(long)); // not sure if this is correct
    visited_at_distance[0] = 1;
    // Check starting_tree:
    // for (long i = 0; i < 2*num_leaves - 1; i++){
    //     printf("children: %ld, %ld, parent: %ld\n", start_tree->tree[i].children[0], start_tree->tree[i].children[1], start_tree->tree[i].parent);
    // }

    // Check if start_tree = dest_tree (if so, we output distance 0)
    int found = 0;
    for (long j = 0; j < 2*num_leaves - 1; j++){
        // printf("current_parent: %ld, dest_parent: %ld\n", neighbours.trees[i].tree[j].parent, dest_tree->tree[j].parent);
        if (start_tree->tree[j].parent != dest_tree->tree[j].parent){
            found = 1;
        }
    }
    if(found ==0){
        return(0);
    }

    // // Initialise output path
    // Tree_List path; // output: list of trees on FP path
    // path.num_trees = 0.5 * (num_leaves-1) * (num_leaves-2) + 1; //diameter of rankedspr is less than quadratic
    // path.trees = malloc(path.num_trees * sizeof(Tree));
    // for (long i = 0; i < path.num_trees; i++){
    //     path.trees[i].num_leaves = num_leaves;
    //     path.trees[i].tree = malloc((2* num_leaves - 1) * sizeof(Node));
    // }

    // current path index (i.e. current path length)
    long index = 0;

    //Deep copy start tree to get new tree to be added to path iteratively
    Tree* current_tree = malloc(sizeof(Node*) + 3 * sizeof(long));
    current_tree->num_leaves = num_leaves;
    current_tree->tree = malloc((2 * num_leaves - 1) * sizeof(Node)); // deep copy start tree
    for (long i = 0; i < 2 * num_leaves - 1; i++){
        current_tree->tree[i] = start_tree->tree[i];
    }
    // // Add the first tree to output path
    // for (long i = 0; i < 2 * num_leaves - 1; i++){
    //     path.trees[index].tree[i] = current_tree->tree[i];
    // }
    index+=1;

    Queue* to_visit = queue_new();  
    queue_push_tail(to_visit, current_tree);
    long r=0; //rank of the lowest node that induces different clusters in current_tree and dest_tree.
    long num_iterations = 0;
    long distance = 1;
    while (queue_is_empty(to_visit) != 0){
        num_iterations++;
        current_tree = queue_pop_head(to_visit);
        // Find the lowest node (at position r) for which current_tree and dest_tree are different
        for (long i = 0; i < 2*num_leaves - 1; i++){
            if ((!(current_tree->tree[i].children[0] == dest_tree->tree[i].children[0] && current_tree->tree[i].children[1]==dest_tree->tree[i].children[1]) &&
            !(current_tree->tree[i].children[0] == dest_tree->tree[i].children[1] && current_tree->tree[i].children[1] == dest_tree->tree[i].children[0]))){
                r = i;
                break;
            }
        }
        // Initialise list of neighbours (copy current_tree to every position in the tree_list)
        Tree_List neighbours;
        neighbours.trees = malloc(5 * sizeof(Tree)); // We consider a max of 5 neighbouring trees (depending on tree shape)
        for (long i = 0; i < 5; i++){
            neighbours.trees[i].num_leaves = num_leaves;
            // neighbours.trees[i].tree = current_tree->tree;
            neighbours.trees[i].tree = calloc(2*num_leaves - 1, sizeof(Node));
            memcpy(neighbours.trees[i].tree, current_tree->tree, (2*num_leaves - 1)*sizeof(Node));
        }
        long index=0;
        // first neighbour : rank move
        long mrca_rank = mrca(current_tree, dest_tree->tree[r].children[0], dest_tree->tree[r].children[1]); //find currently considered mrca and check if the interval below it allows rank move
        if (hspr == 1 && (current_tree->tree[mrca_rank].children[0] != mrca_rank-1 &&
        current_tree->tree[mrca_rank].children[1] != mrca_rank-1) && mrca_rank != 1){
            Tree* neighbour_pointer = &neighbours.trees[index];
            rank_move(neighbour_pointer, mrca_rank-1);
            index++;
        } else if (mrca_rank == 1){
            // alternatively, we make an NNI move to decrease the rank of the mrca.
            // note that this tree might be the same as one of the ones added in the next for loop as SPR neighbours.
            // this has no effect on the computed distance, just increases runtime, but we are fine with that.
            Tree* neighbour_pointer = &neighbours.trees[index];
            nni_move(neighbour_pointer, mrca_rank-1, 0);
            if (mrca(neighbour_pointer, dest_tree->tree[r].children[0], dest_tree->tree[r].children[1]) == mrca_rank){ //do exactly the other NNI move, if the first one did not decrease the mrca.
                nni_move(neighbour_pointer, mrca_rank-1, 1);
            }
            index++;
        }
        // next (up to four) neighbours: move current_tree children[i] to dest_tree children[j] (if possible)
        for (int i = 0; i <2; i++){
            for (int j = 0; j < 2; j++){
                long child_moving = neighbours.trees[index].tree[r].children[i];
                long new_sibling = dest_tree->tree[r].children[j];
                long old_sibling = neighbours.trees[index].tree[r].children[1-i];
                Tree* neighbour_pointer = &neighbours.trees[index];
                if (new_sibling != child_moving && new_sibling != old_sibling){
                    spr_move(neighbour_pointer, r, new_sibling, i);
                    index++;
                }
            }
        }
        // // print neighbouring trees (for testing)
        // printf("neighbour trees:\n");
        // for (long i = 0; i < 5; i++){
        //     for (long j = 0; j < 2*num_leaves-1; j++){
        //         printf("children: %ld, %ld, parent: %ld\n", neighbours.trees[i].tree[j].children[0], neighbours.trees[i].tree[j].children[1], neighbours.trees[i].tree[j].parent);
        //     }
        // }

        // Set the number of neighbours
        neighbours.num_trees=index;

        // update visited_at_distance
        // printf("before update:\n");
        // printf("distance-1: %ld, visited_at_distance[distance-1]: %ld\n", distance-1, visited_at_distance[distance-1]);
        // printf("distance: %ld, visited_at_distance[distance]: %ld\n", distance, visited_at_distance[distance]);
        if (visited_at_distance[distance-1] == 0){
            distance++;
        }
        // not sure about this bit
        visited_at_distance[distance-1]--;
        visited_at_distance[distance]+=neighbours.num_trees;
        // printf("after update:\n");
        // printf("distance-1: %ld, visited_at_distance[distance-1]: %ld\n", distance-1, visited_at_distance[distance-1]);
        // printf("distance: %ld, visited_at_distance[distance]: %ld\n", distance, visited_at_distance[distance]);

        // Now add neighbours to queue and check if we already reached destination tree.
        // If we reached it, we can stop.
        for(int i = 0; i < neighbours.num_trees; i++){
            Tree *neighbour_pointer = &neighbours.trees[i];
            queue_push_tail(to_visit, neighbour_pointer);
            // Check if we reached destination tree already
            int found = 0;
            for (long j = 0; j < 2*num_leaves - 1; j++){
                // printf("current_parent: %ld, dest_parent: %ld\n", neighbours.trees[i].tree[j].parent, dest_tree->tree[j].parent);
                if (neighbours.trees[i].tree[j].parent != dest_tree->tree[j].parent){
                    found = 1;
                }
            }
            if (found ==0){
                for(long i =0; i < num_leaves*num_iterations; i++){
                    if (visited_at_distance[i]!=0){
                        while(visited_at_distance[i] != 0){
                            // printf("i: %ld\n", i);
                            i++;
                        }
                        output = i-1;
                        break;
                    }
                }
                return output;
                // We found a path
            }
            // printf("length of queue: %ld\n", queue_get_length(to_visit));
        }
    }
    // If we cannot find destination tree, return -1
    return -1;
}


// returns length of the path computed by tree search in neighbourhoods (BFS), restricting neighbourhoods to use a top down approach, always taking the neighbour with minimum size of symmetric difference of current cluster
// more detailed description of this algorithm can be found in git repo rankedSPR_paper
long rankedspr_path_top_down_symm_diff(Tree* start_tree, Tree* dest_tree){
    // long output = 0; //length of the path that is being computed in this function
    // compute a path between start_tree and dest_tree (approximation for shortest path)
    // this approach uses tree search by only considering a few specific neighbours to the current tree
    long output = 0; //length of the path that is being computed in this function
    // compute a path between start_tree and dest_tree (approximation for shortest path)
    // this approach uses tree search by only considering a few specific neighbours to the current tree
    long num_leaves = start_tree->num_leaves;
    // array containing at position i the number of trees in i-neighbourhood whose neighbours have already been added to the queue -- needed to derive the distance in the end.
    long* visited_at_distance = calloc(num_leaves * num_leaves, sizeof(long)); // not sure if this is correct
    visited_at_distance[0] = 1;

    // Check starting_tree:
    // for (long i = 0; i < 2*num_leaves - 1; i++){
    //     printf("children: %ld, %ld, parent: %ld\n", start_tree->tree[i].children[0], start_tree->tree[i].children[1], start_tree->tree[i].parent);
    // }

    // Check if start_tree = dest_tree (if so, we output distance 0)
    int found = 0;
    for (long j = 0; j < 2*num_leaves - 1; j++){
        // printf("current_parent: %ld, dest_parent: %ld\n", neighbours.trees[i].tree[j].parent, dest_tree->tree[j].parent);
        if (start_tree->tree[j].parent != dest_tree->tree[j].parent){
            found = 1;
        }
    }
    if(found ==0){
        return(0);
    }

    // // Initialise output path
    // Tree_List path; // output: list of trees on FP path
    // path.num_trees = 0.5 * (num_leaves-1) * (num_leaves-2) + 1; //diameter of rankedspr is less than quadratic
    // path.trees = malloc(path.num_trees * sizeof(Tree));
    // for (long i = 0; i < path.num_trees; i++){
    //     path.trees[i].num_leaves = num_leaves;
    //     path.trees[i].tree = malloc((2* num_leaves - 1) * sizeof(Node));
    // }

    // current path index (i.e. current path length)
    long index = 0;

    //Deep copy start tree to get new tree to be added to path iteratively
    Tree* current_tree = malloc(sizeof(Node*) + 3 * sizeof(long));
    current_tree->num_leaves = num_leaves;
    current_tree->tree = malloc((2 * num_leaves - 1) * sizeof(Node)); // deep copy start tree
    for (long i = 0; i < 2 * num_leaves - 1; i++){
        current_tree->tree[i] = start_tree->tree[i];
    }
    // // Add the first tree to output path
    // for (long i = 0; i < 2 * num_leaves - 1; i++){
    //     path.trees[index].tree[i] = current_tree->tree[i];
    // }
    index+=1;

    Queue* to_visit = queue_new();  
    queue_push_tail(to_visit, current_tree);
    long r=0; //rank of the lowest node that induces different clusters in current_tree and dest_tree.
    long num_iterations = 0;
    long distance = 1; // highest entry in visited_at_distance that is not 0
    while (queue_is_empty(to_visit) != 0){
        num_iterations++;
        current_tree = queue_pop_head(to_visit);
        // printf("d: %ld, visited at distance d-1, d, d+1: %ld, %ld, %ld\n", distance, visited_at_distance[distance-1],visited_at_distance[distance],visited_at_distance[distance+1]);
        if (visited_at_distance[distance-1] == 0){
            distance++;
        }
        visited_at_distance[distance-1]--; //one less tree with distance-1 in queue
        // Find the highest node (at position r) for which clusters induced by current_tree and dest_tree are different
        for (long i = 2*num_leaves-3; i >= num_leaves; i--){
            if (symmetric_cluster_diff(current_tree, dest_tree, i) > 0){
                r = i;
                break;
            }
        }
        Tree_List neighbours = spr_neighbourhood(current_tree);
        long min_symm_diff = symmetric_cluster_diff(current_tree, dest_tree, r);
        for (int i = 0; i < neighbours.num_trees; i++){
            long symm_diff = symmetric_cluster_diff(&neighbours.trees[i], dest_tree, r);
            if (symm_diff < min_symm_diff){
                min_symm_diff = symm_diff;
            }
        }
        if (min_symm_diff == symmetric_cluster_diff(current_tree, dest_tree, r)){
            printf("No improvement in symmetric cluster difference possible for any neighbours.\n");
        }
        
        // Now add neighbours to queue and check if we already reached destination tree.
        // If we reached it, we can stop.
        for(int i = 0; i < neighbours.num_trees; i++){
            long symm_diff = symmetric_cluster_diff(&neighbours.trees[i], dest_tree, r);
            if (symm_diff == min_symm_diff){
                queue_push_tail(to_visit, &neighbours.trees[i]);
                visited_at_distance[distance]++; // add one for every tree that is added to queue
            }
            // Check if we reached destination tree already
            int found = 0;
            // printf("neighbours:\n");
            for (long j = 0; j < 2*num_leaves - 1; j++){
                // printf("current_parent: %ld, dest_parent: %ld\n", neighbours.trees[i].tree[j].parent, dest_tree->tree[j].parent);
                if (neighbours.trees[i].tree[j].parent != dest_tree->tree[j].parent){
                    found = 1;
                }
            }
            if (found == 0){
                // printf("found. d: %ld, visited at distance d-1, d, d+1: %ld, %ld, %ld\n", distance, visited_at_distance[distance-1],visited_at_distance[distance],visited_at_distance[distance+1]);
                if (visited_at_distance[distance]!=0){
                    output = distance;
                }else{
                    output = distance-1;
                }
                return output;
                // We found a path
            }
            // printf("length of queue: %ld\n", queue_get_length(to_visit));
        }
    }
    // If we cannot find destination tree, return -1
    return -1;
}


// Create a path by using a bottom-up approach in RSPR/HSPR, only using HSPR moves -- greedily create dest_tree from bottom to top
Tree_List rankedspr_path_bottom_up_hspr(Tree *start_tree, Tree *dest_tree){
    long num_leaves = start_tree->num_leaves;

    // Initialise output path
    Tree_List path; // output: list of trees on output path
    path.num_trees = (num_leaves-1) * (num_leaves-2) + 1; //maximum of HSPR moves is less than quadratic
    path.trees = malloc(path.num_trees * sizeof(Tree));
    for (long i = 0; i < path.num_trees; i++){
        path.trees[i].num_leaves = num_leaves;
        path.trees[i].tree = malloc((2* num_leaves - 1) * sizeof(Node));
    }

    //Deep copy start tree to get new tree to be added to path iteratively
    Tree* current_tree = malloc(sizeof(Node*) + 3 * sizeof(long));
    current_tree->num_leaves = num_leaves;
    current_tree->tree = malloc((2 * num_leaves - 1) * sizeof(Node)); // deep copy start tree
    for (long i = 0; i < 2 * num_leaves - 1; i++){
        current_tree->tree[i] = start_tree->tree[i];
    }

    // Add the first tree to output path
    for (long i = 0; i < 2 * num_leaves - 1; i++){
        path.trees[0].tree[i] = current_tree->tree[i];
    }

    long index = 1; // index of the currently last tree on output path

    for(long i = num_leaves; i < 2*num_leaves - 1; i++){
        long current_child1 = current_tree->tree[i].children[0];
        long current_child2 = current_tree->tree[i].children[1];
        if((current_child1 == dest_tree->tree[i].children[0] && current_child2 == dest_tree->tree[i].children[1]) || (current_child2 == dest_tree->tree[i].children[0] && current_child1 == dest_tree->tree[i].children[1])){
            // don't do anything, proceed to next iteration
        } else if (current_child1 == dest_tree->tree[i].children[0]){
            spr_move(current_tree, i, dest_tree->tree[i].children[1], 0);
            // add current_tree to path
            for (long j = 0; j < 2 * num_leaves - 1; j++){
                path.trees[index].tree[j] = current_tree->tree[j];
            }
            index ++;
        } else if (current_child1 == dest_tree->tree[i].children[1]){
            spr_move(current_tree, i, dest_tree->tree[i].children[0], 0);
            // add current_tree to path
            for (long j = 0; j < 2 * num_leaves - 1; j++){
                path.trees[index].tree[j] = current_tree->tree[j];
            }
            index ++;
        } else if (current_child2 == dest_tree->tree[i].children[0]){
            spr_move(current_tree, i, dest_tree->tree[i].children[1], 1);
            // add current_tree to path
            for (long j = 0; j < 2 * num_leaves - 1; j++){
                path.trees[index].tree[j] = current_tree->tree[j];
            }
            index ++;
        } else if (current_child2 == dest_tree->tree[i].children[1]){
            spr_move(current_tree, i, dest_tree->tree[i].children[0], 1);
            // add current_tree to path
            for (long j = 0; j < 2 * num_leaves - 1; j++){
                path.trees[index].tree[j] = current_tree->tree[j];
            }
            index ++;
        } else
            { // choose a random child of current node i to move -- we set this to be children[0] << The choice of the child influences the length of the output path!
            spr_move(current_tree, i, dest_tree->tree[i].children[0], 0);
            // add current_tree to path
            for (long j = 0; j < 2 * num_leaves - 1; j++){
                path.trees[index].tree[j] = current_tree->tree[j];
            }
            index ++;
            // find the index of the child that we want to move now (the one that coincides with the one in dest_tree)
            int child_index = 0;
            if (current_tree->tree[i].children[1] == dest_tree->tree[i].children[0]){
                child_index = 1;
            }
            // now move the child that has correct parent to the other one of dest_tree
            spr_move(current_tree, i, dest_tree->tree[i].children[1], child_index);
            // add current_tree to path
            for (long j = 0; j < 2 * num_leaves - 1; j++){
                path.trees[index].tree[j] = current_tree->tree[j];
            }
            index ++;
        }
    }
    path.num_trees = index;
    free(current_tree);
    return(path);
}


// Compute approximated distance using the same approach as above (rankedspr_path_bottom_up_hspr)
long rankedspr_path_bottom_up_hspr_dist(Tree *start_tree, Tree *dest_tree){
    long num_leaves = start_tree->num_leaves;

    //Deep copy start tree to get new tree to be added to path iteratively
    Tree* current_tree = malloc(sizeof(Node*) + 3 * sizeof(long));
    current_tree->num_leaves = num_leaves;
    current_tree->tree = malloc((2 * num_leaves - 1) * sizeof(Node)); // deep copy start tree
    for (long i = 0; i < 2 * num_leaves - 1; i++){
        current_tree->tree[i] = start_tree->tree[i];
    }

    long index = 0; // current length of output path

    for(long i = num_leaves; i < 2*num_leaves - 1; i++){
        // printf("iteration: %ld\n", i);
        long current_child1 = current_tree->tree[i].children[0];
        long current_child2 = current_tree->tree[i].children[1];
        if((current_child1 == dest_tree->tree[i].children[0] && current_child2 == dest_tree->tree[i].children[1]) || (current_child2 == dest_tree->tree[i].children[0] && current_child1 == dest_tree->tree[i].children[1])){
            // don't do anything, proceed to next iteration
        } else if (current_child1 == dest_tree->tree[i].children[0]){
            spr_move(current_tree, i, dest_tree->tree[i].children[1], 0);
            index++;
        } else if (current_child1 == dest_tree->tree[i].children[1]){
            spr_move(current_tree, i, dest_tree->tree[i].children[0], 0);
            index++;
        } else if (current_child2 == dest_tree->tree[i].children[0]){
            spr_move(current_tree, i, dest_tree->tree[i].children[1], 1);
            index++;
        } else if (current_child2 == dest_tree->tree[i].children[1]){
            spr_move(current_tree, i, dest_tree->tree[i].children[0], 1);
            index++;
        } else
            { // choose a random child of current node i to move -- we set this to be children[0]
            spr_move(current_tree, i, dest_tree->tree[i].children[0], 0);
            index ++;
            // find the index of the child that we want to move now (the one that coincides with the one in dest_tree)
            int child_index = 0;
            if(current_tree->tree[i].children[1] == dest_tree->tree[i].children[0]){
                child_index = 1;
            }
            // now move the child that has correct parent to the other one of dest_tree
            spr_move(current_tree, i, dest_tree->tree[i].children[1], child_index);
            index++;
        }
        // printf("current_tree children: %ld, %ld, dest_tree children: %ld, %ld", current_tree->tree[i].children[0], current_tree->tree[i].children[1], dest_tree->tree[i].children[0],dest_tree->tree[i].children[1]);
    }
    free(current_tree);
    return(index);
}


long fp_rspr(Tree* tree1, Tree* tree2){
    // compute first RNNI moves on shortest RSPR path.
    // Follow FP bottom-up approach, but only accept move that does not move any mrca's or parents of leaves in the wrong direction
    // possible extension: if no such FP move exist, try moving one of the children that build current cluster in tree2 down.
    // this extension is not implemented, because the first part already doesn't work.

    long num_leaves = tree1->num_leaves;
    long num_nodes = 2 * num_leaves -1;
    long path_length = 0; // output: length of RNNI path

    // deep copy starting tree
    Tree * current_tree = malloc(sizeof(Node*) + 3 * sizeof(long));
    current_tree->tree = malloc((2 * num_leaves - 1) * sizeof(Node));
    current_tree->num_leaves = num_leaves;
    for (long i = 0; i < 2 * num_leaves - 1; i++){
        current_tree->tree[i] = tree1->tree[i];
    }

    // iterate through nodes of tree2 and try to fix one node at a time
    for(long i = num_leaves; i < num_nodes; i++){
        // printf("iteration %ld\n", i);
        long current_mrca = mrca(current_tree, tree2->tree[i].children[0], tree2->tree[i].children[1]);
        while (current_mrca > i){
            // deep copy tree
            Tree* neighbour = new_tree_copy(current_tree);
            // decrease the mrca of mrca in current_tree
            decrease_mrca(neighbour, tree2->tree[i].children[0], tree2->tree[i].children[1]);
            // check if no mrca has been moved in wrong direction:
            long* n_mrca_array = mrca_array(neighbour, tree2);
            long* c_mrca_array = mrca_array(current_tree, tree2);
            int take_neighbour = 0; // only take neighbour if this variable stays 0
            for(long i = 0; i < num_leaves; i++){
                if (abs(neighbour->tree[i].parent - tree2->tree[i].parent) > abs(current_tree->tree[i].parent - tree2->tree[i].parent)){ // the parent of an existing subtree has been moved in the wrong direction
                    take_neighbour = 1;
                    printf("case1\n");
                }
                if (i < num_leaves-1 && abs(i+num_leaves-n_mrca_array[i+num_leaves]) > abs(i+num_leaves-c_mrca_array[i+num_leaves])){ // an mrca has been moved in the wrong direction
                    take_neighbour = 1;
                }
            }
            if(take_neighbour == 1){
                return(path_length);
            } else{
                path_length++;
                current_mrca--;
                // update current_tree
                for (long i = 0; i < 2 * num_leaves - 1; i++){
                    current_tree->tree[i] = neighbour->tree[i];
                }
            }
        }
    }
    return(path_length); // this only happens if the entire path consists of RNNI moves only
}
