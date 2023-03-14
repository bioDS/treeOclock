##!/usr/bin/env python

# Simulating Trees under coalescent process, i.e. uniform distribution on ranked trees
# Trees genereted in both DCT and ete3 format.

from tree_parser.tree_io import tree_to_cluster_string
import random

from tree_functions import *

def sim_coal(num_leaves, num_trees):
    '''Simulate num_trees trees on num_leaves leaves and save them in a TREE_ARRAY'''
    trees = get_empty_tree_array(num_trees, num_leaves)
    for i in range(0, num_trees):
        # Simulate coalescence events
        current_leaves = [] # list including all current leaves. Starts with list of leaves, in each iteration two elements are replaced by one (which is attached as last element of the list)
        for l in range(1,num_leaves+1):
            current_leaves.append(l)
        for j in range(0,num_leaves-1):
            [n1,n2] = random.sample(current_leaves, k=2)
            # leaves coalescing in internal node of rank j+1 (index n+j+1 in node_list)
            current_leaves.remove(n1)
            current_leaves.remove(n2)
            current_leaves.append(int(num_leaves+j+1)) #add new internal node to leaf set
            # Add new relations to node_list:
            trees.trees[i].node_array[n1-1].parent = num_leaves + j
            trees.trees[i].node_array[n2-1].parent = num_leaves + j
            trees.trees[i].node_array[num_leaves+j].children[0] = n1-1
            trees.trees[i].node_array[num_leaves+j].children[1] = n2-1
            # set times/ranks for internal nodes
            trees.trees[i].node_array[num_leaves + j].time = j + 1
    return(trees)
