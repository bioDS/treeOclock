#include "tree.h"
#include "helpers.h"

#include "generate.h"
#include "orbits.h"
#include "types.h"

#include <iostream>
#include <vector>

static void print_vector(std::vector<int> &v) {
    std::cout<<"(";
        for (size_t i=0; i<v.size(); i++){
            std::cout<<v[i];
            if (i<v.size()-1)
                std::cout<<", ";
        }
        std::cout<<")";
}

static void print_vector(std::vector<count_t> &v) {
    std::cout<<"[";
        for (size_t i=0; i<v.size(); i++){
            std::cout<<v[i];
            if (i<v.size()-1)
                std::cout<<", ";
        }
        std::cout<<"]";
}

int main () {
    int start, end;
    limit_t limit;
    std::cin>>start>>end>>limit;
    for (int n=start; n<=end; n++) {
        std::vector<Tree*> all_trees = all_ul_trees(n, limit);
        std::cout<<"# "<<n<<"-leaved trees - "<<all_trees.size()<<" shapes"<<std::endl;
        std::cout<<std::endl;
        for (Tree* tree: all_trees) {
            std::vector<count_t> result = all_orbit_sizes(tree);
            std::vector<int> rep = shape(tree,1);
            free_tree(tree);

            print_vector(result);
            std::cout<<" - ";
            print_vector(rep);
            std::cout<<std::endl;
        }
        std::cout<<std::endl;
    }
}