#include "orbits.h"

#include "tree.h"
#include "rnni.h"

#include "generate.h"
#include "helpers.h"
#include "types.h"

#include <iostream>
#include <vector>
#include <tuple>
#include <map>

#define fp_key_t std::pair<int, vector<int>>

using std::vector;

static std::map<fp_key_t, vector<count_t>> fp_cache;

vector<count_t> all_orbit_sizes(Tree* tree, int min_rank, int cur_rank){
    fp_key_t key = {cur_rank - min_rank, shape(tree, min_rank)};
    auto cached = fp_cache.find(key);
    if (cached!=fp_cache.end()) {
        return (*cached).second;
    }

    int n = tree->num_leaves;
    Node* nodes = tree->node_array;

    vector<count_t> result;

    if(cur_rank == min_rank){
        result.push_back(1);

        for (int new_min_rank = cur_rank+1; new_min_rank<n-1; new_min_rank++) {
            for (int new_cur_rank = new_min_rank+1; new_cur_rank<n; new_cur_rank++) {
                vector<count_t> delta = all_orbit_sizes(tree, new_min_rank, new_cur_rank);
                add_to(result, delta, 0);
            }
        }

    } else {
        int cur_index = cur_rank + n - 1;
        int r = cur_index - 1;
        if (nodes[r].parent == cur_index) {
            // NNI moves

            nni_move(tree, r, 0);
            vector<count_t> delta = all_orbit_sizes(tree, min_rank, cur_rank-1);
            add_to(result, delta, 1);

            nni_move(tree, r, 1);
            delta = all_orbit_sizes(tree, min_rank, cur_rank-1);
            add_to(result, delta, 1);

            // Reset
            nni_move(tree, r, 0);
            int tmp = nodes[r].children[0];
            nodes[r].children[0] = nodes[r].children[1];
            nodes[r].children[1] = tmp;
            
        } else {
            // Rank Move

            rank_move(tree, r);
            vector<count_t> delta = all_orbit_sizes(tree, min_rank, cur_rank-1);
            add_to(result, delta, 1);
            
            // Reset
            rank_move(tree, r);
        }
    }

    fp_cache[key] = result;
    return result;
}

vector<count_t> all_orbit_sizes(Tree* tree) {
    return all_orbit_sizes(tree, 0, 0);
}