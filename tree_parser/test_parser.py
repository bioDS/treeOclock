from tree_io import *


def test_read_newick():
    tree_str = "(((A:1,B:1):2,(C:2,D:2):1):1,E:4);"
    tree = read_newick(tree_str)

    if tree_to_cluster_string(tree) == \
            "[{1,2}:1,{3,4}:2,{1,2,3,4}:3,{1,2,3,4,5}:4]":
        return True
    print("Newick tree " + tree_str + " read incorrectly.")
    return False


def test_read_newick_dct():
    tree1_str = "(((A:1,B:1):1,C:2):5,(D:5,E:5):2);"
    tree2_str = "((B:2,E:2):4,((A:3,D:3):2,C:5):1);"
    tree1 = read_newick(tree1_str, factor = 1)
    tree2 = read_newick(tree2_str, factor = 1)

    if tree_to_cluster_string(tree1) != \
            "[{1,2}:1,{1,2,3}:2,{4,5}:5,{1,2,3,4,5}:7]":
        print("Newick tree " + tree1_str +" read incorrectly.")
        return False

    if  tree_to_cluster_string(tree2) == \
            "[{2,5}:2,{1,4}:3,{1,3,4}:5,{1,2,3,4,5}:6]":
        return True
    else:
        print("Newick tree " + tree2_str +" read incorrectly.")
        return False


if __name__ == "__main__":
    if test_read_newick():
        print("Test tree read correctly.")
    else:
        print("Error reading test tree.")

    if test_read_newick_dct():
        print("Test DCT tree read correctly.")
    else:
        print("Error reading DCT test tree.")