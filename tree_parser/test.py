
from tree_io import *


def test_read_newick():
    tree_str = "(((A:1,B:1):2,(C:2,D:2):1):1,E:4);"
    tree = read_newick(tree_str)
    if tree_to_cluster_string(tree) == "[{1,2}{3,4}{1,2,3,4}{1,2,3,4,5}]":
        return True


if __name__ == "__main__":
    if test_read_newick():
        print("Test tree read correctly.")
