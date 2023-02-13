__author__ = "Joseph Grace"

from tree_functions import *

from random import choice, choices

def all_topologies(n):
    """
    Returns a generator that iterates over all tree topologies of n leaves.
    These topologies are represented as a list of parent *indexes* in a node_array of all nodes from rank 1 to n-2 inclusive.
    """
    def rec(n, working, available_ranks, next_leaf, next_rank):
        if next_rank==2*n-1:
            yield working.copy()
            return
            
        available_ranks[next_rank] = True

        if next_rank >= n+2:
            for i in range(n, next_rank):
                if not available_ranks[i]: continue

                tmp1 = working[i]
                working[i]=next_rank

                available_ranks[i] = False

                for j in range(i+1, next_rank):
                    if not available_ranks[j]: continue
                    tmp2 = working[j]
                    working[j]=next_rank

                    available_ranks[j] = False
                    yield from rec(n, working, available_ranks, next_leaf, next_rank+1)

                    # Reset
                    working[j]=tmp2
                    available_ranks[j] = True
                
                working[i]=tmp1
                available_ranks[i] = True
        
        if next_leaf < n:
            for i in range(n, next_rank):
                if not available_ranks[i]: continue

                tmp = working[i]
                working[i]=next_rank

                working[next_leaf]=next_rank

                available_ranks[i] = False

                yield from rec(n, working, available_ranks, next_leaf+1, next_rank+1)

                # Reset
                working[i] = tmp
                available_ranks[i] = True

        if next_leaf+1<n:
            working[next_leaf]=next_rank
            working[next_leaf+1]=next_rank

            yield from rec(n, working, available_ranks, next_leaf+2, next_rank+1)
        
        # Reset
        available_ranks[next_rank] = False
    
    
    working = [-1]*(2*n-1)
    available_ranks = [False] *(2*n-1)

    yield from rec(n, working, available_ranks,0,n)

def new_tree_from_parents(n, parents):
    tree = get_empty_tree(n)
    set_child = [0]*(2*n-1)
    for j in range(2*n-1):
        if j >= n:
            tree.contents.node_array[j].time = j-n+1
        parent = parents[j]
        if parent < 0 or parent >= 2*n-1: continue
        tree.contents.node_array[j].parent = parent
        tree.contents.node_array[parent].children[set_child[parent]] = j
        set_child[parent] += 1
    return tree

def all_unlabelled_trees(n, limit=None):
    """
    Returns a generator that iterates over all unlabelled trees on n leaves.
    If limit is given, the generator will only generate up to limit trees.
    
    IMPORTANT: Trees returned by this generator need to be freed with free_tree()!
    """
    all_top = all_topologies(n)
    i = 0
    for parents in all_top:
        yield new_tree_from_parents(n, parents)
        i += 1
        if limit is not None and i>=limit:
            break


def random_topology(n):
    shape = [-1] * (2*n-1)
    
    counts = [0] * (2*n-1)
    available = set([2*n-2])
    for index in range(2*n-3, -1, -1):
        #print(shape, available)
        parent = choice(list(available))
        if counts[parent] == 1:
            available.remove(parent)
        counts[parent]+=1
        shape[index] = parent
        if index >=n:
            available.add(index)
    return tuple(shape)

def random_tree(n):
    parents = random_topology(n)
    #print(parents)
    return new_tree_from_parents(n, parents)