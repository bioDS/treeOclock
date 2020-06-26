/* author: Lena Collienne
basic algorithms for ranked trees*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/* trees will be arrays of nodes, ordered according to their ranks (first n nodes are leaves, order there doesn't matter)*/
typedef struct Node{
    int parent;
    int children[2];
} Node;

// List of trees (e.g. as output of NNI move (2 trees) or findpath(d trees))
typedef struct Tree_List{
    Node* trees; //?
} Tree_List;


int get_num_digits(int integer){
    int n = integer;
    int num_digits = 0;
    while(n != 0){
        n /= 10;
        num_digits++;
    }
    return num_digits;
}

/*read tree  from file*/
Tree_List* read_trees(int num_leaves){

    char filename[200]; // length of filename set to be 200 char max
    printf("What is the file containing trees?\n");
    scanf("%s", filename);
    int num_nodes = num_leaves*2 - 1;
    int num_digits_n = get_num_digits(num_leaves); // number of digits of the int num_leaves
    int max_str_length = 2 * num_leaves * num_leaves * num_digits_n; //upper bound for the maximum length of a tree as string
    //TODO: allocate space for the tree_list that can save as many trees as there are lines in this file!
    int num_trees = 2;
    Tree_List * tree_list = malloc(num_trees * sizeof(Tree_List)); // is this correct? 
    for (int i = 0; i < num_trees; i++){
        tree_list[i].trees = malloc(num_nodes * sizeof(Node));
        memset(tree_list[i].trees, -1, num_nodes * sizeof(Node));
    }

    int *highest_ancestor = malloc(num_leaves * sizeof(int)); // highest_ancestor[i]: index of cluster containing leaf i that is highest below the currently considered cluster
    memset(highest_ancestor, 1, num_leaves * sizeof(int));
    char *buffer = malloc(max_str_length * sizeof(char));
    memset(buffer, '\0', max_str_length * sizeof(char));
    int current_tree = 0;
    //loop through lines (trees) in file
    FILE *f;
    if ((f = fopen(filename, "r"))){
        while(fgets(buffer, max_str_length * sizeof(char), f) != NULL){
            char *cluster_list, *cluster; //malloc()???
            char * tree_str = malloc(strlen(buffer) * sizeof(char));
            strcpy(tree_str, buffer);
            tree_str[strcspn(tree_str, "\n")] = 0; // delete newline at end of each line that has been read
            int rank = num_leaves;
            //Find clusters
            while((cluster_list = strsep(&tree_str, "}")) != NULL){
                cluster_list += 2; // ignore first two characters [{ or ,{
                if(strlen(cluster_list) > 0){ //ignore last bit (just contained ])
                    // Find leaves in clusters
                    while((cluster = strsep(&cluster_list, ",")) != NULL){
                        int actual_node = atoi(cluster);
                        // update node relations if current leaf appears for first time
                        if(tree_list[current_tree].trees[actual_node - 1].parent == -1){
                            tree_list[current_tree].trees[actual_node - 1].parent = rank;
                            // update the current internal node (rank) to have the current leaf as child
                            if(tree_list[current_tree].trees[rank].children[0] == -1){
                                tree_list[current_tree].trees[rank].children[0] = actual_node - 1;
                            } else{
                                tree_list[current_tree].trees[rank].children[1] = actual_node - 1;
                            }
                        } else {
                            tree_list[current_tree].trees[highest_ancestor[actual_node - 1]].parent = rank;
                            // update cluster relation if actual_node already has parent assigned (current cluster is union of two clusters or cluster and leaf)
                            if(tree_list[current_tree].trees[rank].children[0] == -1 || tree_list[current_tree].trees[rank].children[0] == highest_ancestor[actual_node - 1]){ // first children should not be assigned yet. I if contains same value, overwrite that one
                                tree_list[current_tree].trees[rank].children[0] = highest_ancestor[actual_node - 1];
                            } else if (tree_list[current_tree].trees[rank].children[1] == -1)
                            {
                                tree_list[current_tree].trees[rank].children[1] = highest_ancestor[actual_node - 1];
                            }
                        }
                        // set new highest ancestor of leaf
                        highest_ancestor[actual_node - 1] = rank;
                    }
                }
                rank++;
            }
            current_tree++;
        }
        fclose(f);
        free(buffer);
        free(highest_ancestor);
        return tree_list;
    } else{
        printf("Error. File doesn't exist.\n");
        return NULL;
    }
    
    // //check if read_trees reads trees correctly
    // for (int k = 0; k < num_trees; k++){
    //     for(int i = 0; i < 2 * num_leaves - 1; i++){
    //         if (i < num_leaves){
    //             printf("highest ancestor of node %d has rank %d\n", i, highest_ancestor[i] + 1);
    //             printf("leaf %d has parent %d\n", i+1, tree_list[k].trees[i].parent);
    //         } else{
    //             printf("node %d has children %d and %d\n", i, tree_list[k].trees[i].children[0], tree_list[k].trees[i].children[1]);
    //             printf("leaf %d has parent %d\n", i+1, tree_list[k].trees[i].parent);
    //         }
    //     }
    // }
}


// write tree into given file -- runtime quadratic
void write_tree(Node * tree, int num_leaves, char * filename){
    if (tree == NULL){
        printf("Error. Can't write tree. Given tree doesn't exist.\n");
    } else{
        int num_digits_n = get_num_digits(num_leaves); // number of digits of the int num_leaves
        int max_str_length = 2 * num_leaves * num_leaves * num_digits_n; //upper bound for the maximum length of a tree as string
        char *tree_str = malloc(max_str_length * sizeof(char));

        // create matrix cluster*leaves -- 0 if leaf is not in cluster, 1 if it is in cluster
        // int clusters[num_leaves - 1][num_leaves]; // save all clusters as list of lists TODO: malloc

        int ** clusters = malloc((num_leaves) * sizeof(int *));    
        for (int i = 0; i < num_leaves; i++){
            clusters[i] = malloc((num_leaves - 1) * sizeof(int));
        }

        for (int i = 0; i <num_leaves ; i++){
            for (int j = 0; j < num_leaves - 1; j++){
                clusters[j][i] = 0; //initialise all entries to be 0
            }
            int j = i;
            while (tree[j].parent != -1){
                j = tree[j].parent;
                clusters[j - num_leaves][i] = 1;
            }
            clusters[num_leaves - 1][i] = 1;
        }

        // convert matrix into output string tree_str
        sprintf(tree_str, "[{");
        int tree_str_pos = 1; //last position in tree_str that is filled with a character
        for (int i = 0; i < num_leaves - 1; i++){
            for (int j = 0; j < num_leaves; j++){
                if (clusters[i][j] == 1){
                    char leaf_str[num_digits_n + 1];
                    sprintf(leaf_str, "%d,", j + 1);
                    strcat(tree_str, leaf_str);
                    tree_str_pos += 2;
                }
            }
            tree_str[tree_str_pos] = '\0'; // delete last komma
            strcat(tree_str, "},{");
            tree_str_pos +=2;
        }
        tree_str[tree_str_pos] = '\0'; // delete ,{ at end of tree_str
        tree_str[tree_str_pos - 1] = '\0';
        strcat(tree_str, "]");
        printf("%s\n", tree_str);

        for (int i = 0; i < num_leaves - 1; i++){
            free(clusters[i]);
        }
        free(clusters);

        // write tree as string to file
        FILE *f;
        f = fopen(filename, "a"); //add tree at end of output file
        fprintf(f, "%s\n", tree_str); //This adds a new line at the end of the file -- we need to be careful when we read from such files!
        fclose(f);
    }
}


int nni_move(Node * tree, int rank_in_list, int num_leaves, int child_moves_up){
    // NNI move on edge bounded by rank rank_in_list and rank_in_list + 1, moving child_stays (index) of the lower node up
    if (tree == NULL){
        printf("Error. No NNI move possible. Given tree doesn't exist.\n");
    } else{
        int num_nodes = 2 * num_leaves - 1;
        if(tree[rank_in_list].parent != rank_in_list + 1){
            printf("Can't do an NNI - interval [%d, %d] is not an edge!\n", rank_in_list, rank_in_list + 1);
            return 1;
        } else{
            int child_moved_up;
            for (int i = 0; i < 2; i++){
                if (tree[rank_in_list+1].children[i] != rank_in_list){ //find the child of the node of rank_in_list k+1 that is not the node of rank_in_list k
                    //update parent/children relations to get nni neighbour
                    tree[tree[rank_in_list+1].children[i]].parent = rank_in_list; //update parents
                    tree[tree[rank_in_list].children[child_moves_up]].parent = rank_in_list+1;
                    child_moved_up = tree[rank_in_list].children[child_moves_up];
                    tree[rank_in_list].children[child_moves_up] = tree[rank_in_list+1].children[i]; //update children
                    tree[rank_in_list+1].children[i] = child_moved_up;
                }
            }
        }
    }

    // //check if read_trees reads trees correctly
    // for (int k = 0; k < 1; k++){
    //     for(int i = 0; i < 2 * num_leaves - 1; i++){
    //         if (i < num_leaves){
    //             printf("leaf %d has parent %d\n", i+1, tree[i].parent);
    //         } else{
    //             printf("node %d has children %d and %d\n", i, tree[i].children[0], tree[i].children[1]);
    //             printf("node %d has parent %d\n", i, tree[i].parent);
    //         }
    //     }
    // }
    // write_tree(tree, num_leaves, "./output/output.rtree");

    return 0;
}

int rank_move(Node * tree, int rank_in_list, int num_leaves){
    // Make a rank move on tree between nodes of rank rank and rank + 1 (if possible)
    int num_nodes = 2 * num_leaves - 1;
    if (tree == NULL){
        printf("Error. No rank move possible. Given tree doesn't exist.\n");
        return 1;
    } else{
        if (tree[rank_in_list].parent == rank_in_list + 1){
            printf("Error. No rank move possible. The interval [%d,%d] is an edge!\n", rank_in_list, rank_in_list + 1);
        } else{
            // update parents of nodes that swap ranks
            int upper_parent;
            upper_parent = tree[rank_in_list + 1].parent;
            tree[rank_in_list + 1].parent = tree[rank_in_list].parent;
            tree[rank_in_list].parent = upper_parent;

            int upper_child;
            int parent;
            for (int i = 0; i < 2; i++){
                upper_child = tree[rank_in_list + 1].children[i];
                // update children of nodes that swap ranks
                tree[rank_in_list + 1].children[i] = tree[rank_in_list].children[i];
                tree[rank_in_list].children[i] = upper_child;
                // update parents of children of nodes that swap ranks
                tree[tree[rank_in_list + 1].children[i]].parent = rank_in_list + 1; 
                tree[tree[rank_in_list].children[i]].parent = rank_in_list;
                // update children of parents of nodes that swap rank
                if (tree[tree[rank_in_list + 1].parent].children[i] == rank_in_list){ //parent pointer of tree[rank_in_list + 1] is already set correctly!
                    tree[tree[rank_in_list + 1].parent].children[i] = rank_in_list + 1;
                }
                if (tree[tree[rank_in_list].parent].children[i] == rank_in_list + 1){
                    tree[tree[rank_in_list].parent].children[i] = rank_in_list;
                }
            }
        }
    }
    return 0;
}

int mrca(Node * tree, int node1, int node2){
    // find mrca of nodes with positions node1 and node2 in tree
    int rank1 = node1;
    int rank2 = node2;
    while (rank1 != rank2){
        if (rank1 < rank2){
            rank1 = tree[rank1].parent;
        } else{
            rank2 = tree[rank2].parent;
        }
    }
    return rank1;
}


int ** findpath(Node *start_tree, Node *dest_tree, int num_leaves){
    // returns a path in matrix representation -- explanation in data_structures.md
    int max_dist = ((num_leaves - 1) * (num_leaves - 2))/2;
    int ** moves = malloc(max_dist * sizeof(int)); // save moves in a table: each row is move, column 1: rank of lower node bounding the interval of move, column 2: 0,1,2: rank move, nni where children[0] stays, nni where children[1] stays
    for (int i = 0; i < max_dist; i++){
        moves[i] = malloc(2 * sizeof(int));
        moves[i][0] = 0;
        moves[i][1] = 0;
    }
    int path_index = 0; // next position on path that we want to fill with a tree pointer
    if (start_tree == NULL){
        printf("Error. Start tree doesn't exist.\n");
    } else if (dest_tree == NULL){
        printf("Error. Destination tree doesn't exist.\n");
    } else{
        remove("./output/findpath.rtree");
        // write_tree(start_tree, num_leaves, "./output/findpath.rtree"); // this ruins the running time!!!!!!!!
        int current_mrca; //rank of the mrca that needs to be moved down
        Node * current_tree = malloc((2 * num_leaves - 1) * sizeof(Node));
        current_tree =  start_tree;

        for (int i = num_leaves; i < 2 * num_leaves - 1; i++){
            current_mrca = mrca(start_tree, dest_tree[i].children[0], dest_tree[i].children[1]);
            // move current_mrca down
            while(current_mrca != i){
                bool did_nni = false;
                for (int child_index = 0; child_index < 2; child_index++){ // find out if one of the children of current_tree[current_mrca] has rank current_mrca - 1. If this is the case, we want to make an NNI
                    if (did_nni == false && current_tree[current_mrca].children[child_index] == current_mrca - 1){ // do nni if current interval is an edge
                        // check which of the children of current_tree[current_mrca] should move up by the NNI move 
                        bool found_child = false; //indicate if we found the correct child
                        int child_stays; // index of the child of current_tree[current_mrca] that does not move up by an NNI move
                        // find the index of the child of the parent of the node we currently consider -- this will be the index child_stays that we want in the end
                        int current_child_index = dest_tree[i].children[0]; // rank of already existing cluster in both current_tree and dest_tree
                        while (found_child == false){
                            while (current_tree[current_child_index].parent < current_mrca - 1){ // find the x for which dest_tree[i].children[x] is contained in the cluter induced by current_tree[current_mrca - 1]
                                current_child_index = current_tree[current_child_index].parent;
                            }
                            // find the index child_stays
                            if(current_tree[current_child_index].parent == current_mrca - 1){
                                found_child = true;
                                if (current_tree[current_tree[current_child_index].parent].children[0] == current_child_index){
                                    child_stays = 0;
                                } else{
                                    child_stays = 1;
                                }
                            } else{
                                current_child_index = dest_tree[i].children[1];
                            }
                        }
                        nni_move(current_tree, current_mrca - 1, num_leaves, 1 - child_stays);
                        moves[path_index][1] = 1 + child_stays;
                        did_nni = true;
                        current_mrca--;
                    }
                }
                if (did_nni == false){
                    rank_move(current_tree, current_mrca - 1, num_leaves);
                    moves[path_index][1] = 0;
                    current_mrca--;
                }
                moves[path_index][0] = current_mrca;
                path_index++;
            }
        }
    }
    return moves;
}


int main(){
    // TODO: instead of asking for number of leaves, find an upper bound (caterpillar trees?)
    int num_leaves;
    printf("How many leaves do your trees have?\n");
    scanf("%d", &num_leaves);
    Tree_List * trees = malloc( 2 * sizeof(Tree_List)); // because we read two trees --- Needs to be more general!!
    trees = read_trees(num_leaves);
    if (trees != NULL){
        int num_nodes;
        num_nodes = 2 * num_leaves - 1;

        write_tree(trees[0].trees, num_leaves, "./output/output.tree");
        write_tree(trees[1].trees, num_leaves, "./output/output.tree");

        // print trees on path
        int path_index = 0;
        Node * current_tree = malloc((2 * num_leaves - 1) * sizeof(Node)); // deep copy start tree
        for (int i = 0; i < 2 * num_leaves - 1; i++){
            current_tree[i] = trees[0].trees[i];
        }
        // current_tree = trees[0].trees;

        int ** fp = findpath(trees[0].trees, trees[1].trees, 5);
        int diameter = (num_leaves - 1) * (num_leaves - 2) / 2;

        while(path_index < diameter && fp[path_index][0] > 0){
            if (fp[path_index][1] == 0){
                rank_move(current_tree, fp[path_index][0], num_leaves);
            }
            else if (fp[path_index][1] == 1){
                nni_move(current_tree, fp[path_index][0], num_leaves, 1);
            } else{
                nni_move(current_tree, fp[path_index][0], num_leaves, 0);
            }
            path_index++;
            write_tree(current_tree, num_leaves, "./output/findpath.rtree");
        }
        return 0;
    }
    else{
        return 1;
    }
}