__author__ = "Joseph Grace"

from tree_functions import *

import random

def all_shapes(n):
    """
    Returns a generator that iterates over all tree shapes of n leaves. (n>=2)
    These topologies are represented as a list of parent *indexes* in a node_array of all nodes from rank 1 to n-2 inclusive.
    Ranks are 1-indexed.
    May (?) generate shapes in increasing lexicographic order.
    """
    if n==2:
        yield (None,)
        return
    
    shape = [-1]*(n-3) # 0-indexed
    available_ranks = [False] *(n-1) # 1-indexed by rank. Includes ranks 1 to n-2

    def rec(n, shape, available_ranks, next_leaf, next_rank):
        if next_rank==n-1:
            # Fill in last node, truncating last (redundant) element.
            for i in range(1, n-2):
                if available_ranks[i]:
                    shape[i-1] = next_rank
                    continue
            
            yield tuple(shape)
            return
            
        available_ranks[next_rank] = True

        
        # Consider adding two internal nodes as children
        for i in range(1, next_rank):
            if not available_ranks[i]: continue

            shape[i-1]=next_rank

            available_ranks[i] = False

            for j in range(i+1, next_rank):
                if not available_ranks[j]: continue
                
                shape[j-1]=next_rank

                available_ranks[j] = False
                yield from rec(n, shape, available_ranks, next_leaf, next_rank+1)

                # Reset
                available_ranks[j] = True
            
            available_ranks[i] = True
        
        # Consider adding a leaf and internal node as children.
        if next_leaf <= n:
            for i in range(1, next_rank):
                if not available_ranks[i]: continue

                shape[i-1]=next_rank

                available_ranks[i] = False

                yield from rec(n, shape, available_ranks, next_leaf+1, next_rank+1)

                # Reset
                available_ranks[i] = True

        # Consider adding two leaves as children
        if next_leaf <= n-1:
            yield from rec(n, shape, available_ranks, next_leaf+2, next_rank+1)
        
        # Reset
        available_ranks[next_rank] = False 

    yield from rec(n, shape, available_ranks,1,1)

def new_tree_from_shape(n, shape):
    """ Returns a new tree with the given shape. Requires n>=2."""
    if shape == (None,):
        shape = ()
    else:
        shape += (n-1,)

    tree = get_empty_tree(n)

    set_child = [0]*(2*n-1) # 0-indexed Tree indices

    # Internal nodes
    for i in range(1, n-1):
        index = i+n-1
        parent_index = shape[i-1]+n-1
        
        tree.contents.node_array[index].time = i

        tree.contents.node_array[index].parent = parent_index
        tree.contents.node_array[parent_index].children[set_child[parent_index]] = index
        set_child[parent_index] += 1

    # Fill leaves
    index = 0
    for parent_index in range(n, 2*n-1):
        while set_child[parent_index] < 2:
            tree.contents.node_array[index].parent = parent_index
            tree.contents.node_array[parent_index].children[set_child[parent_index]] = index
            set_child[parent_index] += 1
            index += 1

    return tree

def all_unlabelled_trees(n, limit=-1):
    """
    Returns a generator that iterates over all unlabelled trees on <n> leaves.
    If limit is given, the generator will only generate up to <limit> trees.
    
    IMPORTANT: Trees returned by this generator need to be freed with free_tree()!
    """
    limiter = 0
    if limiter == limit: return
    for shape in all_shapes(n):
        yield new_tree_from_shape(n, shape)
        limiter += 1
        if limiter == limit: break


def random_shape(n):
    """Randomly build a tree shape. (Not (?) uniform (May be uniform w.r.t labelled tree space))."""
    if n == 2:
        return (None,)
    
    shape = [-1] * (n-2) # 0-indexed.
    # rank_choices = {0}
    # leaf_count = 0

    rank_choices = range(0, n)
    rank_weights = [n] + [0]*(n-1)

    for parent in range(1, n):
        for j in range(2):
            child = random.choices(rank_choices, k=1, weights = rank_weights)[0]
            if child > 0:
                shape[child-1] = parent
            rank_weights[child] -= 1

        rank_weights[parent] = 1
        
    return tuple(shape[:-1])

def random_tree(n):
    """Returns tree built from random_shape (Not (?) uniform (May be uniform w.r.t labelled tree space))"""
    parents = random_shape(n)
    return new_tree_from_shape(n, parents)
