/* author: Lena Collienne
basic algorithms for ranked trees*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

/* Structure for an internal node of a tree*/
typedef struct Node{
    long parent;
    long children[2];
} Node;


// A tree is an array of nodes ordered according to their ranks (first n nodes are leaves, order there doesn't matter) + number of leaves (long)
typedef struct Tree{
    long num_leaves;
    Node * tree;
} Tree;


// List of trees (e.g. as output of NNI move (2 trees) or findpath(d trees))
typedef struct Tree_List{
    int num_trees;
    Tree * trees;
} Tree_List;


typedef struct Path{
    long length;
    long ** moves;
} Path;

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

/*Start of tree functions. */

// read trees from file and return them in a Tree_List
Tree_List read_trees_from_file(char* filename){
    FILE *f;
    if ((f = fopen(filename, "r"))){
        long num_leaves;
        int num_trees;
        char * first_ints_buffer = malloc(20 * sizeof(char)); // max number of digits for number of leaves and number of trees in file is assumed to be 20
        // read in first two lines: number of leaves and number of trees
        if (fgets(first_ints_buffer, 20 * sizeof(char), f) != NULL){
            num_leaves = atol(first_ints_buffer);
            if (num_leaves == 0){
                printf("Error. Determine number of leaves in input file.\n");
            }
        } // TODO: add else to give error when input file does not have the right format
        if (fgets(first_ints_buffer, 20 * sizeof(char), f) != NULL){
            num_trees = atoi(first_ints_buffer);
            if (num_trees == 0){
                printf("Error. Determine number of trees in input file.\n");
            }
        }
        free(first_ints_buffer);

        long num_nodes = 2 * num_leaves;
        int num_digits_n = get_num_digits(num_leaves); // number of digits of the int num_leaves
        long max_str_length = 2 * num_leaves * num_leaves * num_digits_n * 2; //upper bound for the maximum length of a tree as string -- this is quite a bad approximation and should be improved (?)
        Tree_List tree_list;
        tree_list.num_trees = num_trees;
        tree_list.trees = malloc(num_trees * sizeof(Tree));
        for (int i = 0; i < num_trees; i++){
            tree_list.trees[i].tree = malloc(num_nodes * sizeof(Node));
            tree_list.trees[i].num_leaves = num_leaves;
            for (long j = 0; j < num_nodes; j++){
                tree_list.trees[i].tree[j].parent = -1;
                tree_list.trees[i].tree[j].children[0] = -1;
                tree_list.trees[i].tree[j].children[1] = -1;
            }
        }

        long *highest_ancestor = malloc(num_leaves * sizeof(long)); // highest_ancestor[i]: index of cluster containing leaf i that is highest below the currently considered cluster
        for(long i = 0; i < num_leaves; i++){
            highest_ancestor[i] = 1;
        }
        char *buffer = malloc(max_str_length * sizeof(char));
        for(long i = 0; i < max_str_length; i++){
            buffer[i] = '\0';
        }
        long tree_index = 0;
        //loop through lines (trees) in file
        while(fgets(buffer, max_str_length * sizeof(char), f) != NULL){
            // Remove white spaces from string buffer
            long l = 0;
            for(long k=0;buffer[k]!='\0';++k)
            {
                if(buffer[k]!=' ')
                    buffer[l++]=buffer[k];
            }
            buffer[l]='\0';
            // allocate memory for strings saving clusters
            char *tree_str = malloc(max_str_length * sizeof(char));
            for(long i = 0; i < max_str_length; i++){
                tree_str[i] = buffer[i];
                if(buffer[i] == ']'){
                    break;
                }
            }
            tree_str[strcspn(tree_str, "\n")] = 0; // delete newline at end of each line that has been read
            long rank = num_leaves;
            //Find clusters
            char * partial_tree_str = tree_str;
            char * cluster_list;
            while((cluster_list = strsep(&partial_tree_str, "}")) != NULL){
                cluster_list += 2; // ignore first two characters [{ or ,{
                if(strlen(cluster_list) > 0){ //ignore last bit (just contained ])
                    // Find leaves in clusters
                    char * cluster;
                    char * current_rank;
                    while((cluster = strsep(&cluster_list, ",")) != NULL){
                        long actual_leaf = atol(cluster);
                        // update node relations if current leaf appears for first time
                        if(tree_list.trees[tree_index].tree[actual_leaf - 1].parent == -1){
                            tree_list.trees[tree_index].tree[actual_leaf - 1].parent = rank;
                            // update the current internal node (rank) to have the current leaf as child
                            if(tree_list.trees[tree_index].tree[rank].children[0] == -1){
                                tree_list.trees[tree_index].tree[rank].children[0] = actual_leaf - 1;
                            } else{
                                tree_list.trees[tree_index].tree[rank].children[1] = actual_leaf - 1;
                            }
                        } else {
                            tree_list.trees[tree_index].tree[highest_ancestor[actual_leaf - 1]].parent = rank;
                            // update cluster relation if actual_leaf already has parent assigned (current cluster is union of two clusters or cluster and leaf)
                            if(tree_list.trees[tree_index].tree[rank].children[0] == -1 || tree_list.trees[tree_index].tree[rank].children[0] == highest_ancestor[actual_leaf - 1]){ // first children should not be assigned yet. I if contains same value, overwrite that one
                                tree_list.trees[tree_index].tree[rank].children[0] = highest_ancestor[actual_leaf - 1];
                            } else if (tree_list.trees[tree_index].tree[rank].children[1] == -1)
                            {
                                tree_list.trees[tree_index].tree[rank].children[1] = highest_ancestor[actual_leaf - 1];
                            }
                        }
                        // set new highest ancestor of leaf
                        highest_ancestor[actual_leaf - 1] = rank;
                    }
                }
                rank++;
            }
            tree_index++;
            free(tree_str);
        }
        free(buffer);
        free(highest_ancestor);

        // //check if read_trees_from_file reads trees correctly
        // for (int k = 0; k < num_trees; k++){
        //     for(long i = 0; i < num_nodes; i++){
        //         if (i < num_leaves){
        //             // printf("highest ancestor of node %d has rank %d\n", i, highest_ancestor[i] + 1);
        //             printf("leaf %ld has parent %ld\n", i+1, tree_list.trees[k].tree[i].parent);
        //         } else{
        //             printf("node %ld has children %ld and %ld\n", i, tree_list.trees[k].tree[i].children[0], tree_list.trees[k].tree[i].children[1]);
        //             printf("leaf %ld has parent %ld\n", i+1, tree_list.trees[k].tree[i].parent);
        //         }
        //     }
        // }

        fclose(f);

        return tree_list;
    } else{
        printf("Error. File doesn't exist.\n");
    }
}

/*read one tree from a string*/
Tree read_tree_from_string(int num_leaves, char* tree_string){
    // Deep copy tree string
    int str_length = strlen(tree_string);
    char * tree_str = malloc((str_length + 1) * sizeof(char));
    strcpy(tree_str, tree_string);
    // Remove white spaces from string buffer
    int l = 0;
    for(int k=0;tree_str[k]!=']';++k)
    {
        if(tree_str[k]!=' ')
            tree_str[l++]=tree_str[k];
    }
    tree_str[l] = ']';
    tree_str[++l] = '\0';

    int num_nodes = num_leaves*2 - 1;
    int num_digits_n = get_num_digits(num_leaves); // number of digits of the int num_leaves

    Tree output_tree;
    output_tree.tree = malloc(num_nodes * sizeof(Node));
    for (int i = 0; i < num_nodes; i ++){
        output_tree.tree[i].parent = -1;
        output_tree.tree[i].children[0] = -1;
        output_tree.tree[i].children[1] = -1;
    }
    output_tree.num_leaves = num_leaves;

    int *highest_ancestor = malloc(num_leaves * sizeof(int)); // highest_ancestor[i]: index of cluster containing leaf i that is highest below the currently considered cluster
    for(int i = 0; i < num_leaves; i++){
        highest_ancestor[i] = 1;
    }
    int current_tree = 0; //This will stay 0 as we only read one tree

    // allocate memory for strings saving clusters
    char *cluster;

    int rank = num_leaves;
    int cluster_number = 1; // index of cluster we currently consider -- max is num_leaves - 1
    //Find clusters
    char * partial_tree_str = tree_str;
    char * cluster_list;
    while((cluster_list = strsep(&partial_tree_str, "}")) != NULL){
        cluster_list += 2; // ignore first two characters [{ or ,{
        if(cluster_number < num_leaves){ //only consider clusters, not things that could potentially be in rest of tree string after the actual tree
            // Find leaves in clusters
            while((cluster = strsep(&cluster_list, ",")) != NULL){
                int actual_leaf = atoi(cluster);
                // update node relations if current leaf appears for first time
                if(output_tree.tree[actual_leaf - 1].parent == -1){
                    output_tree.tree[actual_leaf - 1].parent = rank;
                    // update the current internal node (rank) to have the current leaf as child
                    if(output_tree.tree[rank].children[0] == -1){
                        output_tree.tree[rank].children[0] = actual_leaf - 1;
                    } else{
                        output_tree.tree[rank].children[1] = actual_leaf - 1;
                    }
                } else {
                    output_tree.tree[highest_ancestor[actual_leaf - 1]].parent = rank;
                    // update cluster relation if actual_leaf already has parent assigned (current cluster is union of two clusters or cluster and leaf)
                    if(output_tree.tree[rank].children[0] == -1 || output_tree.tree[rank].children[0] == highest_ancestor[actual_leaf - 1]){ // first children should not be assigned yet. I if contains same value, overwrite that one
                        output_tree.tree[rank].children[0] = highest_ancestor[actual_leaf - 1];
                    } else if (output_tree.tree[rank].children[1] == -1)
                    {
                        output_tree.tree[rank].children[1] = highest_ancestor[actual_leaf - 1];
                    }
                }
                highest_ancestor[actual_leaf - 1] = rank;
            }
        }
        cluster_number++;
        rank++;
    }
    free(cluster_list);
    free(tree_str);
    free(highest_ancestor);

    // // check if read_trees_from_file reads trees correctly
    // for (int k = 0; k < 1; k++){
    //     for(int i = 0; i < num_nodes; i++){
    //         if (i < num_leaves){
    //             // printf("highest ancestor of node %d has rank %d\n", i, highest_ancestor[i] + 1);
    //             printf("leaf %d has parent %ld\n", i+1, output_tree.tree[i].parent);
    //         } else{
    //             printf("node %d has children %ld and %ld\n", i, output_tree.tree[i].children[0], output_tree.tree[i].children[1]);
    //             printf("leaf %d has parent %ld\n", i+1, output_tree.tree[i].parent);
    //         }
    //     }
    // }
    return output_tree;
}


char* tree_to_string(Tree * input_tree){
    if (input_tree->tree == NULL){
        printf("Error. Can't write tree. Given tree doesn't exist.\n");
    } else{
        long num_leaves = input_tree->num_leaves;
        int num_digits_n = get_num_digits(input_tree->num_leaves); // number of digits of the int num_leaves
        long max_str_length = 2 * num_leaves * num_leaves * num_digits_n; //upper bound for the maximum length of a tree as string
        char *tree_str = malloc(2 * max_str_length * sizeof(char));

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
                    sprintf(leaf_str, "%ld,", j + 1);
                    strcat(tree_str, leaf_str);
                }
            }
            tree_str_pos = strlen(tree_str) - 1;
            tree_str[tree_str_pos] = '\0'; // delete last comma
            strcat(tree_str, "},{");
            tree_str_pos +=2;
        }
        tree_str[tree_str_pos] = '\0'; // delete ,{ at end of tree_str
        tree_str[tree_str_pos - 1] = '\0';
        strcat(tree_str, "]");

        for (long i = 0; i < num_leaves - 1; i++){
            free(clusters[i]);
        }
        free(clusters);

        return(tree_str);
    }
}

// write one tree into given file -- runtime quadratic
void write_tree(Tree * input_tree, char * filename){
    char * tree_str = tree_to_string(input_tree);
    // write tree as string to file
    FILE *f;
    f = fopen(filename, "a"); //add tree at end of output file
    fprintf(f, "%s\n", tree_str); //This adds a new line at the end of the file -- we need to be careful when we read from such files!
    fclose(f);
}


// write a Tree_List of trees into a given file in the format we require as input for read_trees_from_file (see README.md)
void write_trees(Tree_List * tree_list, char * filename){
    FILE *f;
    f = fopen(filename, "w");
    fclose(f);
    for (int i = 0; i < tree_list->num_trees; i++){
        Tree * tree_to_write;
        tree_to_write = &tree_list->trees[i];
        write_tree(tree_to_write, filename);
    }
}
/** Length move not finshed.*/
int length_move(Tree * input_tree, long rank) {
  Node temp_node;
  if (input_tree->tree == NULL){
      fprintf(stderr, "Error. No length move possible. Given tree doesn't exist.\n");
      return 1;
  } else if(input_tree->tree[rank].parent == rank + 1) {
    fprintf(stderr, "Error. No length move possible. length between parent is not > 1.\n");
    return 1;
  }
  /*update the parent of the node you are increasing the rank of */
  /*figure out which child, then update. currently just 0???*/
  input_tree->tree[input_tree->tree[rank].parent].children[0] = 2;
  /*update the children*/
  input_tree->tree[input_tree->tree[rank].children[0]].parent = rank+1;
  input_tree->tree[input_tree->tree[rank].children[1]].parent = rank+1;
  temp_node = input_tree->tree[rank];
  input_tree->tree[rank+1] = temp_node;
  input_tree->tree = NULL;
  return 0;
}


// NNI move on edge bounded by rank rank_in_list and rank_in_list + 1, moving child_stays (index) of the lower node up
int nni_move(Tree * input_tree, long rank_in_list, int child_moves_up){
    if (input_tree->tree == NULL){
        printf("Error. No RNNI move possible. Given tree doesn't exist.\n");
    } else{
        long num_leaves = input_tree->num_leaves;
        long num_nodes = 2 * num_leaves - 1;
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


// Make a rank move on tree between nodes of rank rank and rank + 1 (if possible)
int rank_move(Tree * input_tree, long rank_in_list){
    long num_leaves = input_tree->num_leaves;
    long num_nodes = 2 * num_leaves - 1;
    if (input_tree->tree == NULL){
        printf("Error. No rank move possible. Given tree doesn't exist.\n");
        return 1;
    } else{
        if (input_tree->tree[rank_in_list].parent == rank_in_list + 1){
            printf("Error. No rank move possible. The interval [%ld,%ld] is an edge!\n", rank_in_list, rank_in_list + 1);
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


// returns a path in matrix representation -- explanation in data_structures.md
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
        current_tree.tree = malloc((2 * num_leaves) * sizeof(Node));
        current_tree.num_leaves = num_leaves;
        for (long i = 0; i < 2 * num_leaves; i++){
            current_tree.tree[i] = start_tree->tree[i];
        }
        Tree * current_tree_pointer;
        current_tree_pointer = &current_tree;
        for (long i = num_leaves; i < 2 * num_leaves; i++){/*#2*/
            current_mrca = mrca(current_tree_pointer, dest_tree->tree[i].children[0], dest_tree->tree[i].children[1]);
            // move current_mrca down
            while(current_mrca != i){ /*#3*/
                bool did_nni = false;
                /*find out if one of the children of current_tree.tree[current_mrca] has rank current_mrca - 1. If this is the case, we want to make an NNI */
                for (int child_index = 0; child_index < 2; child_index++){
                  if (did_nni == false && current_tree.tree[current_mrca].children[child_index] == current_mrca - 1){ /*do nni if current interval is an edge
                        check which of the children of current_tree.tree[current_mrca] should move up by the NNI move #4*/
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
                if (did_nni == false){ /*#6*/
                  /*start here */
                  int min_i;
                  for(int l = num_leaves*2; l > i; l--){
                    if(current_tree.tree[l].parent == -1) {
                      min_i = l;
                    }
                  }
                  for(int j = min_i-1; j <= i; j++) {
                    length_move(current_tree_pointer, j);
                    //T2 is T1 where the time of T1(j) is increased by 1. (length move.)

                    //p = p + T1.
                    /*path.moves[path_index][1] = 0; fix this and a new length move symbol later.*/
                  }
                  current_mrca--;
                    // rank_move(current_tree_pointer, current_mrca - 1);/*#7*/
                    // path.moves[path_index][1] = 0;
                    // current_mrca--;
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
    return path;/*#10*/
}


// returns only the distance
long findpath_distance(Tree *start_tree, Tree *dest_tree){
    int count = 0; // counter to print the progress of the algorithm (in 10% steps of max distance)
    long num_leaves = start_tree->num_leaves;
    long max_dist = ((num_leaves - 1) * (num_leaves - 2))/2 + 1;
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
                        did_nni = true;
                        current_mrca--;
                    }
                }
                if (did_nni == false){
                    rank_move(current_tree_pointer, current_mrca - 1);
                    current_mrca--;
                }
                path_index++;
                // Print progress (in 5% steps from max distance)
                // if (count < (float) path_index / (float) max_dist){
                //      printf("%d Percent of maximum distance reached\n", (int) (100 * count));
                //      count += 0.05;
                // }
            }
        }
        free(current_tree.tree);
    }
    return path_index;
}


// returns the path that findpath computed between two trees given in a Tree_List
Tree_List return_findpath(Tree_List tree_list){
    long path_index = 0;
    long num_leaves = tree_list.trees[0].num_leaves;
    Tree current_tree;
    current_tree.num_leaves = num_leaves;
    current_tree.tree = malloc((2 * num_leaves - 1) * sizeof(Node)); // deep copy start tree
    for (int i = 0; i < 2 * num_leaves - 1; i++){
        current_tree.tree[i] = tree_list.trees[0].tree[i];
    }

    // write_tree(tree_list.trees[1], num_leaves, "output/tree.rtree");
    // write_tree(tree_list.trees[0], num_leaves, "output/tree.rtree");

    // Compute the path to get the moves in the path matrix representation
    Tree * start_tree;
    Tree * dest_tree;
    start_tree = &tree_list.trees[0];
    dest_tree = &tree_list.trees[1];
    Path fp = findpath(start_tree, dest_tree);

    long diameter = (num_leaves - 1) * (num_leaves - 2) / 2 + 1; // this is not the diameter, but the number of trees on a path giving the diameter (= diameter + 1)

    Tree_List findpath_list; // output: list of trees on FP path
    findpath_list.num_trees = fp.length;
    findpath_list.trees = malloc(diameter * sizeof(Tree));
    for (long i = 0; i < diameter; i++){
        findpath_list.trees[i].num_leaves = num_leaves;
        findpath_list.trees[i].tree = malloc((2* num_leaves - 1) * sizeof(Node));
    }
    // findpath_list.trees[0] = current_tree.tree;
    for (long i = 0; i < 2 * num_leaves - 1; i++){
        findpath_list.trees[0].tree[i] = current_tree.tree[i];
    }

    // create actual path by doing moves starting at tree_list.trees[0] with the information in the matrix returned form fp above
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

char** findpath_cluster_list(int num_leaves, char* input_tree1, char* input_tree2){
    // Returns the path computed by findpath as an array of strings where the first element is the number of trees on this path

    Tree tree1 = read_tree_from_string(num_leaves, input_tree1);
    Tree tree2 = read_tree_from_string(num_leaves, input_tree2);
    Tree_List findpath_input;
    findpath_input.num_trees = 2;
    findpath_input.trees = malloc(2 * sizeof(Tree));
    findpath_input.trees[0] = tree1;
    findpath_input.trees[1] = tree2;
    // start_tree = &tree1;
    // dest_tree = &tree2;

    // run FindPath
    int max_dist = ((num_leaves - 1) * (num_leaves - 2))/2 + 1;
    Tree_List path = return_findpath(findpath_input);

    char** output = malloc((max_dist + 1) * sizeof(char* ));
    output[0] = malloc(sizeof(char*));
    sprintf(output[0], "%d", path.num_trees); // first entry in output is number of trees on path
    for (int k = 1; k < path.num_trees+1; k++){
        // output[k] = malloc((num_leaves-1) * (num_leaves-1) * sizeof(char));
        output[k] = tree_to_string(&path.trees[k-1]);
    }
    for (int i = 0; i < max_dist; i++){
        free(path.trees[i].tree);
    }
    free(path.trees);
    free(findpath_input.trees);
    free(tree1.tree);
    free(tree2.tree);
    // TODO: Memory allocation / FREE

    return(output);

}

// Computing the RNNI distance from input trees as strings and number of leaves -- mainly to be executed from Python
int distance(int num_leaves, char* input_tree1, char* input_tree2){

    int num_nodes = 2 * num_leaves - 1;

    // Tree_List tree1;
    // tree1.trees = malloc(sizeof(Node*));
    // tree1.trees[0] = malloc(num_nodes * sizeof(Node));

    // Tree_List tree2;
    // tree2.trees = malloc(sizeof(Node*));
    // tree2.trees[0] = malloc(num_nodes * sizeof(Node));


    // Conveof(char*)rt trees into list of nodes
    Tree tree1 = read_tree_from_string(num_leaves, input_tree1);
    Tree tree2 = read_tree_from_string(num_leaves, input_tree2);
    Tree * start_tree;
    Tree * dest_tree;
    start_tree = &tree1;
    dest_tree = &tree2;

    // run FindPath
    int max_dist = ((num_leaves - 1) * (num_leaves - 2))/2 + 1;
    Path fp = findpath(start_tree, dest_tree);
    int output = fp.length;

    free(tree1.tree);
    free(tree2.tree);

    for (int i = 0; i < max_dist + 1; i++){
        free(fp.moves[i]);
    }
    free(fp.moves);
    return(output);
}

/* End of tree functions. */


int main(){
    // char filename[200]; // length of filename set to be 200 char max
    // printf("What is the file containing trees?\n");
    // scanf("%s", filename);
    //
    // printf("Start reading trees from file\n");
    // Tree_List tree_list = read_trees_from_file(filename);
    // printf("End reading trees from file\n");
    long num_leaves = 4;
    long num_nodes = 2 * num_leaves;
    Tree_List tree_list;
    tree_list.num_trees = 2;
    tree_list.trees = malloc(tree_list.num_trees * sizeof(Tree));
    tree_list.trees[0].num_leaves = num_leaves;
    tree_list.trees[1].num_leaves = num_leaves;
    for (int i = 0; i < tree_list.num_trees; i++){
        tree_list.trees[i].tree = malloc(num_nodes * sizeof(Node));
        tree_list.trees[i].num_leaves = num_leaves;
        for (long j = 0; j < num_nodes; j++){
            tree_list.trees[i].tree[j].parent = -1;
            tree_list.trees[i].tree[j].children[0] = -1;
            tree_list.trees[i].tree[j].children[1] = -1;
        }
    }
    /*first tree */
    tree_list.trees[0].tree[0].parent = 3;
    tree_list.trees[0].tree[1].parent = 3;
    tree_list.trees[0].tree[2].parent = 5;
    tree_list.trees[0].tree[3].parent = 5;
    tree_list.trees[0].tree[3].children[0] = 0;
    tree_list.trees[0].tree[3].children[1] = 1;
    tree_list.trees[0].tree[5].children[0] = 3;
    tree_list.trees[0].tree[0].children[1] = 2;

    /*second tree */
    tree_list.trees[0].tree[0].parent = 3;
    tree_list.trees[0].tree[1].parent = 3;
    tree_list.trees[0].tree[2].parent = 5;
    tree_list.trees[0].tree[3].parent = 5;
    tree_list.trees[0].tree[3].children[0] = 0;
    tree_list.trees[0].tree[3].children[1] = 2;
    tree_list.trees[0].tree[5].children[0] = 3;
    tree_list.trees[0].tree[0].children[1] = 2;

    // // // check if read_trees_from_file reads trees correctly
    // // for (int k = 0; k < num_trees; k++){
    // //     for(int i = 0; i < 2 * num_leaves - 1; i++){
    // //         if (i < num_leaves){
    // //             // printf("highest ancestor of node %d has rank %d\n", i, highest_ancestor[i] + 1);
    // //             printf("leaf %d has parent %d\n", i+1, tree_list.trees[k][i].parent);
    // //         } else{
    // //             printf("node %d has children %d and %d\n", i, tree_list.trees[k][i].children[0], tree_list.trees[k][i].children[1]);
    // //             printf("node %d has parent %d\n", i, tree_list.trees[k][i].parent);
    // //         }
    // //     }
    // // }

    // // read_tree_from_string(5, "[{1,2},{1,2,3},{1,2,3,4},{1,2,3,4,5}]"); // test function when for reading tree as string

    // write_trees(tree_list, "./output/output.rtree"); // write given trees into file
    // Tree_List findpath_list = return_findpath(tree_list); // write FP into file
    // int ** fp = findpath(tree_list.trees[0], tree_list.trees[1], tree_list.num_leaves); //run FP

    Tree * start_tree;
    Tree * dest_tree;
    start_tree = &tree_list.trees[0];
    dest_tree = &tree_list.trees[1];

    printf("Start running FindPath\n");
    clock_t start_time = time(NULL);
    Path path = findpath(start_tree, dest_tree);
    long distance = path.length;
    // long distance = findpath_distance(start_tree, dest_tree);
    clock_t end_time = time(NULL);
    printf("End running FindPath\n");
    long max_dist = ((num_leaves - 1) * (num_leaves - 2))/2 + 1;
    // int distance = fp[0][0];
    // for (int i = 0; i < max_dist + 1; i++){
    //     free(fp[i]);
    // }
    // free(fp);

    char ** fpath;
    char * start_tree_str = tree_to_string(start_tree);
    char * dest_tree_str = tree_to_string(dest_tree);
    // fpath = findpath_cluster_list(num_leaves, start_tree_str, dest_tree_str);
    // printf("FP path:\n");
    // for (int i = 0; i < distance + 1; i++){
    //     printf("%s\n", fpath[i]);
    //     free(fpath[i]);
    // }
    // free(fpath);

    // Free all variables
    for (int i = 0; i < max_dist + 1; i++){
        free(path.moves[i]);
    }
    free(path.moves);

    for (int i = 0; i < tree_list.num_trees; i++){
        free(tree_list.trees[i].tree);
    }
    free(tree_list.trees);
    free(start_tree_str);
    free(dest_tree_str);

    printf("Time to compute FP(T,R): %f sec\n", difftime(end_time, start_time));
    printf("Length of fp: %ld\n", distance);
    // write_trees(findpath_list, "./output/fp.rtree");
    // printf("distance: %d\n", distance(5, "[{1,2},{1,2,3},{1,2,3,4},{1,2,3,4,5}]", "[{4,5},{3,4,5},{1,2},{1,2,3,4,5}]"));
    return 0;
}
