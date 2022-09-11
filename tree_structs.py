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
    _fields_ =  [('node_array', POINTER(NODE)), ('num_leaves', c_long), ('root_time', c_long), ('sos_d', c_long)] # Everything from struct definition in C
    def __init_(self, node_array, num_leaves, root_time, sos_d):
        self.node_array = node_array
        self.num_leaves = num_leaves
        self.root_time = root_time
        self.sos_d = sos_d


class TREE_ARRAY(Structure):
    _fields_ = [('trees', POINTER(TREE)), ('num_trees', c_long), ('max_root_time', c_long)]
    def __init_(self, trees, num_trees, max_root_time):
        self.trees = trees
        self.num_trees = num_trees
        self.max_root_time = max_root_time


unlabelled_rnni_neighbourhood = lib.unlabelled_rnni_neighbourhood
unlabelled_rnni_neighbourhood.argtypes = [POINTER(TREE)]
unlabelled_rnni_neighbourhood.restype = TREE_ARRAY

rank_neighbourhood = lib.rank_neighbourhood
rank_neighbourhood.argtypes = [POINTER(TREE)]
rank_neighbourhood.restype = TREE_ARRAY

nni_move = lib.nni_move
nni_move.argtypes = [POINTER(TREE), c_long, c_int]
nni_move.restype = c_int

rank_move = lib.rank_move
rank_move.argtypes = [POINTER(TREE), c_long]
rank_move.restype = c_int

print_tree = lib.print_tree
print_tree.argtypes = [POINTER(TREE)]
print_tree.restype = c_int

spr_move = lib.spr_move
spr_move.argtypes = [POINTER(TREE), c_long, c_long, c_int]
spr_move.restype = c_int

unlabelled_spr_move = lib.unlabelled_spr_move
unlabelled_spr_move.argtypes = [POINTER(TREE), c_long, c_long, c_int, c_int]
unlabelled_spr_move.restype = c_int

same_topology = lib.same_topology
same_topology.argtypes = [POINTER(TREE), POINTER(TREE)]
same_topology.restype = c_int

mrca_differences = lib.mrca_differences
mrca_differences.argtypes = [POINTER(TREE), POINTER(TREE)]
mrca_differences.restype = c_long

spr_neighbourhood = lib.spr_neighbourhood
spr_neighbourhood.argtypes = [POINTER(TREE)]
spr_neighbourhood.restype = TREE_ARRAY

all_spr_neighbourhood = lib.all_spr_neighbourhood
all_spr_neighbourhood.argtypes = [POINTER(TREE), c_int]
all_spr_neighbourhood.restype = TREE_ARRAY

shortest_rank_path = lib.shortest_rank_path
shortest_rank_path.argtypes = [POINTER(TREE), POINTER(TREE)]
shortest_rank_path.restype = c_long

hspr_neighbourhood = lib.hspr_neighbourhood
hspr_neighbourhood.argtypes = [POINTER(TREE)]
hspr_neighbourhood.restype = TREE_ARRAY

symmetric_cluster_diff = lib.symmetric_cluster_diff
symmetric_cluster_diff.argtypes = [POINTER(TREE), POINTER(TREE), c_long]
symmetric_cluster_diff.restype = c_long

rnni_neighbourhood = lib.rnni_neighbourhood
rnni_neighbourhood.argtypes = [POINTER(TREE)]
rnni_neighbourhood.restype = TREE_ARRAY

rnni_distance = lib.rnni_distance
rnni_distance.argtypes = [POINTER(TREE), POINTER(TREE)]
rnni_distance.restype = c_long

findpath_path = lib.return_findpath
findpath_path.argtypes = [POINTER(TREE), POINTER(TREE)]
findpath_path.restype = TREE_ARRAY

random_walk = lib.random_walk
random_walk.argtypes = [POINTER(TREE), c_long]
random_walk.restype = c_long

mrca_array = lib.mrca_array
mrca_array.argtypes = [POINTER(TREE), POINTER(TREE)]
mrca_array.restype = POINTER(c_long)

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
first_iteration.argtypes = [POINTER(TREE_ARRAY), c_long, c_long, c_long]
first_iteration.restype = c_int

sos = lib.sos
sos.argtypes = [POINTER(TREE_ARRAY), POINTER(TREE)]
sos.restype = c_long