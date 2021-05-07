import ete3


from treeoclock.trees.time_trees import TimeTree, findpath_distance
from treeoclock.trees._converter import ete3_to_ctree


def test_fp_distance(five_taxa_newick_list, five_taxa_newick_list_distances):
    out = []
    t = [TimeTree(i) for i in five_taxa_newick_list]
    for i in t:
        out.extend([i.fp_distance(j) for j in t])
    assert out == five_taxa_newick_list_distances, f'fp_distance wrong {out}'


def test_fp_path(five_taxa_newick_list, five_taxa_newick_list_distances):
    out = []
    t = [TimeTree(i) for i in five_taxa_newick_list]
    for i in t:
        out.extend([len(i.fp_path(j)) for j in t])
    assert out == five_taxa_newick_list_distances, f"fp_path distances wrong {out}"


def test_get_newick(five_taxa_newick_list):
    t = [TimeTree(i) for i in five_taxa_newick_list]
    out = []
    for i in t:
        out.append(i.get_newick())
    assert out == five_taxa_newick_list, "TimeTree does not return correct newick strings"


def test_findpath_distance_timetree(five_taxa_newick_list, five_taxa_newick_list_distances):
    t = [TimeTree(i) for i in five_taxa_newick_list]
    out = []
    for i in t:
        for j in t:
            out.append(findpath_distance(i, j))
    assert out == five_taxa_newick_list_distances, f"findpath_distances for TimeTree class wrong {out}"


def test_findpath_distance_ctree(five_taxa_newick_list, five_taxa_newick_list_distances):
    t = [ete3.Tree(i) for i in five_taxa_newick_list]
    out = []
    for i in t:
        for j in t:
            out.append(findpath_distance(i, j))
    assert out == five_taxa_newick_list_distances, f"findpath_distances for ete3.Tree class wrong {out}"


def test_findpath_distance_ete3tree(five_taxa_newick_list, five_taxa_newick_list_distances):
    t = [ete3_to_ctree(ete3.Tree(i)) for i in five_taxa_newick_list]
    out = []
    for i in t:
        for j in t:
            out.append(findpath_distance(i, j))
    assert out == five_taxa_newick_list_distances, f"findpath_distances for c-TREE class wrong {out}"
