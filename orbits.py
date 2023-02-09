__author__ = "Joseph Grace"

from tree_functions import *
from tree_parser.tree_io import *

from external_functions import *
from helper_functions import *

from generate import all_unlabelled_trees


## Orbit sizes

reverse_findpath_orbit_sizes_cached_1_cache = {}
def reverse_findpath_orbit_sizes_cached_1(tree):
    n=tree.contents.num_leaves
    node_array = tree.contents.node_array

    rank_indices = [None] + [index_of_rank(tree, i) for i in range(1, n-1)] # 1-indexed

    k = (n-1)*(n-2)//2

    def rec(max_rank, d):
        key = (shape(tree), max_rank, rank_indices[max_rank]-n+1)
        if key in reverse_findpath_orbit_sizes_cached_1_cache:
            return reverse_findpath_orbit_sizes_cached_1_cache[key].copy()

        result = [1]
        if d >= k: return result
        
        for rank in range(max_rank, 0, -1):

            r = rank_indices[rank]
            if r<2*n-2:
                # Node can be moved up
                rank_indices[rank]=r+1

                if node_array[r].parent==r+1:
                    # NNI Moves
                    nni_move(tree, r, 0)
                    delta = rec(rank, d+1)

                    add_to(result, delta, 1)
                    
                    nni_move(tree, r, 1)
                    delta = rec(rank, d+1)

                    add_to(result, delta, 1)
                    
                    # Reset
                    nni_move(tree, r, 0)
                    node_array[r].children[0], node_array[r].children[1] = node_array[r].children[1], node_array[r].children[0]
    
                else:
                    # Rank Move
                    rank_move(tree, r)
                    delta = rec(rank, d+1)

                    add_to(result, delta, 1)
                    
                    # Reset
                    rank_move(tree, r)
                
                # Reset
                rank_indices[rank]=r

        reverse_findpath_orbit_sizes_cached_1_cache[key] = result
        return result
    
    return rec(n-2, 0)



reverse_findpath_orbit_sizes_cached_2_cache = {}
def reverse_findpath_orbit_sizes_cached_2(tree):
    n=tree.contents.num_leaves
    node_array = tree.contents.node_array

    k = (n-1)*(n-2)//2
    
    def rec(max_rank, cur_rank, d):
        key = (shape(tree), max_rank, cur_rank)
        if key in reverse_findpath_orbit_sizes_cached_2_cache:
            return reverse_findpath_orbit_sizes_cached_2_cache[key].copy()

        if d >= k: return []

        if max_rank - 1 > 0:
            result = rec(max_rank-1, max_rank-1, d).copy()
        else:
            result = [0]

        
        if cur_rank < n-1:
            # Node can be moved up
            r = cur_rank + n - 1

            if node_array[r].parent==r+1:
                # NNI Moves
                result[0] += 2

                nni_move(tree, r, 0)
                delta = rec(max_rank, cur_rank+1, d+1)

                add_to(result, delta, 1)
                
                nni_move(tree, r, 1)
                delta = rec(max_rank, cur_rank+1, d+1)

                add_to(result, delta, 1)
                
                # Reset
                nni_move(tree, r, 0)
                node_array[r].children[0], node_array[r].children[1] = node_array[r].children[1], node_array[r].children[0]

            else:
                # Rank Move
                result[0] += 1

                rank_move(tree, r)
                delta = rec(max_rank, cur_rank+1, d+1)

                add_to(result, delta, 1)
                
                # Reset
                rank_move(tree, r)
        
        if result == [0]: result = []
        reverse_findpath_orbit_sizes_cached_2_cache[key] = result
        return result
    
    return [1] + rec(n-2, n-2, 0)



def all_destinations_findpath_orbit_sizes(tree, k=None):
    n = tree.contents.num_leaves
    nodes = tree.contents.node_array
    if k is None: k = (n-1)*(n-2)//2
    results = [0] * (k+1)

    def rec(min_rank, cur_rank, d):

        if cur_rank == min_rank:
            results[d] += 1
            if d == k: return
            for new_min_rank in range(cur_rank+1, n-1):
                for new_cur_rank in range(new_min_rank+1, n):
                    rec(new_min_rank, new_cur_rank, d)
        
        else:
            if d == k: return
            cur_index = cur_rank + n - 1
            r = cur_index - 1
            if nodes[r].parent == cur_index:
                # NNI moves

                nni_move(tree, r, 0)
                rec(min_rank, cur_rank-1, d+1)

                nni_move(tree, r, 1)
                rec(min_rank, cur_rank-1, d+1)

                # Reset
                nni_move(tree, r, 0)
                nodes[r].children[0], nodes[r].children[1] = nodes[r].children[1], nodes[r].children[0]
                
            else:
                # Rank Move

                rank_move(tree, r)
                rec(min_rank, cur_rank-1, d+1)
                
                # Reset
                rank_move(tree, r)
    
    rec(0, 0, 0)
    return results



all_destinations_findpath_orbit_sizes_cached_cache = {}
def all_destinations_findpath_orbit_sizes_cached(tree, min_rank=0, cur_rank=0, d=0):
    n = tree.contents.num_leaves
    nodes = tree.contents.node_array

    def rec(min_rank, cur_rank, d):
        if min_rank == cur_rank:
            key = (shape(tree, min_rank+1), 0)
        else:
            key = (shape(tree, min_rank), cur_rank - min_rank)
        if key in all_destinations_findpath_orbit_sizes_cached_cache:
            return all_destinations_findpath_orbit_sizes_cached_cache[key]

        result = []
        if cur_rank == min_rank:
            for new_cur_rank in range(min_rank+2, n):
                delta = rec(min_rank+1, new_cur_rank, d)
                add_to(result, delta, 0)

        else:
            # delta = rec(cur_rank, cur_rank, d)
            # add_to(result, delta, 0)

            cur_index = cur_rank + n - 1
            r = cur_index - 1
            if nodes[r].parent == cur_index:
                # NNI moves
                result.append(2)

                nni_move(tree, r, 0)

                delta = rec(min_rank, cur_rank-1, d+1)
                add_to(result, delta, 1)
                if cur_rank - 1 > min_rank:
                    delta = rec(cur_rank-1, cur_rank-1, d+1)
                    add_to(result, delta, 1)

                nni_move(tree, r, 1)

                delta = rec(min_rank, cur_rank-1, d+1)
                add_to(result, delta, 1)
                if cur_rank - 1 > min_rank:
                    delta = rec(cur_rank-1, cur_rank-1, d+1)
                    add_to(result, delta, 1)

                # Reset
                nni_move(tree, r, 0)
                nodes[r].children[0], nodes[r].children[1] = nodes[r].children[1], nodes[r].children[0]
                
            else:
                # Rank Move
                result.append(1)

                rank_move(tree, r)

                delta = rec(min_rank, cur_rank-1, d+1)
                add_to(result, delta, 1)
                if cur_rank - 1 > min_rank:
                    delta = rec(cur_rank-1, cur_rank-1, d+1)
                    add_to(result, delta, 1)
                
                # Reset
                rank_move(tree, r)
        
        all_destinations_findpath_orbit_sizes_cached_cache[key] = result
        return result
        
    results = rec(min_rank, cur_rank, d)
    return [1] + results



all_destinations_findpath_orbit_sizes_cached_alt_cache = {}
def all_destinations_findpath_orbit_sizes_cached_alt(tree):
    n = tree.contents.num_leaves
    nodes = tree.contents.node_array

    def rec(min_rank, cur_rank, d):
        if min_rank == cur_rank:
            key = (shape(tree, min_rank+1), 0)
        else:
            key = (shape(tree, min_rank), cur_rank - min_rank)
        if key in all_destinations_findpath_orbit_sizes_cached_alt_cache:
            return all_destinations_findpath_orbit_sizes_cached_alt_cache[key]

        if cur_rank == min_rank:
            result = [1]

            for new_min_rank in range(cur_rank+1, n-1):
                for new_cur_rank in range(new_min_rank+1, n):
                    delta = rec(new_min_rank, new_cur_rank, d)
                    add_to(result, delta, 0)

        else:
            result = []

            cur_index = cur_rank + n - 1
            r = cur_index - 1
            if nodes[r].parent == cur_index:
                # NNI moves

                nni_move(tree, r, 0)
                delta = rec(min_rank, cur_rank-1, d+1)
                add_to(result, delta, 1)

                nni_move(tree, r, 1)
                delta = rec(min_rank, cur_rank-1, d+1)
                add_to(result, delta, 1)

                # Reset
                nni_move(tree, r, 0)
                nodes[r].children[0], nodes[r].children[1] = nodes[r].children[1], nodes[r].children[0]
                
            else:
                # Rank Move

                rank_move(tree, r)
                delta = rec(min_rank, cur_rank-1, d+1)
                add_to(result, delta, 1)
                
                # Reset
                rank_move(tree, r)
        
        all_destinations_findpath_orbit_sizes_cached_alt_cache[key] = result
        return result

    results = rec(0, 0, 0)
    return results.copy()



## Output

def print_orbit_sizes(start, end):
    """Calls C implementation of reverse FindPath traversal (No caching)."""
    for n in range(start,end+1):
        all_trees = all_unlabelled_trees(n)

        print(f"# {n}-leaved trees - {num_unlabelled_trees(n-1)} shapes")
        print()

        for tree in all_trees:
            k = (n-1)*(n-2)//2
            results = orbit_sizes(tree, k+1)
            results_str = "[" + ", ".join((str(results[i]) for i in range(k+1))) + "]"

            #print(results_str, "-" ,print_tree_from_root(tree.contents, 2*n-2)+";", "-", top_rep(tree)) # Newick representation
            print(results_str,"-",shape(tree)) # Shape tuple representation

            free_results_array(results)
            free_tree(tree)
        print()



def print_orbit_sizes_cached(start, end):
    for n in range(start,end+1):
        reverse_findpath_orbit_sizes_cached_1_cache.clear()
        #cache_2.clear()

        all_trees = all_unlabelled_trees(n)

        print(f"# {n}-leaved trees - {num_unlabelled_trees(n-1)} shapes")
        print()

        for tree in all_trees:
            results = reverse_findpath_orbit_sizes_cached_1(tree)
            #results = orbit_sizes_cached_2(tree)

            #print(results, "-" ,print_tree_from_root(tree.contents, 2*n-2)+";") # Newick representation
            print(results, "-" ,shape(tree)) # Shape tuple representation
            
            free_tree(tree)
        
        print()



def print_findpath_orbits(start, end):
    for n in range(start,end+1):
        all_trees = all_unlabelled_trees(n)

        print(f"# {n}-leaved trees - {num_unlabelled_trees(n-1)} shapes")
        print()

        for tree in all_trees:
            k = (n-1)*(n-2)//2
            results = all_destinations_findpath_orbit_sizes(tree, k)

            print(results,"-",shape(tree)) # Shape tuple representation

            free_tree(tree)
        
        print()



def print_findpath_orbits_cached(start, end):
    for n in range(start,end+1):
        all_trees = all_unlabelled_trees(n)

        print(f"# {n}-leaved trees - {num_unlabelled_trees(n-1)} shapes")
        print()

        for tree in all_trees:
            results = all_destinations_findpath_orbit_sizes_cached(tree)

            print(results,"-",shape(tree)) # Shape tuple representation

            free_tree(tree)
        
        print()


if __name__ == "__main__":
    #print_orbit_sizes_cached(3, 9)
    #print_orbit_sizes(3, 9)
    #print_findpath_orbits_cached(3, 9)

    #print_orbit_sizes(3, 7)
    #print_findpath_orbits(3, 7)

    #print_findpath_orbits_cached(3, 6)
    n=60
    tree = list(all_unlabelled_trees(n, 1))[0]
    print(all_destinations_findpath_orbit_sizes_cached(tree))
    free_tree(tree)

    pass