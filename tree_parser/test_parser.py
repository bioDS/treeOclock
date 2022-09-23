from tree_io import *


def test_read_newick():
    tree_str = "(((A:1,B:1):2,(C:2,D:2):1):1,E:4);"
    tree = read_newick(tree_str)
    print(tree_to_cluster_string(tree))
    if tree_to_cluster_string(tree) == \
            "[{1,2}:1,{3,4}:2,{1,2,3,4}:3,{1,2,3,4,5}:4]":
        return True
    return False


if __name__ == "__main__":
    if test_read_newick():
        print("Test tree read correctly.")
    else:
        print("Error reading test tree.")
