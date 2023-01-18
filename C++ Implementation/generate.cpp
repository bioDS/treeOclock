#include "generate.h"

#include "tree.h"
#include "types.h"

#include <vector>
#include <stack>

using std::vector;

static void all_rec(vector<vector<int>> &results, int n, vector<int> &working, vector<bool> &available_ranks, int next_leaf, int next_rank, limit_t limit) {
    if (next_rank==2*n-1){
        results.push_back(working);
        return;
    }

    available_ranks[next_rank] = true;

    if (next_rank >= n+2) {
        for (int i=n; i<next_rank; i++) {
            if (!available_ranks[i]) continue;

            int tmp1 = working[i];
            working[i]=next_rank;
            available_ranks[i]=false;

            for (int j=i+1; j<next_rank; j++){
                if (!available_ranks[j]) continue;

                int tmp2 = working[j];
                working[j]=next_rank;
                available_ranks[j]=false;
                if (limit == 0 || results.size() < limit)
                    all_rec(results, n, working, available_ranks, next_leaf, next_rank+1, limit);
                else
                    return;

                // Reset
                working[j]=tmp2;
                available_ranks[j]=true;
            }
            working[i]=tmp1;
            available_ranks[i]=true;
        }
    }
    if (next_leaf < n){
        for (int i=n; i<next_rank; i++) {
            if (!available_ranks[i]) continue;

            int tmp = working[i];
            working[i]=next_rank;
            available_ranks[i]=false;

            working[next_leaf]=next_rank;
            if (limit == 0 || results.size() < limit)
                all_rec(results, n, working, available_ranks, next_leaf+1, next_rank+1, limit);
            else
                return;

            // Reset
            working[i]=tmp;
            available_ranks[i]=true;
        }
    }
    if (next_leaf+1<n) {
        working[next_leaf]=next_rank;
        working[next_leaf+1]=next_rank;
        if (limit == 0 || results.size() < limit)
            all_rec(results, n, working, available_ranks, next_leaf+2, next_rank+1, limit);
    }
    
    available_ranks[next_rank] = false;
}

vector<vector<int>> all_top_rep(int n, limit_t limit) {
    vector<int> working (2*n-1, -1);
    vector<bool> available_ranks (2*n-1, false);
    vector<vector<int>> results;
    all_rec(results, n, working, available_ranks, 0, n, limit);
    return results;
}

vector<Tree*> all_ul_trees(int n, limit_t limit) {
    vector<vector<int>> all_top = all_top_rep(n, limit);
    vector<Tree*> trees;
    for (size_t i=0; i<all_top.size(); i++) {
        Tree* tree = get_empty_tree(n);
        vector<int> parents = all_top[i];
        vector<int> set_child (2*n-1, 0);
        for (int j=0; j<2*n-1; j++) {
            if (j >= n)
                tree->node_array[j].time = j-n+1;
            int parent = all_top[i][j];
            if (parent < 0 or parent >= 2*n-1) continue;
            tree->node_array[j].parent = parent;
            tree->node_array[parent].children[set_child[parent]] = j;
            set_child[parent]++;
        }
        trees.push_back(tree);
    }
    return trees;
}

// vector<int> next_topology (vector<int> & topology) {
//     static vector<int> = for
//     if (next_rank==2*n-1){
//         results.push_back(working);
//         return;
//     }

//     available_ranks[next_rank] = true;

//     if (next_rank >= n+2) {
//         for (int i=n; i<next_rank; i++) {
//             if (!available_ranks[i]) continue;

//             int tmp1 = working[i];
//             working[i]=next_rank;
//             available_ranks[i]=false;

//             for (int j=i+1; j<next_rank; j++){
//                 if (!available_ranks[j]) continue;

//                 int tmp2 = working[j];
//                 working[j]=next_rank;
//                 available_ranks[j]=false;
//                 if (limit == 0 || results.size() < limit)
//                     all_rec(results, n, working, available_ranks, next_leaf, next_rank+1, limit);
//                 else
//                     return;

//                 // Reset
//                 working[j]=tmp2;
//                 available_ranks[j]=true;
//             }
//             working[i]=tmp1;
//             available_ranks[i]=true;
//         }
//     }
//     if (next_leaf < n){
//         for (int i=n; i<next_rank; i++) {
//             if (!available_ranks[i]) continue;

//             int tmp = working[i];
//             working[i]=next_rank;
//             available_ranks[i]=false;

//             working[next_leaf]=next_rank;
//             if (limit == 0 || results.size() < limit)
//                 all_rec(results, n, working, available_ranks, next_leaf+1, next_rank+1, limit);
//             else
//                 return;

//             // Reset
//             working[i]=tmp;
//             available_ranks[i]=true;
//         }
//     }
//     if (next_leaf+1<n) {
//         working[next_leaf]=next_rank;
//         working[next_leaf+1]=next_rank;
//         if (limit == 0 || results.size() < limit)
//             all_rec(results, n, working, available_ranks, next_leaf+2, next_rank+1, limit);
//     }
    
//     available_ranks[next_rank] = false;
// }

// Tree* next_unlabelled_tree(int n, bool reset){

// }