/*Efficient implementation of FINDPATH on ranked trees*/
// Author: Lena Collienne

#include "tree.h"


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


Node empty_node(){
    Node new_node;
    new_node.parent = -1;
    new_node.children[0] = -1;
    new_node.children[1] = -1;
    new_node.time = -1;
    return new_node;
}


Tree* empty_tree(long num_leaves){
    // initalise tree on num_leaves leaves
    long num_nodes = 2 * num_leaves - 1;
    Tree* new_tree = malloc(sizeof(Node*) + 4 * sizeof(long));
    new_tree->tree = calloc((2 * num_leaves - 1), sizeof(Node));
    new_tree->num_leaves = num_leaves;
    for (long i = 0; i < 2 * num_leaves - 1; i++){
        Node new_node = empty_node();
        new_tree->tree[i] = new_node;
    }
    return new_tree;
}


Tree* deep_copy(Tree* tree){
    // deep copy tree
    long num_leaves = tree->num_leaves;
    Tree* tree_copy = empty_tree(num_leaves);
    for (long i = 0; i < 2 * num_leaves - 1; i++){
        tree_copy->tree[i] = tree->tree[i];
    }
    return tree_copy;
}


int print_tree(Tree* tree){
    // print parents and children for each node in tree
    long num_leaves = tree->num_leaves;
    long num_nodes = 2 * num_leaves - 1;
    for (long rank = 0; rank < num_nodes; rank++){
        printf("Node at rank %ld: parent %ld, children %ld, %ld\n", rank, tree->tree[rank].parent, tree->tree[rank].children[0], tree->tree[rank].children[1]);
    }
    return 0;
}


// Check whether two trees have the same (ranked) tree topology -- return 0 if this is the case, 1 otherwise
int same_topology(Tree* tree1, Tree* tree2){
    long num_leaves = tree1->num_leaves;
    for(long i=num_leaves; i < 2 * num_leaves - 1; i++){
        if (tree1->tree[i].parent != tree2->tree[i].parent){
            return(1);
        }
    }
    return(0);
}


// Check whether two trees are identical (output 0 if this is true, 1 otherwise)
int same_tree(Tree* tree1, Tree* tree2){
    for (long i = 0; i < 2* tree1->num_leaves - 1; i++){
        if (tree1->tree[i].parent != tree2->tree[i].parent){
            return(1);
        }
    }
    return(0);
}


// NNI move on edge bounded by rank and rank + 1, moving child_moves_up (index) of the lower node up
int nni_move(Tree * input_tree, long rank, int child_moves_up){
    if (input_tree->tree == NULL){
        printf("Error. No RNNI move possible. Given tree doesn't exist.\n");
        return 1;
    }
    Node* upper_node;
    upper_node = &input_tree->tree[rank + 1];
    Node* lower_node;
    lower_node = &input_tree->tree[rank];
    if(lower_node->parent != rank + 1){
        printf("Can't do an NNI - interval [%ld, %ld] is not an edge!\n", rank, rank + 1);
        return 1;
    }
    int child_moved_up;
    for (int i = 0; i < 2; i++){
        if (upper_node->children[i] != rank){ //find the child of the node of rank k+1 that is not the node of rank k
            // update parent/children relations to get nni neighbour
            input_tree->tree[upper_node->children[i]].parent = rank;
            input_tree->tree[lower_node->children[child_moves_up]].parent = rank+1;
            child_moved_up = lower_node->children[child_moves_up];
            lower_node->children[child_moves_up] = upper_node->children[i];
            upper_node->children[i] = child_moved_up;
        }
    }
    return 0;
}


// Make a rank move on tree between nodes of rank and rank + 1 (if possible)
int rank_move(Tree * input_tree, long rank){
    if (input_tree->tree == NULL){
        printf("Error. No rank move possible. Given tree doesn't exist.\n");
        return 1;
    }
    if (input_tree->tree[rank].parent == rank + 1){
        printf("Error. No rank move possible. The interval [%ld,%ld] is an edge!\n", rank, rank + 1);
        return 1;
    }
    Node* upper_node;
    upper_node = &input_tree->tree[rank + 1];
    Node* lower_node;
    lower_node = &input_tree->tree[rank];

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
        input_tree->tree[upper_node->children[i]].parent ++; 
        input_tree->tree[lower_node->children[i]].parent --;
    }
    for (int i = 0; i < 2; i ++){
        // update children of parents of nodes that swap rank
        if (upper_node->parent == lower_node->parent){
            break;
        }
        if (input_tree->tree[upper_node->parent].children[i] == rank){
            input_tree->tree[upper_node->parent].children[i] ++;
        }
        if (input_tree->tree[lower_node->parent].children[i] == rank + 1){
            input_tree->tree[lower_node->parent].children[i] --;
        }
    }
    return 0;
}


// Compute Tree_List of all RNNI neighbours
Tree_List rnni_neighbourhood(Tree *input_tree){
    long num_leaves = input_tree->num_leaves;

    // Initialise list of neighbours
    Tree_List neighbour_list; // output list of neighbours
    neighbour_list.num_trees = 2 * (num_leaves - 1); //max number of neighbours linear in number of internal nodes; max reached for caterpillar tree
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
        if (input_tree->tree[r].parent != r+1){
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
        else{ // otherwise, we do NNI moves (always 2 options)
            for (long child_moves_up=0; child_moves_up<2; child_moves_up ++){
                nni_move(neighbour, r, child_moves_up);
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


// Compute Tree_List of all rank neighbours
Tree_List rank_neighbourhood(Tree *input_tree){
    long num_leaves = input_tree->num_leaves;

    // Initialise list of neighbours
    Tree_List neighbour_list; // output list of neighbours
    neighbour_list.num_trees = 2 * (num_leaves - 1); //max number of neighbours linear in number of internal nodes; max reached for caterpillar tree
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
        if (input_tree->tree[r].parent != r+1){
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
    }
    neighbour_list.num_trees = index;
    free(neighbour);
    return(neighbour_list);
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


// find mrca of nodes with positions node1 and node2 in tree
// returns the rank of the mrca
long mrca(Tree * input_tree, long node1, long node2){
    long rank1 = node1;
    long rank2 = node2;
    while (rank1 != rank2){
        if (rank1 < rank2){
            rank1 = input_tree->tree[rank1].parent;
            if (rank1 == -1){
                return -1;
            }
        } else{
            rank2 = input_tree->tree[rank2].parent;
            if (rank2 == -1){
                return -1;
            }
        }
    }
    return rank1;
}


long* mrca_list(Tree* tree1, Tree* tree2){
    long num_leaves = tree1->num_leaves;
    long *mrca_list = malloc((2*num_leaves-1)*sizeof(long)); // at position i save rank(mrca_{tree1}(C_i)) where C_i is the cluster induced by node of rank i in tree2

    for (long i = num_leaves; i < 2*num_leaves - 1; i++){
        // iterate through the ranks of mrcas in dest_tree
        // find mrca (distinguish leaves vs. non-leaf and fill mrca_list to get mrcas of non-leafs)
        // printf("i: %ld, sum: %ld\n", i, sum);
        long child0;
        if (tree2->tree[i].children[0] < num_leaves){
            // printf("Child0 is leaf\n");
            child0 = tree2->tree[i].children[0];
        } else{
            child0 = mrca_list[tree2->tree[i].children[0]];
            // printf("Child0 is internal node\n");
        }

        long child1;
        if (tree2->tree[i].children[1] < num_leaves){
            child1 = tree2->tree[i].children[1];
        } else{
            child1 = mrca_list[tree2->tree[i].children[1]];
        }
        // printf("child0: %ld, child1: %ld\n", child0, child1);

        long current_mrca = mrca(tree1, child0, child1);
        mrca_list[i] = current_mrca;
        // printf("child0: %ld, child1: %ld, current_mrca: %ld\n", child0, child1, current_mrca);
    }
    return(mrca_list);
}


long mrca_differences(Tree* current_tree, Tree* dest_tree, int include_leaf_parents){
    // Compute differences of ranks of mrcas of all cluster of dest_tree btw current_tree and dest_tree
    // Also add ranks of parents of leaves if include_leaf_parents == 0
    long sum = 0;
    long num_leaves = dest_tree->num_leaves;
    // long mrcas[2*num_leaves-1]; // at position i save rank(mrca_{current_tree}(C_i)) where C_i is the cluster induced by node of rank i in dest_tree
    // First iterate through leaves
    if (include_leaf_parents == 0){
        for (long i = 0; i < num_leaves; i++){
            sum += abs(current_tree->tree[i].parent - dest_tree->tree[i].parent);
            // printf("i: %ld, sum: %ld\n", i, sum);
        }
    }
    long* mrcas = mrca_list(current_tree, dest_tree);
    for (long i = num_leaves; i < 2*num_leaves-1; i++){
        sum += (mrcas[i] - i);
    }
    return(sum);
}


// decrease the mrca of node1 and node2 in tree by a (unique) RNNI move
int decrease_mrca(Tree* tree, long node1, long node2){
    long num_leaves = tree->num_leaves;
    long current_mrca = mrca(tree, node1, node2);
    // deep copy tree
    Tree* neighbour = malloc(sizeof(Node*) + 3 * sizeof(long));
    neighbour->tree = malloc((2 * num_leaves - 1) * sizeof(Node));
    neighbour->num_leaves = num_leaves;
    for (long i = 0; i < 2 * num_leaves - 1; i++){
        neighbour->tree[i] = tree->tree[i];
    }
    if (neighbour->tree[current_mrca-1].parent == current_mrca){ // edge -> NNI move
        // we try both possible NNI move and see which one decreases the rank of the mrca
        nni_move(neighbour, current_mrca-1, 0);
        if(mrca(neighbour,node1,node2)>=current_mrca){
            // we did not decrease the rank of the mrca by this nni move, so we need to do the other one
            // but first we need to reset neighbour to tree:
            for (long i = 0; i < 2 * num_leaves - 1; i++){
                neighbour->tree[i] = tree->tree[i];
            }
            nni_move(neighbour, current_mrca-1, 1);
        }
    } else{ // otherwise, we make a rank move
        rank_move(neighbour, current_mrca - 1);
    }
    // now update tree to become neighbour
    for (long i = 0; i < 2 * num_leaves - 1; i++){
        tree->tree[i] = neighbour->tree[i];
    }
    return 0;
}


long sum_symmetric_cluster_diff(Tree* tree1, Tree* tree2){
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


// Compute symmetric difference of clusters induced by nodes of rank k in tree1 and tree2
long symmetric_cluster_diff(Tree* tree1, Tree* tree2, long k){
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


// compute length of shortest path among those that only have rank moves (we can use top-down mrca decreasing approach here!)
long shortest_rank_path(Tree* tree1, Tree* tree2){
    long num_leaves = tree1->num_leaves;
    // Deep copy tree1 to perform moves on that tree
     Tree* current_tree = malloc(sizeof(Node*) + 3 * sizeof(long));
    current_tree->num_leaves = num_leaves;
    current_tree->tree = malloc((2 * num_leaves - 1) * sizeof(Node)); // deep copy start tree
    for (long i = 0; i < 2 * num_leaves - 1; i++){
        current_tree->tree[i] = tree1->tree[i];
    }

    long path_length = 0;
    for(int i = num_leaves; i < 2 * num_leaves - 1; i++){
        if (!((current_tree->tree[i].children[0] == tree2->tree[i].children[0] && current_tree->tree[i].children[1] == tree2->tree[i].children[1])||
        (current_tree->tree[i].children[0] == tree2->tree[i].children[1] && current_tree->tree[i].children[1] == tree2->tree[i].children[0]))){
            long current_mrca = mrca(current_tree, tree2->tree[i].children[0], tree2->tree[i].children[1]);
            while(current_mrca != i){
                rank_move(current_tree, current_mrca-1);
                current_mrca--;
                path_length++;
            }
        }
    }
    return path_length;
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


int first_iteration(Tree_List* treelist, long node1, long node2, long r){
    // perform one iteration of FP on every tree in tree_list, such that resulting tree has mrca of i and j at position r
    // Note that this may change every tree in treelist
    for(long i = 0; i < treelist->num_trees; i++){
        while(mrca(&treelist->trees[i], node1, node2) > r){
            decrease_mrca(&treelist->trees[i], node1, node2);
        }
    }
    return 0;
}


long sos(Tree_List* treelist, Tree* focal_tree){
    // compute sum of squared distances for all tree in treelist to focal_tree
    long sos = 0;
    for(long i = 0; i < treelist->num_trees; i++){
        sos += findpath_distance(&treelist->trees[i], focal_tree);
    }
    return sos;
}