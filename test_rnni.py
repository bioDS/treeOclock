from tree_parser.tree_io import *
from tree_functions import *


def test_rnni_distance():
    tree1 = read_newick("(((A:1,B:1):2,(C:2,D:2):1):1,E:4);")
    tree2 = read_newick("((((C:1,E:1):1,B:2):1,A:3):1,D:4);")
    if rnni_distance(tree1, tree2) != 5:
        return False
    return True


def test_findpath():
    tree1 = read_newick("(((A:1,B:1):2,(C:2,D:2):1):1,E:4);")
    tree2 = read_newick("((C:1,D:1):3,((B:2,E:2):1,A:3):1);")
    fp = findpath(tree1, tree2)
    if tree_to_cluster_string(fp.trees[0]) != tree_to_cluster_string(tree1):
        return False
    if tree_to_cluster_string(
            fp.trees[1]) != "[{3,4}:1,{1,2}:2,{1,2,3,4}:3,{1,2,3,4,5}:4]":
        return False
    if tree_to_cluster_string(
            fp.trees[2]) != "[{3,4}:1,{1,2}:2,{1,2,5}:3,{1,2,3,4,5}:4]":
        return False
    if tree_to_cluster_string(fp.trees[3]) != tree_to_cluster_string(tree2):
        print(tree_to_cluster_string(fp.trees[3]),
              tree_to_cluster_string(tree2))
        return False
    return True


def test_dct_distance():
    tree1 = read_newick("(((A:1,B:1):1,C:2):5,(D:5,E:5):2);", factor = 1)
    tree2 = read_newick("((B:1,E:1):5,((A:3,D:3):2,C:5):1);", factor = 1)
    dist = rnni_distance(tree1, tree2)
    if dist == 14:
        return True
    else:
        return False

if __name__ == "__main__":
    if test_rnni_distance():
        print("rnni_distance() computed correctly.")
    else:
        print("Error computing rnni_distance()")
    if test_findpath():
        print("findpath() computed correctly")
    else:
        print("Error computing findpath()")
    if test_dct_distance():
        print("rnni_distance() for DCT trees computed correctly.")
    else:
        print("Error computing rnni_distance() for DCT trees")
