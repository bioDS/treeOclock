__author__ = "Joseph Grace"

"""Functions for enumerating and sampling from the diameter orbit."""

from tree_functions import *
from helper_functions import *
from external_functions import *

import sys

sys.setrecursionlimit(1000**2)

diameter_orbit_size_cache = {}
def diameter_orbit_size(tree, min_rank=0, cur_rank=0, d=0):
    n = tree.contents.num_leaves
    nodes = tree.contents.node_array

    def rec(min_rank, cur_rank, d):
        # print(min_rank, cur_rank)
        if min_rank == cur_rank:
            key = (shape(tree, min_rank+1), 0)
        else:
            key = (shape(tree, min_rank), cur_rank - min_rank)
        if key in diameter_orbit_size_cache:
            return diameter_orbit_size_cache[key]

        
        if cur_rank == min_rank:
            min_rank += 1
            cur_rank = n-1

        
        result = 0
        if min_rank == n-1:
            result = 1
        
        else:

            cur_index = cur_rank + n - 1
            r = cur_index - 1
            if nodes[r].parent == cur_index:
                # NNI moves

                nni_move(tree, r, 0)

                result += rec(min_rank, cur_rank-1, d+1)

                nni_move(tree, r, 1)

                result += rec(min_rank, cur_rank-1, d+1)

                # Reset
                nni_move(tree, r, 0)
                nodes[r].children[0], nodes[r].children[1] = nodes[r].children[1], nodes[r].children[0]
                
            else:
                # Rank Move

                rank_move(tree, r)

                result += rec(min_rank, cur_rank-1, d+1)
                
                # Reset
                rank_move(tree, r)
        
        diameter_orbit_size_cache[key] = result
        return result
        
    result = rec(min_rank, cur_rank, d)
    return result

if __name__ == "__main__":
    from generate import *
    from orbits import all_destinations_findpath_orbit_sizes_cached
    
    n=100

    for _ in range(1):
        # tree = random_tree(n)
        tree = new_tree_from_shape(n, tuple(i for i in range(2, n-1)))
        print(print_tree_from_root(tree.contents))
        # print(all_destinations_findpath_orbit_sizes_cached(tree))
        print(diameter_orbit_size(tree))
        free_tree(tree)
