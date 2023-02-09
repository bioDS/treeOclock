"""Functions taken/modified from other repos"""
import math

# Function taken from treeXplore, unlabelled_trees.py
def num_unlabelled_trees(n):
    # recursion to compute the number of unlabelled rank trees on n+1 leaves -- Andre's problem (number of alternating permutations)
    if (n == 0 or n == 1):
        return (1)
    # first and last summand are calculated separately
    sum = num_unlabelled_trees(n - 1)
    for k in range(1, n - 1):
        sum += 0.5 * (math.factorial(n - 1) /
                      (math.factorial(n - 1 - k) * math.factorial(k))
                      ) * num_unlabelled_trees(n - 1 -
                                               k) * num_unlabelled_trees(k)
    return int(sum)

# Function modified from Summarising-Ranked-Phylogenetic-Trees, nwk_parser.py
def print_tree_from_root(tree, index=None):
    """Recursive Function to get the nwk representation of a node based tree, modified for use with C Tree struct, guarantees unique representation."""
    node_array = tree.node_array
    n = tree.num_leaves
    if index is None: index = 2*n-2
    node = node_array[index]
    left = node.children[0]
    right = node.children[1]
    if left > right:
        left,right = right,left
    if left < n and right < n:
        # Merge two leafs
        return f'({left+1}:{node.time},{right+1}:{node.time})'
    elif left < n and right >= n:
        # Merge left leaf and right subtree
        return f'({left+1}:{node.time},{print_tree_from_root(tree, right)}:{node.time - node_array[right].time})'
    else:
        # Merge two subtrees
        return f'({print_tree_from_root(tree, left)}:{node.time - node_array[left].time},{print_tree_from_root(tree, right)}:{node.time - node_array[right].time})'