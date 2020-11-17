#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

/* A tree is an array of nodes ordered according to their ranks (first n nodes
 are leaves, order there doesn't matter) + number of leaves (long)*/
 typedef struct Node{
     long parent;
     long children[2];
 } Node;
 /* A tree is an array of nodes ordered according to their ranks
 (first n nodes are leaves, order there doesn't matter) + number of leaves (long)*/
 typedef struct Tree{
     long num_leaves;
     long max_height;
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

 /*read one tree from a string*/
 Tree read_tree_from_string(int num_leaves, char* tree_string){
     // Deep copy tree string
     int str_length = strlen(tree_string);
     char * tree_str = malloc((str_length + 1) * sizeof(char));
     strcpy(tree_str, tree_string);
     // Remove white spaces from string buffer
     int l = 0;
     int max_height;
     for(int k=0;tree_str[k]!=']';++k)
     {
         if(tree_str[k]!=' ') {
           tree_str[l++]=tree_str[k];
         }
        if(tree_str[k] == ':') {
          /* go through the whole string to fix the largest node height. */
          max_height = atoi(&tree_str[k-1]);
        }
     }
     fprintf(stderr, "The height of the root is: %d\n", max_height);
     tree_str[l] = ']';
     tree_str[++l] = '\0';

     int num_nodes = num_leaves + max_height - 1;
     int num_digits_n = get_num_digits(num_leaves); // number of digits of the int num_leaves

     Tree output_tree;
     output_tree.tree = malloc(num_nodes * sizeof(Node));
     for (int i = 0; i < num_nodes; i ++){
         output_tree.tree[i].parent = -1;
         output_tree.tree[i].children[0] = -1;
         output_tree.tree[i].children[1] = -1;
     }
     output_tree.num_leaves = num_leaves;
     output_tree.max_height = max_height;

     int *highest_ancestor = malloc(num_leaves * sizeof(int)); // highest_ancestor[i]: index of cluster containing leaf i that is highest below the currently considered cluster
     for(int i = 0; i < num_leaves; i++){
         highest_ancestor[i] = 1;
     }
     int current_tree = 0; //This will stay 0 as we only read one tree

     // allocate memory for strings saving clusters
     char *cluster;
     /*remove rank*/
     int rank = num_leaves;
     int cluster_number = 0; // index of cluster we currently consider -- max is num_leaves - 1
     //Find clusters
     char * partial_tree_str = tree_str;
     fprintf(stderr, "%s\n", partial_tree_str);
     char * cluster_list;

     while((cluster_list = strsep(&partial_tree_str, "}")) != NULL){
          fprintf(stderr, "%s\n", cluster_list);
         cluster_list += 1; // ignore first 1 characters [{ or ,{
           /*learn the rank of the current cluster, which is at the front of each cluster.*/
           int placeholder_int = atoi(cluster_list);
           cluster_list += 3;
         if(cluster_number < num_leaves){ //only consider clusters, not things that could potentially be in rest of tree string after the actual tree
             // Find leaves in clusters
             while((cluster = strsep(&cluster_list, ",")) != NULL){
                 int actual_leaf = atoi(cluster);
                 // update node relations if current leaf appears for first time
                 /*if the leaf appears for the first time */
                 if(output_tree.tree[actual_leaf - 1].parent == -1){
                     fprintf(stderr, "Inserting %d on rank %d\n", actual_leaf, placeholder_int);
                     output_tree.tree[actual_leaf - 1].parent = placeholder_int;
                     // update the current internal node (rank) to have the current leaf as child
                     if(output_tree.tree[placeholder_int].children[0] == -1){
                         output_tree.tree[placeholder_int].children[0] = actual_leaf - 1;
                     } else{
                         output_tree.tree[placeholder_int].children[1] = actual_leaf - 1;
                     }
                     /*if the leaf is not a new leaf*/
                 } else {
                     output_tree.tree[highest_ancestor[actual_leaf - 1]].parent = placeholder_int;
                    // fprintf(stderr, "Assigned parent %d to %d\n", placeholder_int, highest_ancestor[actual_leaf -1]);
                     // update cluster relation if actual_leaf already has parent assigned (current cluster is union of two clusters or cluster and leaf)
                     if(output_tree.tree[placeholder_int].children[0] == -1 || output_tree.tree[placeholder_int].children[0] == highest_ancestor[actual_leaf - 1]){ // first children should not be assigned yet. I if contains same value, overwrite that one
                         output_tree.tree[placeholder_int].children[0] = highest_ancestor[actual_leaf - 1];
                     } else if (output_tree.tree[placeholder_int].children[1] == -1)
                     {
                         output_tree.tree[placeholder_int].children[1] = highest_ancestor[actual_leaf - 1];
                     }
                 }
                 highest_ancestor[actual_leaf - 1] = placeholder_int;
            //     fprintf(stderr, "Highest ancestor of %d is %d\n", actual_leaf, highest_ancestor[actual_leaf - 1]);
             }
         }
         cluster_number++;
         /*rank++;*/
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

/*read trees from file reads a dct tree from a file.*/
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
         tree_list.trees = malloc(num_trees * sizeof(Tree));
         tree_list.num_trees = num_trees;
         while(fgets(buffer, max_str_length * sizeof(char), f) != NULL){
           tree_list.trees[tree_index++] = read_tree_from_string(num_leaves, buffer);
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

 /*Tree to string does not work.*/
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

 int nni_move(Tree * input_tree, long rank_in_list, int child_moves_up){
     if (input_tree->tree == NULL){
         printf("Error. No RNNI move possible. Given tree doesn't exist.\n");
     } else{
         long num_leaves = input_tree->num_leaves;
         long num_nodes = num_leaves + input_tree->max_height;
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
     long num_nodes = num_leaves + input_tree->max_height;
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

/* Increases the time of the node in a tree by one.*/
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


 /* find mrca of nodes with positions node1 and node2 in tree. This will break
 for DCT because if the node doesn't exist in input_tree then you won't find
 the parent. I need a new way of finding the mrca.*/
 long mrca(Tree * input_tree, long node1, long node2){
     long rank1 = node1;
     long rank2 = node2;
     while (rank1 != rank2){
         if (rank1 < rank2){
             rank1 = input_tree->tree[rank1].parent;
             fprintf(stderr, "The rank of %ld is %ld\n", node1, rank1);/*line that could break.*/
         } else{
             rank2 = input_tree->tree[rank2].parent;
             fprintf(stderr, "The rank of %ld is %ld\n", node2, rank2);
         }
     }
     return rank1;
 }

 // returns a path in matrix representation -- explanation in data_structures.md
 Path findpath(Tree *start_tree, Tree *dest_tree){
     float count = 0.05; // counter to print the progress of the algorithm (in 10% steps of max distance)
     long num_leaves = start_tree->num_leaves;
     long max_height = start_tree->max_height;
     long max_dist = ((num_leaves - 1) * (num_leaves - 2))/2 + 1;//is this right?
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
         current_tree.tree = malloc((num_leaves + max_height - 1) * sizeof(Node));
         current_tree.num_leaves = num_leaves;
         current_tree.max_height = max_height;//or could the other tree be larger?
         for (long i = 0; i < max_height + num_leaves - 1; i++){
             current_tree.tree[i] = start_tree->tree[i];
         }
         Tree * current_tree_pointer;
         current_tree_pointer = &current_tree;
         long c_count, d_count;
         c_count = num_leaves;
         d_count = num_leaves;
         while(c_count < max_height+num_leaves-1 && d_count < max_height+num_leaves-) {
           long i = d_count;
      //   for (long i = num_leaves; i < max_height + num_leaves; i++){/*#2 changed to maxheight + #leaves*/
           //either the root or a blank node
          // fprintf(stderr, "The node %ld has a parent %ld\n", i, dest_tree->tree[i].parent);

           if(dest_tree->tree[i].parent != -1 && dest_tree->tree[i].children[0] != -1 && dest_tree->tree[i].children[1] != -1) {

             current_mrca = mrca(current_tree_pointer, dest_tree->tree[i].children[0], dest_tree->tree[i].children[1]);
             fprintf(stderr, "Current mcra is: %ld\n", current_mrca);
             // move current_mrca down
             while(current_mrca > d_count){ /*#3*/
               fprintf(stderr, "Current mcra is: %ld\n", current_mrca);
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
                         fprintf(stderr, "Performing nni move.\n");
                         nni_move(current_tree_pointer, current_mrca - 1, 1 - child_stays);
                         c_count++;
                         path.moves[path_index][1] = 1 + child_stays;
                         did_nni = true;
                         current_mrca--;
                     }
                 }
                 if (did_nni == false){ /*#6*/
                   /*start here */
                   for(int l = num_leaves+max_height-1; l > i; l--){
                     if(current_tree.tree[l].parent == -1) {
                       i = l;
                     }
                   }
                   for(int j = i-1; j < d_count; j++) {
                     fprintf(stderr, "Performing length move.\n");
                     length_move(current_tree_pointer, j);
                     //guess of where to put this function.
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
             } //end of while loop
           }//end of if statement.
             d_count++;
           fprintf(stderr, "%ld\n", d_count);
         } //end of first while loop.
         free(current_tree.tree);
     } //end of else
     path.length = path_index;
     return path;/*#10*/
 } //end of function.



 //testing main class.
int main(){
  int i;
  char filename[200]; /* length of filename set to be 200 char max */
  printf("What is the file containing trees?\n");
  scanf("%s", filename);

  printf("Start reading trees from file\n");
  Tree_List tree_list = read_trees_from_file(filename);
  printf("End reading trees from file\n");
  //Tree my_tree = read_tree_from_string(4, "[1:{1,2},1:{3,4},6:{1,2,3,4}]");
  //printf("End reading trees from string.\n");
      Tree * start_tree;
      Tree * dest_tree;
      start_tree = &tree_list.trees[0];
      dest_tree = &tree_list.trees[1];

      printf("Start running FindPath\n");
      clock_t start_time = time(NULL);
      Path path = findpath(start_tree, dest_tree);
      clock_t end_time = time(NULL);
      printf("End running FindPath\n");
      long max_dist = ((start_tree->num_leaves - 1) * (start_tree->num_leaves - 2))/2 + 1;

      for (i = 0; i < max_dist + 1; i++){
          free(path.moves[i]);
      }
      free(path.moves);

      printf("Time to compute FP(T,R): %f sec\n", difftime(end_time, start_time));
  for (i = 0; i < tree_list.num_trees; i++){
          free(tree_list.trees[i].tree);
      }
      free(tree_list.trees);
      //free(my_tree.tree);
}
//
// int main(){
//     int i;
//     char filename[200]; /* length of filename set to be 200 char max */
//     printf("What is the file containing trees?\n");
//     scanf("%s", filename);
//
//     printf("Start reading trees from file\n");
//     Tree_List tree_list = read_trees_from_file(filename);
//     printf("End reading trees from file\n");
//
//     /*// // // check if read_trees_from_file reads trees correctly
//     // // for (int k = 0; k < num_trees; k++){
//     // //     for(int i = 0; i < 2 * num_leaves - 1; i++){
//     // //         if (i < num_leaves){
//     // //             // printf("highest ancestor of node %d has rank %d\n", i, highest_ancestor[i] + 1);
//     // //             printf("leaf %d has parent %d\n", i+1, tree_list.trees[k][i].parent);
//     // //         } else{
//     // //             printf("node %d has children %d and %d\n", i, tree_list.trees[k][i].children[0], tree_list.trees[k][i].children[1]);
//     // //             printf("node %d has parent %d\n", i, tree_list.trees[k][i].parent);
//     // //         }
//     // //     }
//     // // }
//
//     // // read_tree_from_string(5, "[{1,2},{1,2,3},{1,2,3,4},{1,2,3,4,5}]"); // test function when for reading tree as string
//
//     // write_trees(tree_list, "./output/output.rtree"); // write given trees into file
//     // Tree_List findpath_list = return_findpath(tree_list); // write FP into file
//     // int ** fp = findpath(tree_list.trees[0], tree_list.trees[1], tree_list.num_leaves); //run FP */
//
//     Tree * start_tree;
//     Tree * dest_tree;
//     start_tree = &tree_list.trees[0];
//     dest_tree = &tree_list.trees[1];
//
//     printf("Start running FindPath\n");
//     clock_t start_time = time(NULL);
//     Path path = findpath(start_tree, dest_tree);
//     long distance = path.length;
//     // long distance = findpath_distance(start_tree, dest_tree);
//     clock_t end_time = time(NULL);
//     printf("End running FindPath\n");
//     long max_dist = ((num_leaves - 1) * (num_leaves - 2))/2 + 1;
//     // int distance = fp[0][0];
//     // for (int i = 0; i < max_dist + 1; i++){
//     //     free(fp[i]);
//     // }
//     // free(fp);
//
//     char ** fpath;
//     char * start_tree_str = tree_to_string(start_tree);
//     char * dest_tree_str = tree_to_string(dest_tree);
//     // fpath = findpath_cluster_list(num_leaves, start_tree_str, dest_tree_str);
//     // printf("FP path:\n");
//     // for (int i = 0; i < distance + 1; i++){
//     //     printf("%s\n", fpath[i]);
//     //     free(fpath[i]);
//     // }
//     // free(fpath);
//
//     // Free all variables
//     for (i = 0; i < max_dist + 1; i++){
//         free(path.moves[i]);
//     }
//     free(path.moves);
//
//     for (i = 0; i < tree_list.num_trees; i++){
//         free(tree_list.trees[i].tree);
//     }
//     free(tree_list.trees);
//     free(start_tree_str);
//     free(dest_tree_str);
//
//     printf("Time to compute FP(T,R): %f sec\n", difftime(end_time, start_time));
//     printf("Length of fp: %ld\n", distance);
//     // write_trees(findpath_list, "./output/fp.rtree");
//     // printf("distance: %d\n", distance(5, "[{1,2},{1,2,3},{1,2,3,4},{1,2,3,4,5}]", "[{4,5},{3,4,5},{1,2},{1,2,3,4,5}]"));
//     return 0;
// }
