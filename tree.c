/*Efficient implementation of FINDPATH on ranked trees*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "tree.h"
#include "queue.c"


// Number of digits of an integer -- needed to get an upper bound of the length of an input tree as string (when reading from a file)
int get_num_digits(int integer){
    int n = integer;
    int num_digits = 0;
    while(n != 0){
        n /= 10;
        num_digits++;
    }
    return num_digits;
}


// Return tree as string in cluster format -- for testing purposes. Currently doesn't work
char* tree_to_string(Tree * input_tree){
    if (input_tree->tree == NULL){
        printf("Error. Can't write tree. Given tree doesn't exist.\n");
        return(NULL);
    } else{
        long num_leaves = input_tree->num_leaves;
        int num_digits_n = get_num_digits(input_tree->num_leaves); // number of digits of the int num_leaves
        long max_str_length = 2 * num_leaves * num_leaves * num_digits_n; //upper bound for the maximum length of a tree as string
        char *tree_str = malloc(2 * max_str_length * sizeof(char));

        // // Check if input tree is 'correct'
        // for (int i = 0; i < 2 * num_leaves - 1; i++){
        //     printf("Node %d, Parent %ld, Children %ld and %ld\n", i, input_tree->tree[i].parent, input_tree->tree[i].children[0], input_tree->tree[i].children[1]);
        // }

        // create matrix cluster*leaves -- 0 if leaf is not in cluster, 1 if it is in cluster
        long ** clusters = malloc((num_leaves - 1) * sizeof(long *));
        for (long i = 0; i < num_leaves - 1; i++){
            clusters[i] = malloc((num_leaves) * sizeof(long));
        }

        for (long i = 0; i < num_leaves ; i++){
            for (long j = 0; j < num_leaves - 1; j++){
                clusters[j][i] = 0; //initialise all entries to be 0
            }
            long j = i;
            while (input_tree->tree[j].parent != -1){
                j = input_tree->tree[j].parent;
                // printf("j= %ld, numleaves = %ld, i = %ld\n", j, num_leaves, i);
                clusters[j - num_leaves][i] = 1;
            }
            clusters[num_leaves - 2][i] = 1;
        }

        // convert matrix into output string tree_str
        sprintf(tree_str, "[{");
        long tree_str_pos; //last position in tree_str that is filled with a character
        for (long i = 0; i < num_leaves - 1; i++){
            for (long j = 0; j < num_leaves; j++){
                if (clusters[i][j] == 1){
                    char leaf_str[num_digits_n + 1];
                    sprintf(leaf_str, "%ld,", j+1);
                    strcat(tree_str, leaf_str);
                }
            }
            tree_str_pos = strlen(tree_str) - 1;
            tree_str[tree_str_pos] = '\0'; // delete last comma
            strcat(tree_str, "}:"); // end of cluster, next we add time

            // retrieve the time of the current node (i.e. cluster) as string/ char*
            long num_digits_time = get_num_digits(input_tree->tree[i+num_leaves].time);
            char time_str[num_digits_time + 1];
            sprintf(time_str, "%ld", input_tree->tree[i+num_leaves].time);
            strcat(tree_str, time_str);

            // beginning of next cluster
            strcat(tree_str, ",{");
            tree_str_pos = strlen(tree_str)-1;
        }
        tree_str[strlen(tree_str) - 2] = '\0'; // delete , at end of tree_str
        strcat(tree_str, "]");

        for (long i = 0; i < num_leaves - 1; i++){
            free(clusters[i]);
        }
        free(clusters);

        return(tree_str);
    }
}


// NNI move on edge bounded by rank rank_in_list and rank_in_list + 1, moving child_stays (index) of the lower node up
int nni_move(Tree * input_tree, long rank_in_list, int child_moves_up){
    if (input_tree->tree == NULL){
        printf("Error. No RNNI move possible. Given tree doesn't exist.\n");
    } else{
        if(input_tree->tree[rank_in_list].parent != rank_in_list + 1){
            printf("Can't do an NNI - interval [%ld, %ld] is not an edge!\n", rank_in_list, rank_in_list + 1);
            return 1;
        } else{
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
    }
    return 0;
}


// ranked SPR move pruning the child with index child_moving of the node at position r of the node_list.
// The subtree gets re-attached as sibling of the node at position new_sibling of the node_list.
// Note that r and new_sibling are the positions in the node_list rather than actual ranks.
int spr_move(Tree * input_tree, long r, long new_sibling, int child_moving){
    if (input_tree->tree == NULL){
        printf("Error. No SPR move possible. Given tree doesn't exist.\n");
        return 1;
    } else if (new_sibling > r || input_tree->tree[new_sibling].parent < r){
        printf("Error. No SPR move possible, as destination edge does not cover rank r.\n");
        return 1;
    } else {
        // // Print input tree
        // printf("input tree:\n");
        // for (int i = 0; i < 2 * input_tree->num_leaves - 1; i++){
        //     printf("Node %d, Parent %ld, Children %ld and %ld\n", i, input_tree->tree[i].parent, input_tree->tree[i].children[0], input_tree->tree[i].children[1]);
        // }
        long old_parent = input_tree->tree[r].parent;
        long new_parent = input_tree->tree[new_sibling].parent;
        long old_sibling = input_tree->tree[r].children[1-child_moving];
        // printf("Old parent: %ld, new_parent: %ld, old_sibling: %ld\n", old_parent, new_parent, old_sibling);
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
    }
    // // Print tree after SPR move
    // printf("neighbouring tree:\n");
    // for (int i = 0; i < 2 * input_tree->num_leaves-1; i++){
    //     printf("Node %d, Parent %ld, Children %ld and %ld\n", i, input_tree->tree[i].parent, input_tree->tree[i].children[0], input_tree->tree[i].children[1]);
    // }
    return 0;
}


// Make a rank move on tree between nodes of rank rank and rank + 1 (if possible)
int rank_move(Tree * input_tree, long rank_in_list){
    if (input_tree->tree == NULL){
        printf("Error. No rank move possible. Given tree doesn't exist.\n");
        return 1;
    } else{
        if (input_tree->tree[rank_in_list].parent == rank_in_list + 1){
            printf("error\n");            printf("Error. No rank move possible on tree %s. The interval [%ld,%ld] is an edge!\n", tree_to_string(input_tree), rank_in_list, rank_in_list + 1);
        } else{
            // update parents of nodes that swap ranks
            long upper_parent;
            upper_parent = input_tree->tree[rank_in_list + 1].parent;
            input_tree->tree[rank_in_list + 1].parent = input_tree->tree[rank_in_list].parent;
            input_tree->tree[rank_in_list].parent = upper_parent;

            for (int i = 0; i < 2; i++){
                // update children of nodes that swap ranks
                long upper_child = input_tree->tree[rank_in_list + 1].children[i];
                input_tree->tree[rank_in_list + 1].children[i] = input_tree->tree[rank_in_list].children[i];
                input_tree->tree[rank_in_list].children[i] = upper_child;
            }
            for (int i = 0; i < 2; i++){
                // update parents of children of nodes that swap ranks
                input_tree->tree[input_tree->tree[rank_in_list + 1].children[i]].parent = rank_in_list + 1; 
                input_tree->tree[input_tree->tree[rank_in_list].children[i]].parent = rank_in_list;
            }
            for (int i = 0; i < 2; i ++){
                // update children of parents of nodes that swap rank
                //first case: nodes that swap ranks share a parent. In this case nothing needs to be changed
                if (input_tree->tree[rank_in_list + 1].parent == input_tree->tree[rank_in_list].parent){
                    break;
                }
                else{
                    if (input_tree->tree[input_tree->tree[rank_in_list + 1].parent].children[i] == rank_in_list){ //parent pointer of input_tree->tree[rank_in_list + 1] is already set correctly!
                        input_tree->tree[input_tree->tree[rank_in_list + 1].parent].children[i] = rank_in_list + 1;
                    }
                    if (input_tree->tree[input_tree->tree[rank_in_list].parent].children[i] == rank_in_list + 1){
                        input_tree->tree[input_tree->tree[rank_in_list].parent].children[i] = rank_in_list;
                    }
                }
            }
        }
    }
    return 0;
}


// find mrca of nodes with positions node1 and node2 in tree
// returns the rank of the mrca
long mrca(Tree * input_tree, long node1, long node2){
    long rank1 = node1;
    long rank2 = node2;
    while (rank1 != rank2){
        if (rank1 < rank2){
            rank1 = input_tree->tree[rank1].parent;
        } else{
            rank2 = input_tree->tree[rank2].parent;
        }
    }
    return rank1;
}


// Move up internal nodes that are at position >i in node list so that there are no nodes with rank less than k in the tree at the end (i.e. length moves that move nodes up -- see pseudocode FindPath^+)
int move_up(Tree * itree, long i, long k){
    // printf("move up tree: %s", tree_to_string(itree));
    long num_moves = 0; // counter for the number of moves that are necessary
    // printf("root time before move_up: %ld\n", itree->tree[2*itree->num_leaves - 2].time);
    if (itree->tree == NULL){
        printf("Error. No moves possible. Given tree doesn't exist.\n");
    } else{
        long j = i;
        // printf("k: %ld\n", k);
        // Find the highest j that needs to be moved up -- maximum is reached at root!
        while (itree->tree[j+1].time <= k && j+1 <=2*itree->num_leaves-2){
            j ++;
        }
        // printf("j after first loop: %ld\n", j);
        long num_moving_nodes = j - i; // number of nodes that will need to be moved
        // it might happen that we need to move nodes with times above k up, if there is not enough space for the other nodes that are supposed to move up.
        // Find the uppermost node that needs to move up
        while (itree->tree[j+1].time <= k+num_moving_nodes && j+1 <=2*itree->num_leaves-2){
            j++;
            num_moving_nodes++;
        }
        // printf("Index range in move_up: %ld, %ld\n", i, j);
        // printf("Time range in move_up: %ld, %ld\n", itree->tree[i].time, itree->tree[j].time);
        // Now j is the index of the uppermost node whose time needs to be increased.
        // If j is above k, then we need to move it to time[j]+k
        // In general, the nodes that have index between i and j need to end up having time k+index-i
        // !!!!! SOMETHING IS WRONG WITH THE ROOT TIME HERE, I THINK!!!!
        for (long index = i; index <= j; index++){ // Do all required length moves
            // printf("index: %ld\n", index);
            num_moves += k+index-i - itree->tree[index].time;
            itree->tree[index].time = k+index-i;
        }
        // // last iteration of loop/case where i=j/root
        // if (i==j){
        //     num_moves += k+j-i - itree->tree[j].time;
        //     itree->tree[j].time = k+j-i;
        // }
        // printf("Times after move_up: %ld, %ld\n", itree->tree[i].time, itree->tree[j].time);
    }
    // printf("current tree in move_up: %s\n", tree_to_string(itree));
    // printf("root time after move_up: %ld\n", itree->tree[2*itree->num_leaves - 2].time);
    return num_moves;
}


// Compute a path in SPR and return it as Tree_List;
// If horizontal = 1, we compute the rankedSPR neighbourhood(including rank moves), otherwise the hspr neighbouhood (without rank moves)
Tree_List all_spr_neighbourhood(Tree *input_tree, int horizontal){
    long num_leaves = input_tree->num_leaves;

    // Initialise list of neighbours
    Tree_List neighbour_list; // output: list of trees on FP path
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
        // printf("%s\n", tree_to_string(input_tree));
        // Check if we can do rank move:
        if (horizontal == 1 && r < 2*num_leaves - 2 && input_tree->tree[r].parent != r+1){
            rank_move(neighbour, r);
            // printf("rank move\n");

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
        // printf("rank: %ld\n", r);
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


long mrca_differences(Tree* current_tree, Tree* dest_tree){
    // Compute differences of ranks of mrca's of all cluster of dest_tree btw current_tree and dest_tree
    // Also add ranks of parent of leaves
    long sum = 0;
    long num_leaves = dest_tree->num_leaves;
    long mrca_list[2*num_leaves-1]; // at position i save rank(mrca_{current_tree}(C_i)) where C_i is the cluster induced by node of rank i in dest_tree
    // // First iterate through leaves
    // for (long i = 0; i < num_leaves; i++){
    //     sum += abs(current_tree->tree[i].parent - dest_tree->tree[i].parent);
    //     // printf("i: %ld, sum: %ld\n", i, sum);
    // }
    for (long i = num_leaves; i < 2*num_leaves - 2; i++){
        // iterate through the ranks of mrcas in dest_tree
        // find mrca (distinguish leaves vs. non-leaf and fill mrca_list to get mrcas of non-leafs)
        // printf("i: %ld, sum: %ld\n", i, sum);
        long child0;
        if (dest_tree->tree[i].children[0] < num_leaves){
            // printf("Child0 is leaf\n");
            child0 = dest_tree->tree[i].children[0];
        } else{
            child0 = mrca_list[dest_tree->tree[i].children[0]];
            // printf("Child0 is internal node\n");
        }

        long child1;
        if (dest_tree->tree[i].children[1] < num_leaves){
            child1 = dest_tree->tree[i].children[1];
        } else{
            child1 = mrca_list[dest_tree->tree[i].children[1]];
        }
        // printf("child0: %ld, child1: %ld\n", child0, child1);

        long current_mrca = mrca(current_tree, child0, child1);
        mrca_list[i] = current_mrca;
        // printf("child0: %ld, child1: %ld, current_mrca: %ld\n", child0, child1, current_mrca);
        sum += abs(current_mrca - i);
        // printf("i: %ld, sum: %ld\n", i, sum);
    }
    return(sum);
}


long symmetric_cluster_diff(Tree* tree1, Tree* tree2){
    // Computes sum of symmetric differences of clusters of tree1 and tree2 for all ranks i=1,..,n-1
    long num_leaves = tree1->num_leaves;

    // Get clusters for both trees:
    // create matrix cluster*leaves -- 0 if leaf is not in cluster, 1 if it is in cluster
    long ** clusters_t1 = malloc((num_leaves - 1) * sizeof(long *));
    for (long i = 0; i < num_leaves - 1; i++){
        clusters_t1[i] = malloc((num_leaves) * sizeof(long));
    }

    for (long i = 0; i < num_leaves ; i++){
        for (long j = 0; j < num_leaves - 1; j++){
            clusters_t1[j][i] = 0; //initialise all entries to be 0
        }
        long j = i;
        while (tree1->tree[j].parent != -1){
            j = tree1->tree[j].parent;
            // printf("j= %ld, numleaves = %ld, i = %ld\n", j, num_leaves, i);
            clusters_t1[j - num_leaves][i] = 1;
        }
        clusters_t1[num_leaves - 2][i] = 1;
    }

    long ** clusters_t2 = malloc((num_leaves - 1) * sizeof(long *));
    for (long i = 0; i < num_leaves - 1; i++){
        clusters_t2[i] = malloc((num_leaves) * sizeof(long));
    }

    for (long i = 0; i < num_leaves ; i++){
        for (long j = 0; j < num_leaves - 1; j++){
            clusters_t2[j][i] = 0; //initialise all entries to be 0
        }
        long j = i;
        while (tree2->tree[j].parent != -1){
            j = tree2->tree[j].parent;
            // printf("j= %ld, numleaves = %ld, i = %ld\n", j, num_leaves, i);
            clusters_t2[j - num_leaves][i] = 1;
        }
        clusters_t2[num_leaves - 2][i] = 1;
    }

    // Now compute symmetric difference between clusters (i.e. all columns that have a 1 in cluster_t1 or cluster_t2 and a 0 in the other matrix)
    long symm_diff = 0;
    for (long i = 0; i < num_leaves -1; i++){
        for (long j = 0; j < num_leaves; j++){
            if (clusters_t1[i][j] + clusters_t2[i][j] == 1){
                symm_diff++;
            }
        }
    }
    free(clusters_t1);
    free(clusters_t2);
    return(symm_diff);
}


Tree_List rankedspr_path_mrca_cluster_diff(Tree* start_tree, Tree* dest_tree){
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

    long diff = mrca_differences(current_tree, dest_tree) + symmetric_cluster_diff(current_tree, dest_tree);
    while (diff > 0){
        // printf("current tree: %s\n", tree_to_string(current_tree));
        Tree_List neighbours = spr_neighbourhood(current_tree);
        for (long i = 0; i < neighbours.num_trees; i++){
            Tree* neighbour_pointer;
            neighbour_pointer = &neighbours.trees[i];
            long new_diff =  mrca_differences(neighbour_pointer, dest_tree) + symmetric_cluster_diff(neighbour_pointer, dest_tree);
            // printf("neighbour tree: %s\n", tree_to_string(neighbour_pointer));
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


Tree_List rankedspr_path_mrca_diff(Tree* start_tree, Tree* dest_tree){
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

    long mrca_diff = mrca_differences(current_tree, dest_tree);
    while (mrca_diff > 0){
        // printf("current tree: %s\n", tree_to_string(current_tree));
        Tree_List neighbours = spr_neighbourhood(current_tree);
        for (long i = 0; i < neighbours.num_trees; i++){
            Tree* neighbour_pointer;
            neighbour_pointer = &neighbours.trees[i];
            long new_mrca_diff =  mrca_differences(neighbour_pointer, dest_tree);
            // printf("neighbour tree: %s\n", tree_to_string(neighbour_pointer));
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


// returns length of the path computed by tree search in neighbourhoods (BFS), restricting neighbourhoods to use a bottom up approach like FP, and only moves involving nodes of the currently considered cluster in dest_tree
// more detailed description of this algorithm can be found in git repo rankedSPR_paper.
long rankedspr_path_restricting_neighbourhood(Tree* start_tree, Tree* dest_tree){
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
        // printf("current tree: %s, mrca: %ld, nodes: %ld, %ld\n", tree_to_string(current_tree), mrca_rank, dest_tree->tree[r].children[0], dest_tree->tree[r].children[1]);
        if ((current_tree->tree[mrca_rank].children[0] != mrca_rank-1 &&
        current_tree->tree[mrca_rank].children[1] != mrca_rank-1) && mrca_rank != 1){
            Tree* neighbour_pointer = &neighbours.trees[index];
            rank_move(neighbour_pointer, mrca_rank-1);
            index++;
        } else if (mrca_rank != 1){
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



// Compute symmetric difference of clusters induced by nodes of rank k in tree1 and tree2
long symm_cluster_diff(Tree* tree1, Tree* tree2, long k){
    long num_leaves = tree1->num_leaves;
    // find clusters induced by node of rank k in both tree1 and tree2
    // Note: getting cluster matrix is copy&paste from tree_to_string function. We only need one column of this matrix here
    // create matrix cluster*leaves -- 0 if leaf is not in cluster, 1 if it is in cluster
    // first: clusters_t1 for tree1
    long ** clusters_t1 = malloc((num_leaves - 1) * sizeof(long *));
    for (long i = 0; i < num_leaves - 1; i++){
        clusters_t1[i] = malloc((num_leaves) * sizeof(long));
    }

    for (long i = 0; i < num_leaves ; i++){
        for (long j = 0; j < num_leaves - 1; j++){
            clusters_t1[j][i] = 0; //initialise all entries to be 0
        }
        long j = i;
        while (tree1->tree[j].parent != -1){
            j = tree1->tree[j].parent;
            // printf("j= %ld, numleaves = %ld, i = %ld\n", j, num_leaves, i);
            clusters_t1[j - num_leaves][i] = 1;
        }
        clusters_t1[num_leaves - 2][i] = 1;
    }
    // same thing for tree2
    long ** clusters_t2 = malloc((num_leaves - 1) * sizeof(long *));
    for (long i = 0; i < num_leaves - 1; i++){
        clusters_t2[i] = malloc((num_leaves) * sizeof(long));
    }

    for (long i = 0; i < num_leaves ; i++){
        for (long j = 0; j < num_leaves - 1; j++){
            clusters_t2[j][i] = 0; //initialise all entries to be 0
        }
        long j = i;
        while (tree2->tree[j].parent != -1){
            j = tree2->tree[j].parent;
            // printf("j= %ld, numleaves = %ld, i = %ld\n", j, num_leaves, i);
            clusters_t2[j - num_leaves][i] = 1;
        }
        clusters_t2[num_leaves - 2][i] = 1;
    }

    // now count the number of entries that are a 1 in either cluster_t1 or cluster_t2 in row k-num_leaves
    long output = 0;
    // printf("clusters for trees:\n, %s, %s\n", tree_to_string(tree1), tree_to_string(tree2));
    for (long i = 0; i < num_leaves; i++){
        // printf("i: %ld, k:%ld\n", i, k);
        // printf("clusters_t1[k-num_leaves][i]: %ld\n",clusters_t1[k-num_leaves][i]);
        // printf("clusters_t2[k-num_leaves][i]: %ld\n",clusters_t2[k-num_leaves][i]);
        if (clusters_t1[k-num_leaves][i]+clusters_t2[k-num_leaves][i]==1){
            output++;
        }
    }
    for (long i = 0; i < num_leaves - 1; i++){
        free(clusters_t1[i]);
        free(clusters_t2[i]);
    }
    free(clusters_t1);
    free(clusters_t2);
    return(output);
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
    long distance = 1;
    while (queue_is_empty(to_visit) != 0){
        num_iterations++;
        current_tree = queue_pop_head(to_visit);
        // Find the highest node (at position r) for which clusters induced by current_tree and dest_tree are different
        for (long i = 2*num_leaves-3; i >= num_leaves; i--){
            if (symm_cluster_diff(current_tree, dest_tree, i) > 0){
                r = i;
                break;
            }
        }
        Tree_List neighbours = spr_neighbourhood(current_tree);
        long min_symm_diff = symm_cluster_diff(current_tree, dest_tree, r);
        for (int i = 0; i < neighbours.num_trees; i++){
            long symm_diff = symm_cluster_diff(&neighbours.trees[i], dest_tree, r);
            if (symm_diff < min_symm_diff){
                min_symm_diff = symm_diff;
            }
        }
        if (min_symm_diff == symm_cluster_diff(current_tree, dest_tree, r)){
            printf("No improvement in symmetric cluster difference possible for any neighbours of %s\n", tree_to_string(current_tree));
            return(-1);
        }
        // printf("number of neighbours: %ld\n", neighbours.num_trees);
        // // print neighbouring trees (for testing)
        // printf("num_iterations: %ld, neighbour trees:\n", num_iterations);
        // for (long i = 0; i < neighbours.num_trees; i++){
        //     for (long j = 0; j < 2*num_leaves-1; j++){
        //         printf("children: %ld, %ld, parent: %ld\n", neighbours.trees[i].tree[j].children[0], neighbours.trees[i].tree[j].children[1], neighbours.trees[i].tree[j].parent);
        //     }
        // }

        // // update visited_at_distance
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
            long symm_diff = symm_cluster_diff(&neighbours.trees[i], current_tree, r);
            if (symm_diff == min_symm_diff){
                queue_push_tail(to_visit, neighbour_pointer);
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
            if (found ==0){
                for(long i =0; i < num_leaves*num_iterations; i++){
                    if (visited_at_distance[i]!=0){
                        while(visited_at_distance[i] != 0){
                            // printf("visited at distance %ld: %ld\n", i, visited_at_distance[i]);
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


// FINDPATH. returns a path in matrix representation -- explanation in data_structures.md
// This function only works for ranked trees
Path findpath(Tree *start_tree, Tree *dest_tree){
    float count = 0.05; // counter to print the progress of the algorithm (in 10% steps of max distance)
    long num_leaves = start_tree->num_leaves;
    long max_dist = ((num_leaves - 1) * (num_leaves - 2))/2 + 1;
    Path path;
    path.moves = malloc((max_dist + 1) * sizeof(long*)); // save moves in a table: each row (after the first) is move, column 1: rank of lower node bounding the interval of move, column 2: 0,1,2: rank move, nni where children[0] stays, nni where children[1] stays; the first row only contains distance between the trees (moves[0][0])
    for (long i = 0; i < max_dist + 1; i++){
        path.moves[i] = malloc(2 * sizeof(long));
        path.moves[i][0] = 0;
        path.moves[i][1] = 0;
    }
    long path_index = 0; // next position on path that we want to fill with a tree pointer
    if (start_tree->tree == NULL){
        printf("Error. Start tree doesn't exist.\n");
    } else if (dest_tree->tree == NULL){
        printf("Error. Destination tree doesn't exist.\n");
    } else{
        remove("./output/findpath.rtree");
        // write_tree(start_tree->tree, num_leaves, "./output/findpath.rtree"); // this ruins the running time!!!!!!!!
        long current_mrca; //rank of the mrca that needs to be moved down
        Tree current_tree;
        current_tree.tree = malloc((2 * num_leaves - 1) * sizeof(Node));
        current_tree.num_leaves = num_leaves;
        for (long i = 0; i < 2 * num_leaves - 1; i++){
            current_tree.tree[i] = start_tree->tree[i];
        }
        Tree * current_tree_pointer;
        current_tree_pointer = &current_tree;
        for (long i = num_leaves; i < 2 * num_leaves - 1; i++){
            current_mrca = mrca(current_tree_pointer, dest_tree->tree[i].children[0], dest_tree->tree[i].children[1]);
            // move current_mrca down
            while(current_mrca != i){
                bool did_nni = false;
                for (int child_index = 0; child_index < 2; child_index++){ // find out if one of the children of current_tree.tree[current_mrca] has rank current_mrca - 1. If this is the case, we want to make an NNI
                    if (did_nni == false && current_tree.tree[current_mrca].children[child_index] == current_mrca - 1){ // do nni if current interval is an edge
                        // check which of the children of current_tree.tree[current_mrca] should move up by the NNI move 
                        bool found_child = false; //indicate if we found the correct child
                        int child_stays; // index of the child of current_tree.tree[current_mrca] that does not move up by an NNI move
                        // find the index of the child of the parent of the node we currently consider -- this will be the index child_stays that we want in the end
                        int current_child_index = dest_tree->tree[i].children[0]; // rank of already existing cluster in both current_tree.tree and dest_tree->tree
                        while (found_child == false){
                            while (current_tree.tree[current_child_index].parent < current_mrca - 1){ // find the x for which dest_tree->tree[i].children[x] is contained in the cluster induced by current_tree.tree[current_mrca - 1]
                                current_child_index = current_tree.tree[current_child_index].parent;
                            }
                            // find the index child_stays
                            if(current_tree.tree[current_child_index].parent == current_mrca - 1){
                                found_child = true;
                                if (current_tree.tree[current_tree.tree[current_child_index].parent].children[0] == current_child_index){
                                    child_stays = 0;
                                } else{
                                    child_stays = 1;
                                }
                            } else{
                                current_child_index = dest_tree->tree[i].children[1];
                            }
                        }
                        nni_move(current_tree_pointer, current_mrca - 1, 1 - child_stays);
                        path.moves[path_index][1] = 1 + child_stays;
                        did_nni = true;
                        current_mrca--;
                    }
                }
                if (did_nni == false){
                    rank_move(current_tree_pointer, current_mrca - 1);
                    path.moves[path_index][1] = 0;
                    current_mrca--;
                }
                path.moves[path_index][0] = current_mrca;
                path_index++;
                // Print progress (in 5% steps from max distance)
                if (count < (float) path_index / (float) max_dist){
                     printf("%d Percent of maximum distance reached\n", (int) (100 * count));
                     count += 0.05;
                }
            }
        }
        free(current_tree.tree);
    }
    path.length = path_index;
    return path;
}


// FINDPATH without saving the path -- returns only the distance
long findpath_distance(Tree *start_tree, Tree *dest_tree){
    long num_leaves = start_tree->num_leaves;
    long path_index = 0; // next position on path that we want to fill with a tree pointer
    if (start_tree->tree == NULL){
        printf("Error. Start tree doesn't exist.\n");
    } else if (dest_tree->tree == NULL){
        printf("Error. Destination tree doesn't exist.\n");
    } else{
        long current_mrca; //rank of the mrca that needs to be moved down
        Tree current_tree;
        current_tree.tree = malloc((2 * num_leaves - 1) * sizeof(Node));
        current_tree.num_leaves = num_leaves;
        for (long i = 0; i < 2 * num_leaves - 1; i++){
            current_tree.tree[i] = start_tree->tree[i];
        }
        // This pointer is needed for finding the mrca, and doing moves (nni, rank, length)
        Tree * current_tree_pointer;
        current_tree_pointer = &current_tree;
        for (long i = num_leaves; i < 2 * num_leaves - 1; i++){
            if (current_tree.tree[i].time < dest_tree->tree[i].time){
                path_index += move_up(current_tree_pointer, i, dest_tree->tree[i].time);
            }
            // we now need to find the current MRCA and decrease its time in the tree
            current_mrca = mrca(current_tree_pointer, dest_tree->tree[i].children[0], dest_tree->tree[i].children[1]); //rank of the current mrca (i.e. index in the list of nodes representing the tree)
            // move current_mrca down -- one rank or NNI move per iteration of this loop, but multiple length moves (which are summarised to one 'jump')
            while(current_tree.tree[current_mrca].time != dest_tree->tree[i].time){
                // We first see if we need to do length moves:
                // We need to move the current node down by length moves if its time is greater than the time  of the next lower node + 1
                // After this, we do an NNI or rank move and then repeat the while loop
                if (current_tree.tree[current_mrca-1].time < current_tree.tree[current_mrca].time - 1){
                    // We either need to move the node to be right above the time of the next lower node...
                    if( current_tree.tree[current_mrca-1].time + 1 > dest_tree->tree[i].time){
                        // Update the time to be one greater than the time of the next lower node.
                        // This is equivalent to doing length moves, so we add the time difference to the distance
                        path_index += current_tree.tree[current_mrca].time - (current_tree.tree[current_mrca-1].time + 1);
                        current_tree.tree[current_mrca].time = current_tree.tree[current_mrca-1].time + 1;
                    // Or we move the current node to be at the same position as the corresponding node in dest_tree
                    } else{ // in this case we move the node to its final position
                        path_index += current_tree.tree[current_mrca].time - dest_tree->tree[i].time;
                        current_tree.tree[current_mrca].time = dest_tree->tree[i].time;
                        break; // the current iteration i is finished
                    }
                }
                bool did_nni = false; //we first check if we are at an edge. If not, then did_rnni stays false and we do a rank move
                for (int child_index = 0; child_index < 2; child_index++){
                    // find out if one of the children of current_tree.tree[current_mrca] has rank current_mrca - 1. If this is the case, we want to make an NNI
                    if (did_nni == false && current_tree.tree[current_mrca].children[child_index] == current_mrca - 1){ // do nni if current interval is an edge
                        // check which of the children of current_tree.tree[current_mrca] should move up by the NNI move 
                        bool found_child = false; //indicate if we found the correct child
                        int child_stays; // index of the child of current_tree.tree[current_mrca] that does not move up by an NNI move
                        // find the index of the child of the parent of the node we currently consider -- this will be the index child_stays that we want in the end
                        int current_child_index = dest_tree->tree[i].children[0]; // rank of already existing cluster in both current_tree.tree and dest_tree->tree
                        while (found_child == false){
                            while (current_tree.tree[current_child_index].parent < current_mrca - 1){ // find the x for which dest_tree->tree[i].children[x] is contained in the cluster induced by current_tree.tree[current_mrca - 1]
                                current_child_index = current_tree.tree[current_child_index].parent;
                            }
                            // find the index child_stays
                            if(current_tree.tree[current_child_index].parent == current_mrca - 1){
                                found_child = true;
                                if (current_tree.tree[current_tree.tree[current_child_index].parent].children[0] == current_child_index){
                                    child_stays = 0;
                                } else{
                                    child_stays = 1;
                                }
                            } else{
                                current_child_index = dest_tree->tree[i].children[1];
                            }
                        }
                        nni_move(current_tree_pointer, current_mrca - 1, 1 - child_stays);
                        did_nni = true;
                        current_mrca--;
                    }
                }
                if (did_nni == false){
                    rank_move(current_tree_pointer, current_mrca - 1);
                    current_mrca--;
                }
                path_index++;
            }
            // printf("path_index: %ld \n", path_index);
        }
        free(current_tree.tree);
    }
    return path_index;
}


// returns the FINDPATH path between two given given trees as Tree_List -- runs findpath and translates path matrix to actual trees on path
Tree_List return_findpath(Tree *start_tree, Tree *dest_tree){
    long path_index = 0;
    long num_leaves = start_tree->num_leaves;
    Tree current_tree;
    current_tree.num_leaves = num_leaves;
    current_tree.tree = malloc((2 * num_leaves - 1) * sizeof(Node)); // deep copy start tree 
    for (int i = 0; i < 2 * num_leaves - 1; i++){
        current_tree.tree[i] = start_tree->tree[i];
    }

    Path fp = findpath(start_tree, dest_tree);

    long diameter = (num_leaves - 1) * (num_leaves - 2) / 2 + 1; // this is not the diameter, but the number of trees on a path giving the diameter (= diameter + 1)

    Tree_List findpath_list; // output: list of trees on FP path
    findpath_list.num_trees = fp.length;
    findpath_list.trees = malloc(diameter * sizeof(Tree));
    for (long i = 0; i < diameter; i++){
        findpath_list.trees[i].num_leaves = num_leaves;
        findpath_list.trees[i].tree = malloc((2* num_leaves - 1) * sizeof(Node));
    }
    for (long i = 0; i < 2 * num_leaves - 1; i++){
        findpath_list.trees[0].tree[i] = current_tree.tree[i];
    }

    // create actual path by doing moves starting at start_tree with the information in the matrix returned form fp above
    while(path_index < diameter - 1 && fp.moves[path_index][0] > 0){
        if (fp.moves[path_index][1] == 0){
            rank_move(&current_tree, fp.moves[path_index][0]);
        }
        else if (fp.moves[path_index][1] == 1){
            nni_move(&current_tree, fp.moves[path_index][0], 1);
        } else{
            nni_move(&current_tree, fp.moves[path_index][0], 0);
        }
        path_index++;
        // deep copy currently last tree one path
        for (long i = 0; i < 2 * num_leaves - 1; i++){
            findpath_list.trees[path_index].tree[i] = current_tree.tree[i];
        }
    }
    for (int i = 0; i < diameter + 1; i++){
        free(fp.moves[i]);
    }
    free(fp.moves);
    free(current_tree.tree);
    return findpath_list;
}


int uniform_neighbour(Tree * input_tree){
    // Perform a random RNNI move (at uniform) on input_tree
    // Deep copy input tree, so we can perform move on it
    long num_leaves = input_tree->num_leaves;
    // Tree * output_tree = malloc(sizeof(Node*) + 3 * sizeof(long));
    // output_tree->num_leaves = num_leaves;
    // output_tree->tree = malloc((2 * num_leaves - 1) * sizeof(Node)); // deep copy start tree
    // for (long i = 0; i < 2 * num_leaves - 1; i++){
    //     output_tree->tree[i] = input_tree->tree[i];
    // }
    // Count number of possible moves (rank interval + 2*NNI interval)
    long num_moves = 0; // total number of possible moves on given tree (2* #edge intervals + 1 * #rank intervals) 
    int ** move_list = malloc(2 * (num_leaves - 1) * sizeof(int*));
    for (long i = 0; i < 2*(num_leaves - 1); i++){ // max number of moves is reached if every internal edge has length one (caterpillar)
        move_list[i] = malloc(2*sizeof(int));
        move_list[i][0] = -1; // lower node of edge for move
        move_list[i][1] = -1; // rank vs nni move
    }
    // Fill move list
    for (long i = num_leaves; i < 2 * num_leaves - 1; i++){
        // printf("rank: %ld, children[0]: %ld, children[1]: %ld, parent: %ld\n", i, input_tree->tree[i].children[0], input_tree->tree[i].children[1], input_tree->tree[i].parent);
        if (input_tree->tree[i].parent == i+1){
            // printf("move_list index: %ld, num_moves: %ld, type of move: %d\n", i, num_moves, 1);
            move_list[num_moves][0] = i;
            move_list[num_moves][1] = 1; // NNI move 0
            move_list[num_moves + 1][0] = i;
            move_list[num_moves + 1][1] = 2; // NNI move 1
            // printf("move_list index: %ld, num_moves: %ld, type of move: %d\n", i, num_moves+1, 2);
            num_moves += 2;
        } else{
            // printf("move_list index: %ld, num_moves: %ld, type of move: %d\n", i, num_moves, 0);
            move_list[num_moves][0] = i;
            move_list[num_moves][1] = 0; // rank move is 0
            num_moves += 1;
        }
        // printf("num_moves: %ld\n", num_moves);
    }

    // Pick random move
    // srand(time());
    long r = rand() % (num_moves-1);
    // printf("r: %ld\n", r);
    if (move_list[r][1] == 0){
        rank_move(input_tree, move_list[r][0]);
    } else if (move_list[r][1] == 1){
        nni_move(input_tree, move_list[r][0], 0);
    } else{
        nni_move(input_tree, move_list[r][0], 1);
    }
    // free move_list
    for (long i = 0; i < 2*(num_leaves - 1); i++){ // max number of moves is reached if every internal edge has length one (caterpillar)
        free(move_list[i]);
    }
    free(move_list);
    return(0);
}


long random_walk(Tree * tree, long k){
    // Perform a series of k random RNNI moves to receive a random walk in RNNI, starting at input tree
    Tree * current_tree = malloc(sizeof(Node*) + 3 * sizeof(long));
    current_tree->num_leaves = tree->num_leaves;
    current_tree->tree = malloc((2 * tree->num_leaves - 1) * sizeof(Node)); // deep copy start tree
    for (long i = 0; i < 2 * tree->num_leaves - 1; i++){
        current_tree->tree[i] = tree->tree[i];
    }
    for (long i = 0; i < k; i++){
        uniform_neighbour(current_tree);
    }
    long distance = findpath_distance(current_tree, tree);
    free(current_tree);
    return(distance);
}
