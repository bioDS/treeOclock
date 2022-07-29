__author__ = 'Lena Collienne, Jordan Kettles'

import os
from ctypes import *

lib = CDLL(f'{os.path.dirname(os.path.realpath(__file__))}/tree.so')

class NODE(Structure):
    _fields_ = [('parent', c_long), ('children', c_long * 2), ('time', c_long)] # The order of arguments here matters! Needs to be the same as in C code!
    def __init_(self, parent, children, time):
        self.parent = parent
        self.children = children
        self.time = time


class TREE(Structure):
    _fields_ =  [('tree', POINTER(NODE)), ('num_leaves', c_long), ('root_time', c_long), ('sos_d', c_long)] # Everything from struct definition in C
    def __init_(self, tree, num_leaves, root_time, sos_d):
        self.tree = tree
        self.num_leaves = num_leaves
        self.root_time = root_time
        self.sos_d = sos_d


class TREE_LIST(Structure):
    _fields_ = [('trees', POINTER(TREE)), ('num_trees', c_long), ('max_root_time', c_long)]
    def __init_(self, trees, num_trees, max_root_time):
        self.trees = trees
        self.num_trees = num_trees
        self.max_root_time = max_root_time


class TREE_PATH(Structure):
    _fields_ = [('trees', POINTER(TREE)), ('num_trees', c_long), ('length', c_long), ('tree_positions', POINTER(c_long))]
    def __init_(self, trees, num_trees, length, tree_positions):
        self.trees = trees
        self.num_trees = num_trees
        self.length = length
        self.tree_positions = tree_position


tree_to_cluster_string = lib.tree_to_string
tree_to_cluster_string.argtypes = [POINTER(TREE)]
tree_to_cluster_string.restype = c_char_p

same_topology = lib.same_topology
same_topology.argtypes = [POINTER(TREE), POINTER(TREE)]
same_topology.restype = c_int

mrca_differences = lib.mrca_differences
mrca_differences.argtypes = [POINTER(TREE), POINTER(TREE)]
mrca_differences.restype = c_long

sum_symmetric_cluster_diff = lib.sum_symmetric_cluster_diff
sum_symmetric_cluster_diff.argtypes = [POINTER(TREE), POINTER(TREE)]
sum_symmetric_cluster_diff.restype = c_long

shortest_rank_path = lib.shortest_rank_path
shortest_rank_path.argtypes = [POINTER(TREE), POINTER(TREE)]
shortest_rank_path.restype = c_long

symmetric_cluster_diff = lib.symmetric_cluster_diff
symmetric_cluster_diff.argtypes = [POINTER(TREE), POINTER(TREE), c_long]
symmetric_cluster_diff.restype = c_long

rnni_neighbourhood = lib.rnni_neighbourhood
rnni_neighbourhood.argtypes = [POINTER(TREE)]
rnni_neighbourhood.restype = TREE_LIST

findpath_distance = lib.findpath_distance
findpath_distance.argtypes = [POINTER(TREE), POINTER(TREE)]
findpath_distance.restype = c_long

findpath = lib.findpath
findpath.argtypes = [POINTER(TREE), POINTER(TREE)]
findpath.restype = TREE_PATH

findpath_after_x_percent_tree = lib.findpath_after_x_percent_tree
findpath_after_x_percent_tree.argtypes = [POINTER(TREE), POINTER(TREE), c_float]
findpath_after_x_percent_tree.restype = TREE

random_walk = lib.random_walk
random_walk.argtypes = [POINTER(TREE), c_long]
random_walk.restype = c_long

mrca_list = lib.mrca_list
mrca_list.argtypes = [POINTER(TREE), POINTER(TREE)]
mrca_list.restype = POINTER(c_long)

decrease_mrca = lib.decrease_mrca
decrease_mrca.argtypes = [POINTER(TREE), c_long, c_long]
decrease_mrca.restype = c_int

same_tree = lib.same_tree
same_tree.argtypes = [POINTER(TREE), POINTER(TREE)]
same_tree.restype = c_int

mrca = lib.mrca
mrca.argtypes = [POINTER(TREE), c_long, c_long]
mrca.restype = c_long

first_iteration = lib.first_iteration
first_iteration.argtypes = [POINTER(TREE_LIST), c_long, c_long, c_long]
first_iteration.restype = c_int

sos = lib.sos
sos.argtypes = [POINTER(TREE_LIST), POINTER(TREE)]
sos.restype = c_long