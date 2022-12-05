#include "tree.h"
#include "rnni.h"
#include <stdio.h>

static int index_of_rank(Tree* tree, long r){
    return tree->num_leaves + r - 1;
}

static void reverse_findpath(long* results, int results_size, Tree* cur_tree, int d, long* rank_indices, long max_rank, long mult) {
    long n = cur_tree->num_leaves;
    Node* node_array = cur_tree->node_array;
    results[d] += mult;
    if (d+1 >= results_size)
        return;
    
    for (long rank=max_rank; rank>=1; rank--) {
        long r = rank_indices[rank];
        if (r < 2*n-2) {
            // Node can be moved up
            rank_indices[rank] += 1;

            if (node_array[r].parent == r+1) {
                // NNI moves
                if (node_array[r].children[0] < n && node_array[r].children[1] < n) {
                    // Cherry - can optimise
                    nni_move(cur_tree, r, 0);
                    reverse_findpath(results, results_size, cur_tree, d+1, rank_indices, rank, mult*2);

                    // Reset
                    nni_move(cur_tree, r, 0);

                } else {
                    nni_move(cur_tree, r, 0);
                    reverse_findpath(results, results_size, cur_tree, d+1, rank_indices, rank, mult);

                    nni_move(cur_tree, r, 1);
                    reverse_findpath(results, results_size, cur_tree, d+1, rank_indices, rank, mult);

                    // Reset
                    nni_move(cur_tree, r, 0);
                    long tmp = node_array[r].children[0];
                    node_array[r].children[0] = node_array[r].children[1];
                    node_array[r].children[1] = tmp;
                }
            } else {
                // Rank move
                rank_move(cur_tree, r);
                reverse_findpath(results, results_size, cur_tree, d+1, rank_indices, rank, mult);
                // Reset
                rank_move(cur_tree, r);
            }
            // Reset
            rank_indices[rank] -= 1;
        }
    }
}

long* orbit_sizes(Tree* tree, long k) {
    long n = tree->num_leaves;
    if (k<0) {
        k = (n-1)*(n-2)/2;
    }
    long* results = calloc(k+1, sizeof(long));
    long* rank_indices = calloc(n-2+1, sizeof(long));
    for (long i=1; i<=n-2; i++) {
        rank_indices[i] = index_of_rank(tree, i);
    }
    long max_rank = n-2;

    reverse_findpath(results, k+1, tree, 0, rank_indices, max_rank, 1);

    free(rank_indices);

    return results;
}

void free_results_array(long* array){
    free(array);
}