#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "tree.h"

typedef struct Node Node;
/* A tree is an array of nodes ordered according to their ranks (first n nodes
 are leaves, order there doesn't matter) + number of leaves (long)*/
 typedef struct Node{
     long parent;
     long children[2];
 };
 /* A tree is an array of nodes ordered according to their ranks
 (first n nodes are leaves, order there doesn't matter) + number of leaves (long)*/
 typedef struct Tree{
     long num_leaves;
     Node * tree;
 };
 // List of trees (e.g. as output of NNI move (2 trees) or findpath(d trees))
 typedef struct Tree_List{
     int num_trees;
     Tree * trees;
 } Tree_List;
 typedef struct Path{
     long length;
     long ** moves;
 };

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
