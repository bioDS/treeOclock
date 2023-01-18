#include "helpers.h"

#include "tree.h"
#include <vector>

int index_of_rank(Tree* tree, int r){
    return tree->num_leaves + r - 1;
}

std::vector<int> shape(Tree* tree, int start_rank) {
    int n = tree->num_leaves;
    if (n==2)
        return {-1};
    Node* node_array = tree->node_array;
    std::vector<int> rep;
    for (int i=start_rank+n-1; i<2*n-3; i++) {
        rep.push_back(node_array[node_array[i].parent].time - start_rank + 1);
    }
    return rep;
}

void add_to(std::vector<count_t> &array, std::vector<count_t> &added, int offset){
    for(size_t i=0; i<added.size(); i++){
        while (i+offset>=array.size()){
            array.push_back(0);
        }
        array[i+offset] += added[i];
    }
}