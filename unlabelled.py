__author__ = "Joseph Grace"
"""
Unlabelled RNNI Distance Computation
Work in Progress!
"""

from tree_functions import *

from orbits import shape, add_to

from generate import *

from math import inf

import matplotlib.pyplot as plt
from math import log

unlabelled_distance_safe_cache = {}
def unlabelled_distance_safe(tree, goal, min_rank=0, cur_rank=0, d=inf):
    n = tree.contents.num_leaves
    nodes = tree.contents.node_array
    assert goal.contents.num_leaves == n

    goal_shape = shape(goal)

    def rec(min_rank, cur_rank, d):
        if d==0: return 0
        if min_rank == cur_rank:
            for rank in range(1, min(n-2,min_rank)):
                if goal_shape[rank-1] <= min_rank and shape(tree)[rank-1] != goal_shape[rank-1]:
                    return inf

        # if min_rank == 0:
        #     key=None
        # else:
        #     key = (shape(tree, min_rank), cur_rank - min_rank)

        # key = (shape(tree), min_rank, cur_rank)

        # if key is not None and key in ul_cache:
        #     return ul_cache[key]

        result = d

        if cur_rank == min_rank:
            if min_rank == n-1:
                assert shape(tree) == goal_shape
                return 0
            for new_cur_rank in range(min_rank+1, n):
                delta = rec(min_rank+1, new_cur_rank, result)
                result = min(result, delta)

        else:
            cur_index = cur_rank + n - 1
            r = cur_index - 1
            if nodes[r].parent == cur_index:
                # NNI moves

                nni_move(tree, r, 0)
                delta = rec(min_rank, cur_rank-1, result-1)
                result = min(result, delta+1)

                nni_move(tree, r, 1)
                delta = rec(min_rank, cur_rank-1, result-1)
                result = min(result, delta+1)

                # Reset
                nni_move(tree, r, 0)
                nodes[r].children[0], nodes[r].children[1] = nodes[r].children[1], nodes[r].children[0]
                
            else:
                # Rank Move

                rank_move(tree, r)
                delta = rec(min_rank, cur_rank-1, result-1)
                result = min(result, delta+1)
                
                # Reset
                rank_move(tree, r)
        
        # ul_cache[key] = result
        # print(min_rank, cur_rank, print_tree_from_root(tree.contents), result)
        return result
        
    results = rec(min_rank, cur_rank, d)
    return results




def partial_key(tree, cutoff=1):
    n=tree.contents.num_leaves
    nodes = tree.contents.node_array

    index = 0
    
    ret = [[] for i in range(n-cutoff)]

    for i in range(n, 2*n-2):
        parent_rank = nodes[i].parent-n+1
        #print(parent_rank)
        if parent_rank >= cutoff:
            ret[parent_rank-cutoff].append(index)
            index+=1
    return tuple(tuple(c) for c in ret)
        


unlabelled_distance_hybrid_cache={}
def unlabelled_distance_hybrid(tree, dest, upper_bound=0):
    n = tree.contents.num_leaves
    assert n == dest.contents.num_leaves
    nodes = tree.contents.node_array
    dest_nodes = dest.contents.node_array

    dest_partial_keys = [None] + [partial_key(dest, rank) for rank in range(1, n)]
    #print(*dest_partial_keys, sep='\n')
    
    # Build Free Leaves Structure
    free_leaves = [1]*n + [0]*(n-1)
    for i in range(n, 2*n-1):
        free_leaves[i] = free_leaves[nodes[i].children[0]] + free_leaves[nodes[i].children[1]]


    def add_to_free(index, amount, free_leaves=free_leaves):
        while index!=-1:
            free_leaves[index]+=amount
            index = nodes[index].parent

    redone = [0]
    
    def rec(min_rank, cur_rank, resolve, d, result, upper_bound):

        # print(d, min_rank, cur_rank, "resolving", resolve, print_tree_from_root(tree.contents), free_leaves)

        # result = inf
        if result <= cur_rank-min_rank:
            return inf
        
        # if result <= upper_bound:
        #     return result

        prev_bound = result

        # if min_rank == cur_rank:
        #     key = (partial_key(tree, min_rank+1), partial_key(dest, min_rank+1), 0)
        # else:
        #     key = (partial_key(tree, min_rank), partial_key(tree, min_rank), min_rank - cur_rank)

        if min_rank == 0:
            key = None
        else:
            key = (partial_key(tree, min_rank), dest_partial_keys[min_rank], min_rank - cur_rank)
        
        if key is not None and key in unlabelled_distance_hybrid_cache:
            cached = unlabelled_distance_hybrid_cache[key]
            if cached[2] >= prev_bound:
                cached[1]+=1
                return cached[0]
            redone[0]+=1


        

        if cur_rank == min_rank:
            min_index = min_rank+n-1
            # if min_rank > 0:
            #     children = (nodes[min_index].children[0], nodes[min_index].children[1])
            #     dest_children = (dest_nodes[min_index].children[0], dest_nodes[min_index].children[1])
            #     if sorted([nodes[child].time for child in children]) != sorted([dest_nodes[child].time for child in dest_children]):
            #         print("Uh Oh!")
            #         # print(children, dest_children)
            #         # print(sorted([nodes[child].time for child in children]), sorted([dest_nodes[child].time for child in dest_children]))
            #         raise AssertionError

            if min_rank == n-2:
                assert shape(tree) == shape(dest)
                return 0

            # Modify free_leaves
            if min_rank > 0:
                removed_leaves = free_leaves[min_index]
                add_to_free(min_index, -removed_leaves)
            else:
                removed_leaves = 0

            min_rank += 1
            min_index += 1
            a = dest_nodes[min_index].children[0]
            b = dest_nodes[min_index].children[1]
            if a>b: a,b=b,a

            if b<n:
                resolve = (None, None)
                for new_cur_rank in range(min_rank, n):
                    if free_leaves[new_cur_rank+n-1] >= 2:
                        result = min(result, rec(min_rank, new_cur_rank, resolve, d, result, upper_bound))

            elif a<n:
                resolve = (b, None)
                new_cur_index = nodes[b].parent
                while free_leaves[new_cur_index] < 1:
                    new_cur_index = nodes[new_cur_index].parent
                    # assert new_cur_index != -1
                while new_cur_index != -1:
                    new_cur_rank = new_cur_index-n+1
                    result = min(result, rec(min_rank, new_cur_rank, resolve, d, result, upper_bound))
                    new_cur_index = nodes[new_cur_index].parent
            else:
                resolve = (a,b)
                new_cur_rank = mrca(tree, a,b) -n+1
                result = min(result, rec(min_rank, new_cur_rank, resolve, d, result, upper_bound))

            # Reset free_leaves
            min_index-=1
            add_to_free(min_index, removed_leaves)

        else:
            cur_index = cur_rank + n - 1
            r = cur_index - 1
            if nodes[r].parent == cur_index:
                # NNI move(s)
                a,b=resolve

                if a is None:
                    for i in range(2):
                        #child_index = nodes[r].children[i]
                        other_child_index = nodes[r].children[i^1]
                        old_free_r = free_leaves[r]
                        new_free_r = free_leaves[cur_index] - free_leaves[r] + free_leaves[other_child_index]
                        if new_free_r >= 2:
                            nni_move(tree, r, i)
                            free_leaves[r] = new_free_r
                            result = min(result, 1+rec(min_rank, cur_rank-1, resolve, d+1, result-1, upper_bound-1))
                            nni_move(tree, r, i) # Reset
                            free_leaves[r] = old_free_r
               
                elif b is None:
                    for i in range(2):
                        child_index = nodes[r].children[i]
                        other_child_index = nodes[r].children[i^1]
                        old_free_r = free_leaves[r]
                        new_free_r = free_leaves[cur_index] - free_leaves[r] + free_leaves[other_child_index]
                        
                        if mrca(tree, a, child_index) != child_index and new_free_r >= 1:
                            # assert mrca(tree, a, child_index) == r or mrca(tree, a, child_index) == cur_index
                            nni_move(tree, r, i)
                            free_leaves[r] = new_free_r
                            result = min(result, 1+rec(min_rank, cur_rank-1, resolve, d+1, result-1, upper_bound-1))
                            nni_move(tree, r, i) # Reset
                            free_leaves[r] = old_free_r # Reset
                
                else:
                    if mrca(tree, a, r) == cur_index:
                        a,b=b,a
                    # assert mrca(tree,b,r) == cur_index
                    # assert mrca(tree, a, r) == r
                    for i in range(2):
                        child_index = nodes[r].children[i]
                        other_child_index = nodes[r].children[i^1]
                        old_free_r = free_leaves[r]
                        new_free_r = free_leaves[cur_index] - free_leaves[r] + free_leaves[other_child_index]
                        if mrca(tree, a, child_index) != child_index:
                            # assert mrca(tree, a, child_index) == r
                            nni_move(tree, r, i)
                            free_leaves[r] = new_free_r
                            result = min(result, 1+rec(min_rank, cur_rank-1, resolve, d+1, result-1, upper_bound-1))
                            nni_move(tree, r, i) # Reset
                            free_leaves[r] = old_free_r # Reset
                            break

            else:
                # Rank Move

                a,b=resolve
                # if b is not None:
                #     assert mrca(tree, a, b) == cur_index
                # elif a is not None:
                #     assert mrca(tree, a, cur_index) == cur_index
                #     assert free_leaves[cur_index] >= 1
                # else:
                #     assert free_leaves[cur_index] >= 2

                rank_move(tree, r)
                free_leaves[r], free_leaves[r+1] = free_leaves[r+1], free_leaves[r]
                result = min(result, 1+rec(min_rank, cur_rank-1, resolve, d+1, result-1, upper_bound-1))
                free_leaves[r], free_leaves[r+1] = free_leaves[r+1], free_leaves[r] # Reset
                rank_move(tree, r) # Reset
        #if result < prev_bound:
        unlabelled_distance_hybrid_cache[key] = [result, 0, prev_bound]
        return result
        
    results = rec(0, 0, (None, None), 0, inf, upper_bound)
    return results, redone[0]

def cache_freqs(cache,bin_size=1):
    freqs={}
    for value in cache.values():
        index = value[1]//bin_size
        freqs[index] = freqs.get(index, 0) + 1
    return freqs

def plot_performance(max_n, num_trials):
    points = []
    for n in range(4, max_n+1):
        print(f"n: {n}", flush=True)
        for i in range(num_trials):
            tree1=random_tree(n)
            tree2=random_tree(n)
            dist, count = unlabelled_distance_hybrid(tree1, tree2)
            print(dist, flush=True)
            count += len(unlabelled_distance_hybrid_cache)
            points.append((n, count))
            unlabelled_distance_hybrid_cache.clear()

            free_tree(tree1, tree2)
    xs, ys=list(zip(*points))
    plt.figure(0)
    plt.plot(xs, ys, ".")

    plt.figure(1)
    plt.plot(xs, [log(y) for y in ys], ".")

    plt.show()

if __name__ == "__main__":
    from generate import random_tree
    from external_functions import print_tree_from_root

    # random.seed(0)

    n=17

    plot_performance(n, 3)
    
    # # all_top = list(all_topologies(n))
    # # tree1 = new_tree_from_parents(n, all_top[0])
    # # tree2 = new_tree_from_parents(n, all_top[-1])
    # # print("#", flush=True)
    # tree1=random_tree(n)
    # tree2=random_tree(n)
    
    # print(print_tree_from_root(tree1.contents))
    # print(print_tree_from_root(tree2.contents))
    # print()

    # print(unlabelled_distance_hybrid(tree1, tree2))

    # print(*sorted(cache_freqs(unlabelled_distance_hybrid_cache).items()), sep='\n')
    # print(len(unlabelled_distance_hybrid_cache))

    # unlabelled_distance_hybrid_cache.clear()
    # print()

    # print(unlabelled_distance_hybrid(tree2, tree1))

    
    # print(*sorted(cache_freqs(unlabelled_distance_hybrid_cache).items()), sep='\n')
    # print(len(unlabelled_distance_hybrid_cache))

    # free_tree(tree1)
    # free_tree(tree2)

    # trees1 = list(all_unlabelled_trees(n))
    # trees2 = list(all_unlabelled_trees(n))

    # for tree1 in trees1:
    #     for tree2 in trees2:
    #         ans = unlabelled_distance_pruned(tree1, tree2)
    #         true_ans = unlabelled_distance_safe(tree1, tree2)
    #         # if ans!=true_ans:
    #         #     print(print_tree_from_root(tree1.contents))
    #         #     print(print_tree_from_root(tree2.contents))
    #         #     print(ans, true_ans)
    #         #     raise AssertionError
    #         print(ans, end=" ")
    #     print()

    # (free_tree(tree) for tree in trees1)
    # (free_tree(tree) for tree in trees2)
