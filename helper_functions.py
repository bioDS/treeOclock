from tree_functions import *

def shape(tree, start_rank=1):
    """Returns parents of nodes at and above start_rank as a tuple. (Or (None,) if shape is 2-leaved) Can be used for caching."""
    n =tree.contents.num_leaves
    if start_rank == n-1: return (None,)
    node_array = tree.contents.node_array
    return tuple(node_array[node_array[i].parent].time-start_rank+1 for i in range(start_rank+n-1, 2*n-3))



def index_of_rank(tree_p, r):
    """Returns index of node of rank r in tree_p.contents.node_array."""
    if not 1 <= r <= tree_p.contents.num_leaves - 1:
        raise ValueError()
    
    index = tree_p.contents.num_leaves+r-1
    try:
        assert r==tree_p.contents.node_array[index].time
    except AssertionError:
        print_tree(tree_p.contents)
        raise
    return index



def add_to(array, added, offset):
    """
    Increments elements in array with elements in added, at the specified (non-negative) offset.
    Adds zeroes to array as needed.
    """
    for i in range(len(added)):
        while i+offset>=len(array):
            array.append(0)
        array[i+offset] += added[i]


def mrca(tree, a, b):
    """Returns the index of the MRCA of nodes of index a and b in tree"""
    n=tree.contents.num_leaves
    nodes=tree.contents.node_array
    while a!=b:
        if a < b:
            a=nodes[a].parent
        else:
            b=nodes[b].parent
    return a


def max_remaining_reverse_steps(n, max_rank, cur_rank):
    """
    Given the current rank and maximum rank parameters of a reverse FindPath traversal,
    returns the maximum number of moves that may remain in the traversal.
    """
    if cur_rank == n-1:
        max_rank = max_rank-1
        cur_rank = max_rank

    sub = (n-2)-max_rank
    return (
        sub*max_rank +
        (max_rank*(max_rank+1))//2 -
        (cur_rank - max_rank)
    )