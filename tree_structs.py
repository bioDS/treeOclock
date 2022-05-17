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


# C function tree_to_string (for testing purposes)
tree_to_cluster_string = lib.tree_to_string
tree_to_cluster_string.argtypes = [POINTER(TREE)]
tree_to_cluster_string.restype = c_char_p

# C function spr_move (for testing purposes)
spr_move = lib.spr_move
spr_move.argtypes = [POINTER(TREE), c_long, c_long, c_int]
spr_move.restype = c_int

# C function to compute sum of mrca ranks (for testing)
mrca_differences = lib.mrca_differences
mrca_differences.argtypes = [POINTER(TREE), POINTER(TREE)]
mrca_differences.restype = c_long

# C function to compute paths in rankedspr
rankedspr_path_mrca_diff = lib.rankedspr_path_mrca_diff
rankedspr_path_mrca_diff.argtypes = [POINTER(TREE), POINTER(TREE)]
rankedspr_path_mrca_diff.restype = TREE_LIST

# C function to compute paths in rankedspr
rankedspr_path_mrca_cluster_diff = lib.rankedspr_path_mrca_cluster_diff
rankedspr_path_mrca_cluster_diff.argtypes = [POINTER(TREE), POINTER(TREE)]
rankedspr_path_mrca_cluster_diff.restype = TREE_LIST

# C function to compute paths in rankedspr
rankedspr_path_restricting_neighbourhood = lib.rankedspr_path_restricting_neighbourhood
rankedspr_path_restricting_neighbourhood.argtypes = [POINTER(TREE), POINTER(TREE), c_long]
rankedspr_path_restricting_neighbourhood.restype = c_long

# C function to compute symmetric cluster differences (for testing)
symmetric_cluster_diff = lib.symmetric_cluster_diff
symmetric_cluster_diff.argtypes = [POINTER(TREE), POINTER(TREE)]
symmetric_cluster_diff.restype = c_long

# C function spr_neighbourhood (for testing purposes)
spr_neighbourhood = lib.spr_neighbourhood
spr_neighbourhood.argtypes = [POINTER(TREE)]
spr_neighbourhood.restype = TREE_LIST

# C function spr_neighbourhood (for testing purposes)
all_spr_neighbourhood = lib.all_spr_neighbourhood
all_spr_neighbourhood.argtypes = [POINTER(TREE)]
all_spr_neighbourhood.restype = TREE_LIST

# C function spr_neighbourhood (for testing purposes)
hspr_neighbourhood = lib.hspr_neighbourhood
hspr_neighbourhood.argtypes = [POINTER(TREE)]
hspr_neighbourhood.restype = TREE_LIST

# C function calculating size of symmetric cluster difference
symm_cluster_diff = lib.symm_cluster_diff
symm_cluster_diff.argtypes = [POINTER(TREE), POINTER(TREE), c_long]
symm_cluster_diff.restype = c_long

# C function approximating shortest path in top-down fashion
rankedspr_path_top_down_symm_diff = lib.rankedspr_path_top_down_symm_diff
rankedspr_path_top_down_symm_diff.argtypes = [POINTER(TREE), POINTER(TREE)]
rankedspr_path_top_down_symm_diff.restype = c_long

# C function findpath_distance
findpath_distance = lib.findpath_distance
findpath_distance.argtypes = [POINTER(TREE), POINTER(TREE)]
findpath_distance.restype = c_long

# C function return_findpath
findpath_path = lib.return_findpath
findpath_path.argtypes = [POINTER(TREE), POINTER(TREE)]
findpath_path.restype = TREE_LIST

# C function random_walk
random_walk = lib.random_walk
random_walk.argtypes = [POINTER(TREE), c_long]
random_walk.restype = c_long