from collections import OrderedDict
from tree_functions import *
import math
import re
__author__ = 'Lena Collienne, Jordan Kettles'

# Handling Tree input and output (to C)
# Written by Lena Collienne, modified by Jordan Kettles.

import sys

sys.path.append('..')


def get_children_edges_leaves_from_newick(tree_str):
    # Iterate once through tree_str to get
    # (i) dict 'children' of children of each node (intX for node of rank X as keys, children as values)
    # (ii) dict 'edge' of length of edge above every node
    # (iii) list 'leaves' of leaf names

    children = dict()  # contains children for all internal nodes (as sets)
    edges = dict()  # length of edges above every node (internal and leaf)
    leaves = list()  # list of leaf names

    next_parent = list()  # stack of added internal node that do not have two children yet. Next read node will get the last element in this list (i.e. on top of stack) as parent

    # We ignore the first part of the sting up to the first bracket '(', which signals the beginning of the newick string
    i = 0
    while tree_str[i] != '(':
        i += 1
    i += 1  # Add one as we add a node for the bracket before starting the iteration
    # Add node for root (for the first opening bracket '(')
    children['int0'] = set()
    next_parent.append('int0')

    int_node_index = 1  # index of next internal node (X for intX)
    prev_node = ''  # name of the previously considered node

    # Iterate through all position in newick_string
    while i < len(tree_str) - 1:
        # new internal node
        if tree_str[i] == '(':
            new_node = 'int' + str(int_node_index)
            children[new_node] = set()  # Empty set for children
            children[next_parent[len(next_parent) - 1]].add(new_node)
            int_node_index += 1
            prev_node = new_node
            # Parent of new_node is the node on top of the next_parent stack
            next_parent.append(new_node)
            i += 1
        # reached end of internal node
        elif tree_str[i] == ')':
            prev_node = next_parent.pop(len(next_parent) - 1)
            i += 1
        elif tree_str[i] == ',':  # Commas can be ignored. We use parentheses to identify nodes
            i += 1
        # The next element after this is the edge length of the node considered last (which can be internal node or a leaf)
        elif tree_str[i] == ':':
            # Read the numbers following the colon, this is the edge length of the edge leading to prev_node
            i += 1
            edge_length = str()
            while tree_str[i].isnumeric() or tree_str[i] == '.' or tree_str[i] == 'E' or tree_str[i] == 'e' or tree_str[i] == '-':
                edge_length = edge_length + tree_str[i]
                i += 1
            edges[prev_node] = float(edge_length)
        elif tree_str[i] == ';':  # End of newick string
            break
        elif tree_str[i] != '[':
            # We are at a leaf label
            name = str()
            while tree_str[i] != ':':
                name = name + tree_str[i]
                i += 1
            leaves.append(name)
            # Add leaf as child of node on top of next_parent
            children[next_parent[len(next_parent) - 1]].add(name)
            prev_node = name
        # There is some information behind a node in square brackets. We do not need this information, so we ignore it.
        elif tree_str[i] == '[':
            while tree_str[i] != ']':
                i += 1
    return children, edges, leaves


def tree_times(edges, children):
    # return dict containing times of internal nodes
    # We can use the numbering of internal nodes here. Iterating backwards through their names means we go from leaf to root and can assign times to internal nodes
    times = dict()
    for i in range(len(children)-1, -1, -1):
        current_node = 'int' + str(i)
        # Take an arbitrary child (and put back into set)
        child = children[current_node].pop()
        children[current_node].add(child)
        if 'int' in child:
            times[current_node] = times[child] + edges[child]
        else:
            times[current_node] = edges[child]
    return times


# read newick string, return TREE ('Tree', see tree.h)
# 'factor': factor by which the times of internal nodes are multiplied to receive integer-valued times.
# Default factor: 0 -- ranked tree
# How to choose factor>0 for Discrete Coalescent Trees (DCTs) is not obvious -- WIP
def read_newick(s, factor=0):

    factor = float(factor)

    # Get all important information:
    # (i) dict 'children' of children of each node (intX for node of rank X as keys, children as values)
    # (ii) dict 'edge' of length of edge above every node
    # (iii) list 'leaves' of leaf names
    (children, edges, leaves) = get_children_edges_leaves_from_newick(s)
    # dict containing times for all internal nodes
    times = tree_times(edges, children)

    # Create empty node_array for output TREE
    num_nodes = 2*len(children)+1
    node_array = (NODE * num_nodes)()
    for i in range(0, num_nodes):
        node_array[i] = get_empty_node()

    leaves.sort()  # Sort leaves alphabetical to save them in node_array

    # Internal nodes ordered according to ranking (increasing time)
    ranking = list(times.values())
    ranking.sort()
    if len(ranking) != len(set(ranking)):
        print('Error. There are internal nodes with equal times.')
        return 1

    prev_node_time = -1
    # We fill the node_array from top (root) to bottom
    for i in range(num_nodes-1, len(leaves)-1, -1):
        current_node = max(times, key=times.get)

        # Get the integer-valued node time
        if factor > 0:  # In practice we expect factor to be much larger than 1!
            # We multiply times by factor and round them up to next integer
            node_time = int(math.ceil(times.pop(current_node)*factor))
        else:  # In this case we return a ranked tree
            times.pop(current_node)
            node_time = i - len(leaves) + 1
        if prev_node_time > -1 and node_time >= prev_node_time:
            # If there is already a node with this time we need to pick the next lower time that is not taken yet
            # This is prev_node_time - 1
            node_time = prev_node_time - 1
            prev_node_time = node_time
        if node_time == 0:  # we cannot pick a lower time -> factor is to small to get DCT tree
            print('The factor for discretising trees needs to be bigger')
            return 1
        # Set node time in node_array
        node_array[i].time = node_time

        # Find children and add them to node_array
        child_1 = children[current_node].pop()
        child_2 = children[current_node].pop()
        # Add children-parent relationship to node_array for both children (0 and 1)
        for k in [0, 1]:
            child = locals()["child_" + str(k+1)]
            # Distinguish whether child is leaf or not to get correct index in node_array
            if 'int' in child:
                child_rank = ranking.index(times[child])
                node_array[i].children[k] = child_rank + len(leaves)
                node_array[child_rank + len(leaves)].parent = i
            else:
                node_array[i].children[k] = leaves.index(child)
                node_array[leaves.index(child)].parent = i
        # keep the node time for the next iteration to make sure no two nodes get the same time
        prev_node_time = node_time

    # Create and return output tree:
    num_leaves = len(leaves)
    output_tree = TREE(node_array, num_leaves)
    return output_tree


# Read trees from nexus file and save leaf labels as dict and trees as TREE_ARRAY
# factor is the factor for discretising trees -- see read_newick. If 0, we read trees as ranked trees.
def read_nexus(file_handle, factor=0):
    # Precompiled Regex for a line containing a tree
    re_tree = re.compile("\t?tree .*=? (.*);$", flags=re.I | re.MULTILINE)
    # Used to delete the ; and a potential branchlength of the root

    # Count the number of lines fitting the tree regex
    num_trees = len(re_tree.findall(open(file_handle).read()))
    # running variables for reading trees
    index = 0

    # Save trees in an array to give to output TREE_ARRAY
    trees = (TREE * num_trees)()

    # If leaf label dict is needed, see the dtt-package or Summarizing-ran... repository!

    # Regex to delete additional data in []
    brackets = re.compile(r'\[[^\]]*\]')

    with open(file_handle, 'r') as f:
        # Read trees
        for line in f:
            if re_tree.match(line):
                # First extract the newick string and then delete everything after the last occurence of ')'
                tree_string = f'{re.split(re_tree, line)[1][:re.split(re_tree, line)[1].rfind(")")+1]};'
                t = read_newick(re.sub(brackets, "", tree_string), factor)
                if t != 1:
                    trees[index] = t
                else:
                    print(
                        "Couldn't read all trees in file, choose higher value for 'factor'.")
                    return (1)
                index += 1

    return TREE_ARRAY(trees, num_trees)


def read_from_cluster(s):
    # Note that we assume that leafs are labelled by integers
    # Read a tree from a string s that is the cluster representation of the tree (with times)
    clusters = s.split("{")
    leaf_pattern = re.compile(r'([^,^}^:]*)[,}\]]')
    num_leaves = len(clusters)
    # save highest ancestor for leaf i that we already found at position i
    highest_ancestor = []

    # We are now ready to use our information to create a tree in the C data structure
    num_nodes = 2*num_leaves-1
    node_array = (NODE * num_nodes)()

    # empty child array for initialising the node_array.
    empty_children = (c_long * 2)()
    empty_children[0] = -1
    empty_children[1] = -1

    # Initialise Node list
    for i in range(0, num_nodes):
        node_array[i] = NODE(-1, empty_children, 0)
        if i >= num_leaves:
            node_array[i].time = i - (num_leaves-1)

    for i in range(0, num_leaves):
        highest_ancestor.append(i)
    for i in range(1, num_leaves):
        m = leaf_pattern.findall(clusters[i])
        child1 = -1
        child2 = -1
        # loop through elements in clusters (last element in m is time of cluster)
        for k in range(0, len(m)-1):
            leaf = m[k]
            leaf_index = int(leaf)-1
            if child1 == -1:
                child1 = highest_ancestor[leaf_index]
            elif child1 != highest_ancestor[leaf_index]:
                child2 = highest_ancestor[leaf_index]
            highest_ancestor[leaf_index] = i+num_leaves-1
        node_array[i+num_leaves-1].children[0] = child1
        node_array[i+num_leaves-1].children[1] = child2
        node_array[child1].parent = i+num_leaves-1
        node_array[child2].parent = i+num_leaves-1
        node_array[i+num_leaves-1].time = int(m[-1])

    output_tree = TREE(node_array, num_leaves,
                       node_array[num_nodes - 1].time, -1)
    return output_tree


def tree_to_cluster_string(tree):
    # return tree as string in cluster representation (for testing)
    num_leaves = tree.num_leaves
    num_nodes = 2 * num_leaves - 1
    # pos i in list: string containing all leaves descending from node at rank i, separated by ","
    cluster_list = list()
    times = list()  # save time of node with rank i at position i
    for i in range(0, num_leaves-1):
        cluster_list.append("")
    # fill cluster list
    for i in range(num_leaves, num_nodes):
        times.append(tree.node_array[i].time)
        for child_index in [0, 1]:
            if tree.node_array[i].children[child_index] < num_leaves:
                cluster_list[i - num_leaves] += str(
                    tree.node_array[i].children[child_index]+1)
            else:
                cluster_list[i - num_leaves] += cluster_list[tree.node_array[i]
                                                             .children[child_index] - num_leaves]
            cluster_list[i - num_leaves] += ","

    # iteratively build tree_str from cluster_lust
    tree_str = "["
    for i in range(0, num_leaves-1):
        tree_str += "{"
        cluster = cluster_list[i].split(",")
        cluster.sort()
        for leaf in cluster:
            if len(leaf) > 0:
                tree_str += leaf + ","
        tree_str = tree_str[:len(tree_str)-1]
        tree_str += "}:" + str(times[i]) + ","
    tree_str = tree_str[:-1]  # del last ","
    tree_str += "]"
    return tree_str
