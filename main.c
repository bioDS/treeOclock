#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

/* DCT Space Find Path.
* Date: 26th November 2020.
* Author: Jordan Kettles
* Original RNNI space Find Path program written by Lena Collienne.*/

/* A Node is either an internal node or a leaf of a DCT tree.
* long parent The position of the node's parent in the tree array.
* long children[2] The position of the node's children in the tree array.
* children[0] is the left child, children[1] is the right child. If the node
* is a leaf node, the children are -1.
* long time The time of the node. Each internal node is assigned a unique
* time. All leaves have times of 0.
*/
typedef struct Node{
  long parent;
  long children[2];
  long time;
} Node;

/* A Tree is a set of nodes.
* Node * trees The nodes of tree, stored in an array. The first n nodes are
* leaves, followed by the internal nodes.
* long num_leaves The amount of leaves in the tree.
* long root_time The time of the root node.
*/
typedef struct Tree{
  Node * tree;
  long num_leaves;
  long root_time;
} Tree;

/* A Tree List is a list of trees.
* Tree * trees The list of trees, stored in an array.
* int num_trees The number of trees in the list.
*/
typedef struct Tree_List{
  Tree * trees;
  int num_trees;
} Tree_List;

/* A Path is a matrix representation of a list of moves to convert one tree
* into another. The first row only contains the distance between the two
* trees, and each following row represents a move. Please read
* data_structures.md for more explanation.
* long ** moves The path matrix.
* long length The length of the path.
*/
typedef struct Path{
  long ** moves;
  long length;
} Path;

/* emalloc is an error checking version of the malloc function.
* @param size_t size An amount of memory to allocate.
* @return void *r A pointer to the allocated memory.
*/
void *emalloc(size_t size) {
  void *r = malloc(size);
  if(r == NULL) {
    fprintf(stderr, "Error allocating memory.\n");
    exit(EXIT_FAILURE);
  }
  return r;
}

/* Get number of digits returns the number of digits in an int. This function
* is needed to get an upper bound on the length of an input tree as string
* when reading from a file.
* @param int integer The input number.
* @return int The number of digits in the input.
*/
int get_num_digits(int integer){
  int num_digits = 0;
  while(integer > 0){
    integer = integer / 10;
    num_digits++;
  }
  return num_digits;
}

/* Read Tree from String reads in one string and returns a DCT tree.
* @param int num_leaves The amount of leaves in the tree.
* @param char* tree_string A tree in string format.
* @return Tree the converted Tree.
*/
Tree read_tree_from_string(int num_leaves, char* tree_string){
  /*Deep copy the tree string*/
  int str_length = strlen(tree_string);
  char * tree_str = emalloc((str_length + 1) * sizeof(char));
  strcpy(tree_str, tree_string);
  long root_time = 0;
  /*Remove white spaces from string buffer*/
  int l = 0;
  for(int k=0;tree_str[k]!=']';++k)
  {
    if(tree_str[k]!=' ') {
      tree_str[l++]=tree_str[k];
    }
  }
  tree_str[l] = ']';
  tree_str[++l] = '\0';

  int num_nodes = num_leaves*2 - 1;

  Tree output_tree;
  output_tree.root_time = 0;
  output_tree.tree = emalloc(num_nodes * sizeof(Node));
  for (int i = 0; i < num_nodes; i++){
    output_tree.tree[i].parent = -1;
    output_tree.tree[i].children[0] = -1;
    output_tree.tree[i].children[1] = -1;
    output_tree.tree[i].time = -1;
  }
  output_tree.num_leaves = num_leaves;
  /*highest_ancestor[i]: index of cluster containing leaf i that is highest below the currently considered cluster*/
  int *highest_ancestor = emalloc(num_leaves * sizeof(int));
  for(int i = 0; i < num_leaves; i++){
    highest_ancestor[i] = 1;
  }
  /*allocate memory for strings saving clusters*/
  char *cluster;
  int rank = num_leaves;
  /*index of cluster we currently consider -- max is num_leaves - 1.*/
  int cluster_number = 1;
  char * partial_tree_str = tree_str;
  char * cluster_list;

  while((cluster_list = strsep(&partial_tree_str, "}")) != NULL){
    /*ignore first 1 character {*/
    cluster_list += 1;
    /*learn the rank of the current cluster, which is at the front of each cluster.*/
    long time = atol(cluster_list);
    root_time = root_time < time ? time : root_time;
    cluster_list += (2 + get_num_digits(time));
    /*only consider clusters, not things that could potentially be in rest of tree string after the actual tree*/
    if(cluster_number < num_leaves){
      /*Find leaves in clusters*/
      while((cluster = strsep(&cluster_list, ",")) != NULL){
        int actual_leaf = atoi(cluster);
        /*update node relations if the leaf appears for the first time */
        if(output_tree.tree[actual_leaf - 1].parent == -1){
          output_tree.tree[actual_leaf - 1].parent = rank;
          /*If we are on the first row we must update the parent.*/
          output_tree.tree[rank].time = time;
          output_tree.tree[actual_leaf - 1].time = 0;
          /*update the current internal node (rank) to have the current leaf as child*/
          if(output_tree.tree[rank].children[0] == -1){
            output_tree.tree[rank].children[0] = actual_leaf - 1;
          } else{
            output_tree.tree[rank].children[1] = actual_leaf - 1;
          }
          /*if the leaf is not a new leaf*/
        } else {
          output_tree.tree[rank].time = time;
          output_tree.tree[highest_ancestor[actual_leaf - 1]].parent = rank;
          /*update cluster relation if actual_leaf already has parent assigned (current cluster is union of two clusters or cluster and leaf)*/
          if(output_tree.tree[rank].children[0] == -1 || output_tree.tree[rank].children[0] == highest_ancestor[actual_leaf - 1]){
            /*first children should not be assigned yet. I if contains same value, overwrite that one*/
            output_tree.tree[rank].children[0] = highest_ancestor[actual_leaf - 1];
          } else if (output_tree.tree[rank].children[1] == -1)
          {
            output_tree.tree[rank].children[1] = highest_ancestor[actual_leaf - 1];
          }
        }
        highest_ancestor[actual_leaf - 1] = rank;
        /*printf("Highest ancestor of %d is %d\n", actual_leaf, highest_ancestor[actual_leaf - 1]);*/
      }
    }
    cluster_number++;
    rank++;
  }
  output_tree.root_time = root_time;
  free(cluster_list);
  free(tree_str);
  free(highest_ancestor);

  /*check if read_trees_from_file reads trees correctly*/
  /*for (int k = 0; k < 1; k++){
      for(int i = 0; i < num_nodes; i++){
          if (i < num_leaves){
              printf("highest ancestor of node %d has rank %d\n", i, highest_ancestor[i] + 1);
              printf("leaf %d has parent %ld\n", i+1, output_tree.tree[i].parent);
          } else{
              printf("node %d has children %ld and %ld\n", i, output_tree.tree[i].children[0], output_tree.tree[i].children[1]);
              printf("leaf %d has parent %ld\n", i+1, output_tree.tree[i].parent);
          }
      }
  }*/
  return output_tree;
}

/* Read Trees from file reads in multiple DCT trees from a file and returns a
* Tree List containing the trees. Usually reads in two trees to run FINDPATH
* on them, but can read in more. Please read the readme for more information
* on how files are formatted.
* @param char* filename the name of the file containing trees.
* @return Tree_List the list of the trees.
*/
Tree_List read_trees_from_file(char* filename){
  FILE *f;
  if ((f = fopen(filename, "r"))){
    long num_leaves = 0;
    int num_trees;
    /*max number of digits for number of leaves and number of trees in file is
     * assumed to be 20*/
    char * first_ints_buffer = emalloc(20 * sizeof(char));
    /*read in first two lines: number of leaves and number of trees*/
    if (fgets(first_ints_buffer, 20 * sizeof(char), f) != NULL){
      num_leaves = atol(first_ints_buffer);
      if (num_leaves == 0){
        printf("Error. Determine number of leaves in input file.\n");
        exit(EXIT_FAILURE);
      }
    }
    if (fgets(first_ints_buffer, 20 * sizeof(char), f) != NULL){
      num_trees = atoi(first_ints_buffer);
      if (num_trees == 0){
        printf("Error. Determine number of trees in input file.\n");
        exit(EXIT_FAILURE);
      }
    }
    free(first_ints_buffer);

    int num_digits_n = get_num_digits(num_leaves);
    /*upper bound for the maximum length of a tree as string -- this is quite
    * a bad approximation and should be improved (?) Max number of digits
    * for root time of tree assumed to be 20.*/
    long max_str_length = 2 * num_leaves * num_leaves * num_digits_n + (1+20)*num_leaves;
    Tree_List tree_list;

    /*/highest_ancestor[i]: index of cluster containing leaf i that is highest below the currently considered cluster*/
    long *highest_ancestor = emalloc(num_leaves * sizeof(long));
    for(long i = 0; i < num_leaves; i++){
      highest_ancestor[i] = 1;
    }
    char *buffer = emalloc(max_str_length * sizeof(char));
    for(long i = 0; i < max_str_length; i++){
      buffer[i] = '\0';
    }
    long tree_index = 0;
    /*loop through lines (trees) in file*/
    tree_list.trees = emalloc(num_trees * sizeof(Tree));
    tree_list.num_trees = num_trees;
    while(fgets(buffer, max_str_length * sizeof(char), f) != NULL){
      tree_list.trees[tree_index++] = read_tree_from_string(num_leaves, buffer);
    }
    free(buffer);
    free(highest_ancestor);

    /*check if read_trees_from_file reads trees correctly*/
    /* long num_nodes = 2 * num_leaves;

    for (int k = 0; k < num_trees; k++){
        for(long i = 0; i < num_nodes; i++){
            if (i < num_leaves){
                printf("highest ancestor of node %d has rank %d\n", i, highest_ancestor[i] + 1);
                printf("leaf %ld has parent %ld\n", i+1, tree_list.trees[k].tree[i].parent);
            } else{
                printf("node %ld has children %ld and %ld\n", i, tree_list.trees[k].tree[i].children[0], tree_list.trees[k].tree[i].children[1]);
                printf("leaf %ld has parent %ld\n", i+1, tree_list.trees[k].tree[i].parent);
            }
        }
    }*/

    fclose(f);
    return tree_list;
  } else{
    printf("Error. File doesn't exist.\n");
    exit(EXIT_FAILURE);
  }
}

/* Tree to String converts a Tree object into a string.
* @param Tree* input_tree The tree to convert to string.
* @return char* The tree string.
*/
char* tree_to_string(Tree * input_tree){
  if (input_tree->tree == NULL){
    printf("Error. Can't write tree. Given tree doesn't exist.\n");
    exit(EXIT_FAILURE);
  } else {
    long num_leaves = input_tree->num_leaves;
    int num_digits_n = get_num_digits(input_tree->num_leaves);
    /*Upper bound for the maximum length of a tree as a string.*/
    long max_str_length = 2 * num_leaves * num_leaves * num_digits_n + (1+get_num_digits(input_tree->root_time))*num_leaves;
    char *tree_str = emalloc(2 * max_str_length * sizeof(char));

    /*create matrix cluster*leaves -- 0 if leaf is not in cluster, 1 if it is in cluster*/
    long ** clusters = emalloc((num_leaves - 1) * sizeof(long *));
    for (long i = 0; i < num_leaves - 1; i++){
      clusters[i] = emalloc((num_leaves + 1) * sizeof(long));
    }

    for (long i = 0; i < num_leaves; i++){
      for (long j = 0; j < num_leaves - 1; j++){
        /*initialise all entries to be 0*/
        clusters[j][i] = 0;
      }
      long j = i;
      while (input_tree->tree[j].parent != -1){
        j = input_tree->tree[j].parent;
        /*printf("j= %ld, numleaves = %ld, i = %ld\n", j, num_leaves, i);*/
        clusters[j - num_leaves][i] = 1;
      }
      clusters[num_leaves - 2][i] = 1;
    }

    /*Assign a time to each cluster in the last column of each row.*/
    for (long k = 0; k < num_leaves - 1; k++) {
      clusters[k][num_leaves] = input_tree->tree[k+num_leaves].time;
    }
    /*convert matrix into output string tree_str*/
    sprintf(tree_str, "[");
    /*last position in tree_str that is filled with a character*/
    long tree_str_pos;
    int root_digits = 0;
    for (long i = 0; i < num_leaves - 1; i++){
      root_digits = get_num_digits(input_tree->root_time);
      char timestr[root_digits];
      sprintf(timestr, "%ld:{", clusters[i][num_leaves]);
      strcat(tree_str, timestr);
      for (long j = 0; j < num_leaves; j++){
        if (clusters[i][j] == 1){
          char leaf_str[num_digits_n + 1];
          sprintf(leaf_str, "%ld,", j + 1);
          strcat(tree_str, leaf_str);
        }
      }
      tree_str_pos = strlen(tree_str) - 1;
      /*delete last comma*/
      tree_str[tree_str_pos] = '\0';
      strcat(tree_str, "},");
      tree_str_pos +=2;
    }
    /* delete ,{ at end of tree_str*/
    tree_str[tree_str_pos] = '\0';
    tree_str[tree_str_pos - 1] = '\0';
    strcat(tree_str, "]");

    for (long i = 0; i < num_leaves-1; i++){
      free(clusters[i]);
    }
    free(clusters);

    return(tree_str);
  }
}

/* Write Tree writes one tree into a given file. The runtime is quadratic.
* @param Tree* input_tree The tree to write out.
* @param char* filename the file to write to.
*/
void write_tree(Tree * input_tree, char * filename){
  char * tree_str = tree_to_string(input_tree);
  FILE *f;
  /*add the tree to end of output file*/
  f = fopen(filename, "a");
  /*This adds a new line at the end of the file -- we need to be careful when we read from such files!*/
  fprintf(f, "%s\n", tree_str);
  fclose(f);
  free(tree_str);
}

/* Write Trees writes a Tree List of trees into a given file.
* @param Tree_List* tree_list the Tree List to write out.
* @param char* filename the file to write to.
*/
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

/* NNI move makes an NNI move on node rank_in_list of a tree if possible.
* @param Tree* input_tree The tree to make the NNI move on.
* @param long rank_in_list The node to make the NNI move on.
* @param int child_moves_up The child to move up during the NNI move.
* @return int 0 if successful, else 1.
*/
int nni_move(Tree * input_tree, long rank_in_list, int child_moves_up){
  if (input_tree->tree == NULL){
    printf("Error. No RNNI move possible. Given tree doesn't exist.\n");
  } else{
    if(input_tree->tree[input_tree->tree[rank_in_list].parent].time != input_tree->tree[rank_in_list].time + 1){
      printf("Can't do an NNI - interval [%ld, %ld] is not an edge!\n", rank_in_list, rank_in_list + 1);
      return 1;
    } else{
      int child_moved_up;
      for (int i = 0; i < 2; i++){
        /*find the child of the node of rank_in_list k+1 that is not the node of rank_in_list k*/
        if (input_tree->tree[rank_in_list+1].children[i] != rank_in_list){
          /*update parent/children relations to get nni neighbour*/
          input_tree->tree[input_tree->tree[rank_in_list+1].children[i]].parent = rank_in_list;
          input_tree->tree[input_tree->tree[rank_in_list].children[child_moves_up]].parent = rank_in_list+1;
          child_moved_up = input_tree->tree[rank_in_list].children[child_moves_up];
          input_tree->tree[rank_in_list].children[child_moves_up] = input_tree->tree[rank_in_list+1].children[i];
          input_tree->tree[rank_in_list+1].children[i] = child_moved_up;
          printf("swapping [%ld, %ld] moved up: %d\n", rank_in_list, rank_in_list+1, child_moved_up+1);
        }
      }
    }
  }
  return 0;
}

/* Rank Move makes a rank move on a tree between nodes rank and rank + 1 if
* possible.
* @param Tree* input_tree The tree to make the rank move on.
* @param long rank_in_list The lower bound of the node to make the rank move.
* @return int 0 if successful, else 1.
*/
int rank_move(Tree * input_tree, long rank_in_list){
  if (input_tree->tree == NULL){
    fprintf(stderr, "Error. No rank move possible. Given tree doesn't exist.\n");
    return 1;
  } else{
    if (input_tree->tree[rank_in_list].parent == rank_in_list + 1){
      fprintf(stderr, "Error. No rank move possible. The interval [%ld,%ld] is an edge!\n", rank_in_list, rank_in_list + 1);
    } else{
      /*update parents of nodes that swap ranks*/
      printf("swapping ranks (times) of %ld and %ld\n", rank_in_list, rank_in_list+1);
      long upper_parent;
      upper_parent = input_tree->tree[rank_in_list + 1].parent;
      input_tree->tree[rank_in_list + 1].parent = input_tree->tree[rank_in_list].parent;
      input_tree->tree[rank_in_list].parent = upper_parent;

      for (int i = 0; i < 2; i++){
        /*update children of nodes that swap ranks*/
        long upper_child = input_tree->tree[rank_in_list + 1].children[i];
        input_tree->tree[rank_in_list + 1].children[i] = input_tree->tree[rank_in_list].children[i];
        input_tree->tree[rank_in_list].children[i] = upper_child;
      }
      for (int i = 0; i < 2; i++){
        /*update parents of children of nodes that swap ranks*/
        input_tree->tree[input_tree->tree[rank_in_list + 1].children[i]].parent = rank_in_list + 1;
        input_tree->tree[input_tree->tree[rank_in_list].children[i]].parent = rank_in_list;
      }
      for (int i = 0; i < 2; i ++){
        /*update children of parents of nodes that swap rank*/
        /*first case: nodes that swap ranks share a parent. In this case nothing needs to be changed*/
        if (input_tree->tree[rank_in_list + 1].parent == input_tree->tree[rank_in_list].parent){
          break;
        }
        else{
          /*parent pointer of input_tree->tree[rank_in_list + 1] is already set correctly!*/
          if (input_tree->tree[input_tree->tree[rank_in_list + 1].parent].children[i] == rank_in_list){
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

/* Increasing Length move makes a length move on node rank of a tree if
* possible. Increases the time of the node by 1.
* @param Tree* input_tree The tree to make the length move on.
* @param long rank the node to make the length move on.
* @return int 0 if successful, else 1.
*/
int increasing_length_move(Tree * input_tree, long rank) {
  if (input_tree->tree == NULL){
    fprintf(stderr, "Error. No length move possible. Given tree doesn't exist.\n");
    return 1;
  } else if(input_tree->tree[rank].parent != -1 && input_tree->tree[input_tree->tree[rank].parent].time == input_tree->tree[rank].time + 1) {
    fprintf(stderr, "Error. No length move possible. length between parent is not > 1.\n");
    return 1;
  }
  printf("increasing %ld to time %ld\n", rank, input_tree->tree[rank].time+1);
  input_tree->tree[rank].time++;
  return 0;
}

/* Decreasing Length move makes a length move on node rank of a tree if
* possible. Decreases the time of the node by 1.
* @param Tree* input_tree The tree to make the length move on.
* @param long rank the node to make the length move on.
* @return int 0 if successful, else 1.
*/
int decreasing_length_move(Tree * input_tree, long rank) {
  if (input_tree->tree == NULL){
    fprintf(stderr, "Error. No length move possible. Given tree doesn't exist.\n");
    return 1;
  } else if(input_tree->tree[input_tree->tree[rank].children[0]].time == input_tree->tree[rank].time - 1 || input_tree->tree[input_tree->tree[rank].children[1]].time == input_tree->tree[rank].time - 1) {
    fprintf(stderr, "Error. No length move possible. length between a child is not > 1.\n");
    return 1;
  }
  printf("decreasing %ld to time %ld\n", rank, input_tree->tree[rank].time-1);
  input_tree->tree[rank].time--;
  return 0;
}

/* Print Tree prints out each nodes time, rank, parent and children.
* @param Tree* input_tree The tree to print out.
*/
void print_tree_info(Tree *input_tree) {
  for(int i = 0; i < input_tree->num_leaves*2 -1; i++) {
    printf("Node: %d Rank: %d Time: %ld\n", i, i, input_tree->tree[i].time);
    printf("Node: %d's parent: %ld\n", i, input_tree->tree[i].parent);
    printf("Node: %d's children: L: %ld, R:%ld\n", i, input_tree->tree[i].children[0], input_tree->tree[i].children[1]);
  }
}

/* mrca finds the most recent common ancestor of nodes with positions node1
* and node2 in tree.
* @param Tree* input_tree The tree to search.
* @param long node1 node 1.
* @param long node2 node 2.
* @return long the mrca node of node 1 and node 2.
*/
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

/* Find Path computes the shortest path between two DCT trees and returns the
*path in a matrix representation. Please read data_structures.md for more
* information on the matrix format.
* @param Tree *start_tree The start DCT tree (T).
* @param Tree *dest_tree The destination DCT tree (R).
* @return Path the shortest path between the two DCT trees.
*/
Path findpath(Tree *start_tree, Tree *dest_tree){

  /*counter to print the progress of the algorithm (in 10% steps of max distance)*/
  float count = 0.05;
  long num_leaves = start_tree->num_leaves;
  long root_time = start_tree->root_time;
  long max_root_time = start_tree->root_time > dest_tree->root_time ? start_tree->root_time : dest_tree->root_time;
  long max_dist = ((num_leaves - 1) * (num_leaves - 2))/2 + (max_root_time + 1 - num_leaves + 1) * (num_leaves - 1);
  Path path;
  path.moves = emalloc((max_dist + 1) * sizeof(long*));
  path.length = 0;
  for (long i = 0; i < max_dist + 1; i++){
    path.moves[i] = emalloc(2 * sizeof(long));
    path.moves[i][0] = 0;
    path.moves[i][1] = 0;
  }
  long path_index = 0;
  if (start_tree->tree == NULL){
    printf("Error. Start tree doesn't exist.\n");
  } else if (dest_tree->tree == NULL){
    printf("Error. Destination tree doesn't exist.\n");
  } else{
    remove("./output/findpath.rtree");
    /*This ruins the running time!!!!*/
    /*write_tree(start_tree->tree, num_leaves, "./output/findpath.rtree");*/
    long current_mrca;
    long current_mrca_time;
    Tree current_tree;
    current_tree.tree = emalloc((num_leaves*2 - 1) * sizeof(Node));
    current_tree.num_leaves = num_leaves;
    current_tree.root_time = root_time;
    for (long i = 0; i < num_leaves*2 - 1; i++){
      current_tree.tree[i] = start_tree->tree[i];
    }
    Tree * current_tree_pointer;
    current_tree_pointer = &current_tree;
    long current_node = num_leaves;

    for(long i = num_leaves; i < num_leaves*2 -1; i++) {
      printf("Time of current_tree[%ld]: %ld\n", i, current_tree.tree[i].time);
      printf("Time of dest_tree[%ld]: %ld\n", i, dest_tree->tree[i].time);
    }

    for (long i = num_leaves; i < dest_tree->root_time + num_leaves; i++){

      if(dest_tree->tree[current_node].time == i - num_leaves + 1) {
        current_mrca = mrca(current_tree_pointer, dest_tree->tree[current_node].children[0], dest_tree->tree[current_node].children[1]);
        current_mrca_time = current_tree.tree[current_mrca].time;
        printf("mrca: %ld, time: %ld\n", current_mrca, current_mrca_time);
        printf("i: %ld, current node: %ld\n", i - num_leaves + 1, current_node);
        while(current_mrca > current_node || current_mrca_time != i - num_leaves + 1){
          bool did_nni = false;
          bool did_rnkmv = false;
          /*find out if one of the children of current_tree.tree[current_mrca] has rank current_mrca - 1. If this is the case, we want to make an NNI */
          for (int child_index = 0; child_index < 2; child_index++){
            /*do nni if current interval is an edge */
            if (did_nni == false && current_tree.tree[current_mrca].children[child_index] == current_mrca - 1 && current_tree.tree[current_tree.tree[current_mrca].children[child_index]].time + 1 == current_tree.tree[current_mrca].time){
              bool found_child = false;
              /*index of the child of current_tree.tree[current_mrca] that does not move up by an NNI move*/
              int child_stays;
              /*find the index of the child of the parent of the node we currently consider -- this will be the index child_stays that we want in the end*/
              /*rank of already existing cluster in both current_tree.tree and dest_tree->tree*/
              int current_child_index = dest_tree->tree[current_node].children[0];
              while (found_child == false){
                /*find the x for which dest_tree->tree[i].children[x] is contained in the cluster induced by current_tree.tree[current_mrca - 1]*/
                while (current_tree.tree[current_child_index].parent < current_mrca - 1){
                  current_child_index = current_tree.tree[current_child_index].parent;
                }
                /*find the index child_stays*/
                if(current_tree.tree[current_child_index].parent == current_mrca - 1){
                  found_child = true;
                  if (current_tree.tree[current_tree.tree[current_child_index].parent].children[0] == current_child_index){
                    child_stays = 0;
                  } else{
                    child_stays = 1;
                  }
                } else{
                  current_child_index = dest_tree->tree[current_node].children[1];
                }
              }/*end of while child not found*/
              nni_move(current_tree_pointer, current_mrca - 1, 1 - child_stays);
              path.moves[path_index][1] = 1 + child_stays;
              current_mrca--;
              did_nni = true;
            }
          }
          if (did_nni == false && current_tree.tree[current_mrca].time == current_tree.tree[current_mrca-1].time + 1){
            rank_move(current_tree_pointer, current_mrca - 1);
            path.moves[path_index][1] = 0;
            current_mrca--;
            did_rnkmv = true;
          }
          if (did_nni == false && did_rnkmv == false && current_tree.tree[current_mrca].time > dest_tree->tree[current_mrca].time) {
            /*do decreasing length moves from mrca*/
            long l = current_mrca;
            for(long p = l; p > i; p--) {
              if(current_tree.tree[p-1].time < current_tree.tree[p].time - 1) {
                l = p;
              }
            }  /*we decrease the node that was larger than expected.*/
            long j = l;
            while(j >= current_node && current_tree.tree[j].time > dest_tree->tree[j].time) {
              decreasing_length_move(current_tree_pointer, j);
              path.moves[path_index][1] = 4;
              path.moves[path_index][0] = j;
              j--;
              path_index++;
            }
          }
          if (did_nni || did_rnkmv) {
            path.moves[path_index][0] = current_mrca;
            path_index++;
          }
          current_mrca_time = current_tree.tree[current_mrca].time;
        } /*end of while loop*/
        /*If we reach the root time of the dest_tree but the current_tree root time
        * is too large, we must decrease it.*/
        if(current_node < num_leaves*2 - 2 && mrca(current_tree_pointer, dest_tree->tree[current_node].children[0], dest_tree->tree[current_node].children[1]) == current_node && current_mrca_time == i - num_leaves + 1) {
          current_node++;
        }
      } else if(current_tree.tree[current_node].time == i - num_leaves + 1){
        /*Do increasing length moves*/
        long l = num_leaves * 2 - 2;
        long p = l;

        while(current_tree.tree[p].time >= i - num_leaves + 1) {
          /*&& skips the root node, as we could increase it, but we don't need to compare it to anything.*/
          if(p < num_leaves * 2 - 2 && current_tree.tree[p].time + 1 < current_tree.tree[p+1].time) {
            l = p;
          }
          p--;
        }
        /*If the root node will be increased but the max root time has already been reached, stop.*/
        if (l == num_leaves * 2 - 1) {
          fprintf(stderr, "No gaps to increase to.\n");
        } else if(l == num_leaves * 2 - 2 && current_tree.tree[num_leaves * 2 - 2].time >= dest_tree->root_time){
          fprintf(stderr, "Root is already at dest height.\n");
        } else {
          long j = l;
          while(current_tree.tree[j].time >= i - num_leaves + 1 && current_tree.tree[j].time < dest_tree->tree[j].time) {
            increasing_length_move(current_tree_pointer, j);
            path.moves[path_index][1] = 3;
            path.moves[path_index][0] = j;
            path_index++;
            j--;
          }
        }
      }
      /*If the current node is the same time in both trees, we can move on.*/
      if(current_node < num_leaves*2 - 2 && current_tree.tree[current_node].time == dest_tree->tree[current_node].time && mrca(current_tree_pointer, dest_tree->tree[current_node].children[0], dest_tree->tree[current_node].children[1]) == current_node) {
        current_node++;
      }
      if (count < (float) path_index / (float) max_dist){
        printf("%d Percent of maximum distance reached\n", (int) (100 * count));
        count += 0.05;
      }
    } /*end of first for loop.*/
    free(current_tree.tree);
  }
  path.length = path_index;
  return path;
} /*end of findpath function.*/

/* Find Path distance computes the shortest path between two DCT trees and
* returns the distance of the path. This function does not create a matrix.
* @param Tree *start_tree The start DCT tree (T).
* @param Tree *dest_tree The destination DCT tree (R).
* @return long the distance of the shortest path between the two DCT trees.
*/
long findpath_distance(Tree *start_tree, Tree *dest_tree){
  long num_leaves = start_tree->num_leaves;
  long root_time = start_tree->root_time;
  long path_index = 0;
  if (start_tree->tree == NULL){
    printf("Error. Start tree doesn't exist.\n");
  } else if (dest_tree->tree == NULL){
    printf("Error. Destination tree doesn't exist.\n");
  } else{
    remove("./output/findpath.rtree");
    /*This ruins the running time!!!!*/
    /* write_tree(start_tree->tree, num_leaves, "./output/findpath.rtree"); */
    long current_mrca;
    long current_mrca_time;
    Tree current_tree;
    current_tree.tree = emalloc((num_leaves*2 - 1) * sizeof(Node));
    current_tree.num_leaves = num_leaves;
    current_tree.root_time = root_time;
    for (long i = 0; i < num_leaves*2 - 1; i++){
      current_tree.tree[i] = start_tree->tree[i];
    }
    Tree * current_tree_pointer;
    current_tree_pointer = &current_tree;
    long current_node = num_leaves;
    for (long i = num_leaves; i < dest_tree->root_time + num_leaves; i++){

      if(dest_tree->tree[current_node].time == i - num_leaves + 1) {
        current_mrca = mrca(current_tree_pointer, dest_tree->tree[current_node].children[0], dest_tree->tree[current_node].children[1]);
        current_mrca_time = current_tree.tree[current_mrca].time;
        printf("mrca: %ld, time: %ld\n", current_mrca, current_mrca_time);
        printf("i: %ld, current node: %ld\n", i - num_leaves + 1, current_node);
        while(current_mrca > current_node || current_mrca_time != i - num_leaves + 1){ /*#3*/
          bool did_nni = false;
          bool did_rnkmv = false;
          /*find out if one of the children of current_tree.tree[current_mrca] has rank current_mrca - 1. If this is the case, we want to make an NNI */
          for (int child_index = 0; child_index < 2; child_index++){
            /*do nni if current interval is an edge*/
            if (did_nni == false && current_tree.tree[current_mrca].children[child_index] == current_mrca - 1 && current_tree.tree[current_tree.tree[current_mrca].children[child_index]].time + 1 == current_tree.tree[current_mrca].time){
              bool found_child = false;
              /*index of the child of current_tree.tree[current_mrca] that does not move up by an NNI move*/
              int child_stays;
              /*find the index of the child of the parent of the node we currently consider -- this will be the index child_stays that we want in the end*/
              /*rank of already existing cluster in both current_tree.tree and dest_tree->tree*/
              int current_child_index = dest_tree->tree[current_node].children[0];
              while (found_child == false){
                /*/find the x for which dest_tree->tree[i].children[x] is contained in the cluster induced by current_tree.tree[current_mrca - 1]*/
                while (current_tree.tree[current_child_index].parent < current_mrca - 1){
                  current_child_index = current_tree.tree[current_child_index].parent;
                }
                /*find the index child_stays*/
                if(current_tree.tree[current_child_index].parent == current_mrca - 1){
                  found_child = true;
                  if (current_tree.tree[current_tree.tree[current_child_index].parent].children[0] == current_child_index){
                    child_stays = 0;
                  } else{
                    child_stays = 1;
                  }
                } else{
                  current_child_index = dest_tree->tree[current_node].children[1];
                }
              }/*end of while child not found*/
              nni_move(current_tree_pointer, current_mrca - 1, 1 - child_stays);
              current_mrca--;
              did_nni = true;
            }
          }
          if (did_nni == false && current_tree.tree[current_mrca].time == current_tree.tree[current_mrca-1].time + 1){
            rank_move(current_tree_pointer, current_mrca - 1);
            current_mrca--;
            did_rnkmv = true;
          }
          if (did_nni == false && did_rnkmv == false && current_tree.tree[current_mrca].time > dest_tree->tree[current_mrca].time) {
            /*do decreasing length moves from mrca*/
            long l = current_mrca;
            for(long p = l; p > i; p--) {
              if(current_tree.tree[p-1].time < current_tree.tree[p].time - 1) {
                l = p;
              }
            }  /*we decrease the node that was larger than expected.*/
            long j = l;
            while(j >= current_node && current_tree.tree[j].time > dest_tree->tree[j].time) {
              decreasing_length_move(current_tree_pointer, j);
              j--;
              path_index++;
            }
          }
          if (did_nni || did_rnkmv) {
            path_index++;
          }
          current_mrca_time = current_tree.tree[current_mrca].time;
        } /*end of while loop*/
        /*If we reach the root time of the dest_tree but the current_tree root time
        * is too large, we must decrease it.*/
        if(current_node < num_leaves*2 - 2 && mrca(current_tree_pointer, dest_tree->tree[current_node].children[0], dest_tree->tree[current_node].children[1]) == current_node && current_mrca_time == i - num_leaves + 1) {
          current_node++;
        }
      } else if(current_tree.tree[current_node].time == i - num_leaves + 1){
        /*do increasing length moves*/
        long l = num_leaves * 2 - 2;
        long p = l;

        while(current_tree.tree[p].time >= i - num_leaves + 1) {
          if(current_tree.tree[p].time + 1 < current_tree.tree[p+1].time) {
            l = p;
          }
          p--;
        }
        /*If the root node will be increased but the max root time has already been reached, stop.*/
        if (l == num_leaves * 2 - 1) {
          fprintf(stderr, "No gaps to increase to.\n");
        } else if(l == num_leaves * 2 - 2 && current_tree.tree[num_leaves * 2 - 2].time >= dest_tree->root_time){
          fprintf(stderr, "Root is already at dest height.\n");
        } else {
          long j = l;
          while(current_tree.tree[j].time >= i - num_leaves + 1 && current_tree.tree[j].time < dest_tree->tree[j].time) {
            increasing_length_move(current_tree_pointer, j);
            path_index++;
            j--;
          }
        }
      }
      /*If the current node is the same time in both trees, we can move on.*/
      if(current_node < num_leaves*2 - 2 && current_tree.tree[current_node].time == dest_tree->tree[current_node].time && mrca(current_tree_pointer, dest_tree->tree[current_node].children[0], dest_tree->tree[current_node].children[1]) == current_node) {
        current_node++;
      }
    } /*end of first for loop.*/
    free(current_tree.tree);
  }
  return path_index;/*#10*/
} /*end of findpath distance function.*/

/* Return Find Path computes the shortest path between the first two trees in a
* tree list and returns the path as a new tree list, with each tree in the new
* tree list being one move of the path.
* @param Tree_List tree_list the tree list to read the two trees from.
* @return Tree_List the moves of the path.
*/
Tree_List return_findpath(Tree_List tree_list){
  long max_root_time = tree_list.trees[0].root_time > tree_list.trees[1].root_time ? tree_list.trees[0].root_time : tree_list.trees[1].root_time;
  long path_index = 0;
  long num_leaves = tree_list.trees[0].num_leaves;
  long root_time = tree_list.trees[0].root_time;
  Tree current_tree;
  current_tree.num_leaves = num_leaves;
  current_tree.root_time = root_time;
  /*deep copy start tree*/
  current_tree.tree = emalloc((2 * num_leaves - 1) * sizeof(Node));
  for (int i = 0; i < 2 * num_leaves - 1; i++){
    current_tree.tree[i] = tree_list.trees[0].tree[i];
  }

  /*write_tree(tree_list.trees[1], num_leaves, "output/tree.rtree");*/
  /*write_tree(tree_list.trees[0], num_leaves, "output/tree.rtree");*/

  /*Compute the path to get the moves in the path matrix representation*/
  Tree * start_tree;
  Tree * dest_tree;
  start_tree = &tree_list.trees[0];
  dest_tree = &tree_list.trees[1];

  Path fp = findpath(start_tree, dest_tree);
  /*this is not the diameter, but the number of trees on a path giving the diameter (= diameter + 1)*/
  long diameter = ((num_leaves - 1) * (num_leaves - 2) / 2 ) + ((max_root_time + 1 - num_leaves + 1)*(num_leaves - 1));

  /*output: list of trees on FP path*/
  Tree_List findpath_list;
  findpath_list.num_trees = fp.length;
  findpath_list.trees = emalloc(diameter * sizeof(Tree));
  for (long i = 0; i < diameter; i++){
    findpath_list.trees[i].num_leaves = num_leaves;
    findpath_list.trees[i].tree = emalloc((2* num_leaves - 1) * sizeof(Node));
  }
  /*findpath_list.trees[0] = current_tree.tree;*/
  for (long i = 0; i < 2 * num_leaves - 1; i++){
    findpath_list.trees[0].tree[i] = current_tree.tree[i];
    findpath_list.trees[0].root_time = current_tree.root_time;
  }

  /*create actual path by doing moves starting at tree_list.trees[0] with the information in the matrix returned form fp above*/
  while(path_index < diameter -1 && fp.moves[path_index][0] > 0){
    if (fp.moves[path_index][1] == 0){
      rank_move(&current_tree, fp.moves[path_index][0]);
    }
    else if (fp.moves[path_index][1] == 1){
      nni_move(&current_tree, fp.moves[path_index][0], 1);
    } else if (fp.moves[path_index][1] == 2){
      nni_move(&current_tree, fp.moves[path_index][0], 0);
    } else if (fp.moves[path_index][1] == 3){
      increasing_length_move(&current_tree, fp.moves[path_index][0]);
    } else {
      decreasing_length_move(&current_tree, fp.moves[path_index][0]);
    }
    path_index++;
    /*deep copy currently last tree one path. (resulting tree)*/
    findpath_list.trees[path_index].root_time = current_tree.root_time;
    for (long i = 0; i < 2 * num_leaves - 1; i++){
      findpath_list.trees[path_index].tree[i] = current_tree.tree[i];
    }
  }
  for (int i = 0; i < diameter+1; i++){
    free(fp.moves[i]);
  }
  free(fp.moves);
  free(current_tree.tree);
  return findpath_list;
}

/* Find Path cluster list computes the shortest path between two trees inputted
* as strings, and returns the path as an array of strings representing the tree
* after each move in the path. The first row of the array only contains the
* distance of the path.
* @param int num_leaves The number of leaves in both trees.
* @param char* input_tree1 The first tree.
* @param char* input_tree2 The second tree.
* @return char** The path as an array of strings.
*/
char** findpath_cluster_list(int num_leaves, char* input_tree1, char* input_tree2){

  Tree tree1 = read_tree_from_string(num_leaves, input_tree1);
  Tree tree2 = read_tree_from_string(num_leaves, input_tree2);
  Tree_List findpath_input;
  findpath_input.num_trees = 2;
  findpath_input.trees = emalloc(2 * sizeof(Tree));
  findpath_input.trees[0] = tree1;
  findpath_input.trees[0].root_time = tree1.root_time;
  findpath_input.trees[1] = tree2;
  findpath_input.trees[1].root_time = tree2.root_time;

  long max_root_time = tree1.root_time > tree2.root_time ? tree1.root_time : tree2.root_time;
  long max_dist = ((num_leaves - 1) * (num_leaves - 2))/2 + (max_root_time + 1 - num_leaves + 1) * (num_leaves - 1);
  Tree_List path = return_findpath(findpath_input);

  char** output = emalloc((max_dist + 1) * sizeof(char* ));
  output[0] = emalloc(sizeof(char*));
  /*first entry in output is number of trees on path*/
  sprintf(output[0], "%d", path.num_trees);
  for (int k = 0; k < path.num_trees+1; k++){
    /* output[k] = emalloc((num_leaves-1) * (num_leaves-1) * sizeof(char));*/
    output[k+1] = tree_to_string(&path.trees[k]);
  }
  for (int i = 0; i < max_dist; i++){
    free(path.trees[i].tree);
  }
  free(findpath_input.trees);
  free(path.trees);
  free(tree1.tree);
  free(tree2.tree);

  return(output);

}

/* Free Tree List is a helper function that frees a given Tree List.
* @param Tree_List *tl A Tree List to free.
*/
void free_tree_list(Tree_List * tl) {
  if(tl != NULL) {
    int i;
    for (i = 0; i < tl->num_trees; i++){
      free(tl->trees[i].tree);
    }
    free(tl->trees);
  }
}

/* Main the starting point of the program. Usage: main 'filename'.
* Reads in a Tree List file given as an argument, runs findpath, prints the
* distance, then runs return findpath and prints each tree after each move.
*/
int main(int argc, char const *argv[]) {
  int i;
  char filename[200]; /* length of filename set to be 200 char max */
  /*printf("What is the file containing trees?\n");
  scanf("%s", filename);*/
  /*Read in tree-list file from arguments.*/
  if(argc != 2) {
    printf("Usage: main 'filename'\n");
    exit(EXIT_FAILURE);
  }
  char * start_tree_str;
  char * dest_tree_str;
  strcpy(filename, argv[1]);
  printf("Start reading trees from file\n");
  Tree_List tree_list = read_trees_from_file(filename);
  printf("End reading trees from file\n");


  /*Print out both Trees.*/
  start_tree_str = tree_to_string(&tree_list.trees[0]);
  dest_tree_str = tree_to_string(&tree_list.trees[1]);
  printf("Tree 1:\n");
  printf("%s\n", start_tree_str);
  printf("\n");
  printf("Tree 2:\n");
  printf("%s\n", dest_tree_str);
  /*Tree my_tree = read_tree_from_string(4, "[1:{1,2},1:{3,4},6:{1,2,3,4}]");*/
  /*printf("End reading trees from string.\n");*/
  Tree * start_tree;
  Tree * dest_tree;
  start_tree = &tree_list.trees[0];
  dest_tree = &tree_list.trees[1];
  printf("Start running FindPath\n");
  clock_t start_time = time(NULL);
  long distance;
  clock_t end_time = time(NULL);
  printf("End running FindPath\n");
  long max_root_time = start_tree->root_time > dest_tree->root_time ? start_tree->root_time : dest_tree->root_time;
  long max_dist = ((start_tree->num_leaves - 1) * (start_tree->num_leaves - 2))/2 + (max_root_time + 1 - start_tree->num_leaves + 1) * (start_tree->num_leaves - 1);

  /*Create cluster list of the path.*/
  char** my_cluster_list = findpath_cluster_list(start_tree->num_leaves, start_tree_str, dest_tree_str);
  distance = atol(my_cluster_list[0]);
  /*Print the distance between the two trees.*/
  printf("FP(T,R) = %ld\n", distance);
  /*Print the path between the two trees.*/
  for(int i = 0; i < distance + 1; i++) {
    printf("%d: %s\n", i, my_cluster_list[i+1]);
  }
  /*Print the dest tree for comparison.*/
  printf("R: %s\n", dest_tree_str);

  /*Frees a path.*/
  /*for (i = 0; i < max_dist + 1; i++){
    free(path.moves[i]);
  }
  free(path.moves);*/

  printf("Time to compute FP(T,R): %f sec\n", difftime(end_time, start_time));
  for(int i = 0; i < distance+2; i++) {
    free(my_cluster_list[i]);
  }
  free(my_cluster_list);
  free_tree_list(&tree_list);
  free(start_tree_str);
  free(dest_tree_str);

  return 0;
}
