__author__ = "Joseph Grace"

from tree_functions import *
from tree_parser.tree_io import *

from external_functions import *

from sys import setrecursionlimit, getsizeof

from generate import all_unlabelled_trees, random_tree
from helper_functions import *

from orbits import all_destinations_findpath_orbit_sizes_cached, all_destinations_findpath_orbit_sizes_cached_cache

from uniform_sample import uniform_sample_orbit, uniform_sample_ball

import matplotlib.pyplot as plt
import numpy as np


## Orbit Count Data Structure

class OrbitCount:
    def __init__(self, reach, count=0):
        self.reach = reach
        self.count = count
        self.children = []
        self.cache = None
    
    def link(self, other, offset):
        if self.cache is not None:
            raise RuntimeError("Could invalidate caches")
        self.children.append((other, offset))
        self.cache = None
    
    def get(self, k):
        if self.cache is not None:
            return self.cache[k]
        if k==0:
            return self.count
        else:
            total = 0
            for child, offset in self.children:
                total += child.get(k-offset)
    
    def resolve(self):
        if self.cache is not None:
            return self.cache
        self.cache = [self.count]
        for child, offset in self.children:
            add_to(self.cache, child.resolve(), offset)
        return self.cache[:self.reach]
    


## Orbits
hits=0
misses = 0
limited_findpath_cache = {}
def limited_findpath_orbits_cached(tree, limit, min_rank=0, cur_rank=0):
    global hits, misses
    n = tree.contents.num_leaves
    nodes = tree.contents.node_array

    def rec(min_rank, cur_rank, k):
        global hits, misses
        if k==0: return OrbitCount(0)
        if min_rank == cur_rank:
            key = (shape(tree, min_rank+1), 0)
        else:
            key = (shape(tree, min_rank), cur_rank - min_rank)
        if key in limited_findpath_cache:
            cached = limited_findpath_cache[key]
            if cached.reach >= k:
                hits+=1
                return cached
        
        misses += 1
        result = OrbitCount(k)
        if cur_rank == min_rank:
            for new_cur_rank in range(min_rank+2, n):
                delta = rec(min_rank+1, new_cur_rank, k)
                result.link(delta, 0)

        else:
            cur_index = cur_rank + n - 1
            r = cur_index - 1
            if nodes[r].parent == cur_index:
                # NNI moves
                result.count = 2

                nni_move(tree, r, 0)

                delta = rec(min_rank, cur_rank-1, k-1)
                result.link(delta, 1)
                if cur_rank - 1 > min_rank:
                    delta = rec(cur_rank-1, cur_rank-1, k-1)
                    result.link(delta, 1)

                nni_move(tree, r, 1)

                delta = rec(min_rank, cur_rank-1, k-1)
                result.link(delta, 1)
                if cur_rank - 1 > min_rank:
                    delta = rec(cur_rank-1, cur_rank-1, k-1)
                    result.link(delta, 1)

                # Reset
                nni_move(tree, r, 0)
                nodes[r].children[0], nodes[r].children[1] = nodes[r].children[1], nodes[r].children[0]
                
            else:
                # Rank Move
                result.count = 1

                rank_move(tree, r)

                delta = rec(min_rank, cur_rank-1, k-1)
                result.link(delta, 1)
                if cur_rank - 1 > min_rank:
                    delta = rec(cur_rank-1, cur_rank-1, k-1)
                    result.link(delta, 1)
                
                # Reset
                rank_move(tree, r)
        
        limited_findpath_cache[key] = result
        return result

    delta = rec(min_rank, cur_rank, limit)
    result = OrbitCount(limit+1, 1)
    result.link(delta, 1)
    return result

from collections import deque

max_q_size=0
limited_findpath_cache2 = {}
def limited_findpath_orbits_cached2(center_tree, limit, min_rank=0, cur_rank=0):
    global hits, misses
    global max_q_size
    n = center_tree.contents.num_leaves
    q=deque()
    new_tree = get_empty_tree(n)
    copy_tree(new_tree, center_tree)

    final_result = OrbitCount(limit+1, 1)
    q.append((new_tree, min_rank, cur_rank, limit, final_result, 1))

    while len(q)>0:
        max_q_size=max(max_q_size, len(q))
        tree, min_rank, cur_rank, k, prev_link, offset = q.popleft()
        #print(min_rank, cur_rank, k, print_tree_from_root(tree.contents))
        if k==0:
            free_tree(tree)
            continue
        if min_rank == cur_rank:
            key = (shape(tree, min_rank+1), 0)
        else:
            key = (shape(tree, min_rank), cur_rank - min_rank)
        if key in limited_findpath_cache2:
            cached = limited_findpath_cache2[key]
            if cached.reach >= k:
                hits+=1
                prev_link.link(cached, offset)
                free_tree(tree)
                continue
        
        misses += 1
        result = OrbitCount(k)

        prev_link.link(result, offset)

        if cur_rank == min_rank:
            for new_cur_rank in range(min_rank+2, n):
                new_tree = get_empty_tree(n)
                copy_tree(new_tree, tree)
                q.append((new_tree, min_rank+1, new_cur_rank, k, result, 0))

        else:
            cur_index = cur_rank + n - 1
            r = cur_index - 1
            if tree.contents.node_array[r].parent == cur_index:
                # NNI moves
                result.count = 2

                for i in range(2):
                    new_tree = get_empty_tree(n)
                    copy_tree(new_tree, tree)
                    nni_move(new_tree, r, i)

                    q.append((new_tree, min_rank, cur_rank-1, k-1, result, 1))

                    if cur_rank - 1 > min_rank:
                        new_tree2 = get_empty_tree(n)
                        copy_tree(new_tree2, new_tree)

                        q.append((new_tree2, cur_rank-1, cur_rank-1, k-1, result, 1))
                
            else:
                # Rank Move
                result.count = 1
                new_tree = get_empty_tree(n)
                copy_tree(new_tree, tree)
                rank_move(new_tree, r)

                q.append((new_tree, min_rank, cur_rank-1, k-1, result, 1))

                if cur_rank - 1 > min_rank:
                    new_tree2 = get_empty_tree(n)
                    copy_tree(new_tree2, new_tree)

                    q.append((new_tree2, cur_rank-1, cur_rank-1, k-1, result, 1))
                
        
        limited_findpath_cache2[key] = result
        free_tree(tree)

    #print(final_result)
    return final_result


if __name__ == "__main__":
    """
    n=50
    cat = list(all_unlabelled_trees(n, limit=1))[0]
    for initial_d in range((n-1)*(n-2)//3,(n-1)*(n-2)//2+1):
        print(f"Distance {initial_d} from caterpillar:")
        #limited_findpath_cache.clear()
        tree = uniform_sample(cat, initial_d)
        print(print_tree_from_root(tree.contents, 2*n-2))

        test_ans = limited_findpath_orbits_cached(tree, 10)
        print(test_ans, sum(test_ans))
        print(hits, misses, len(limited_findpath_cache))
        free_tree(tree)
    free_tree(cat)
    """
    setrecursionlimit(10000)

    def fact(x): return 1 if x==0 else fact(x-1)*x
    from math import exp

    n=200
    k=6#(n-1)*(n-2)//2
    xs = np.arange(n,n+1)
    ks = range(1, k+1)
    balls= [[]]+[[] for j in ks]
    for i in xs:
        cat = list(all_unlabelled_trees(i, limit=1))[0]
        free_tree(cat)
        tree = random_tree(i)#uniform_sample_ball(cat, (n-1)*(n-2)//2)
        print(print_tree_from_root(tree.contents))
        try:
            test_ans = limited_findpath_orbits_cached2(tree, k)
        except KeyboardInterrupt:
            pass
        else:
            print("Done, now resolving.")
            test_ans = test_ans.resolve()
            print(test_ans, sum(test_ans))
            for j in ks:
                balls[j].append(sum(test_ans[:j+1]))
            balls.append(sum(test_ans))
        print(hits, misses, len(limited_findpath_cache2), max_q_size)
        free_tree(tree)
    """for j in ks:
        plt.semilogy(xs, balls[j], ".", label=f"{j}")
    plt.xlabel("n")
    plt.legend()
    plt.show()"""
   