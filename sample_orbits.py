__author__ = "Joseph Grace"

from tree_functions import *
from external_functions import num_unlabelled_trees, print_tree_from_root

from generate import all_unlabelled_trees
from orbits import findpath_orbits_cached, shape

import random


def uniform_sample(tree, k):
    """
    Returns a tree uniformly sampled from the orbit at distance k from tree.
    Only works efficiently for trees similar to catepillar trees.
    """
    n = tree.contents.num_leaves
    nodes = tree.contents.node_array
    sampled_tree = get_empty_tree(n)

    if k > (n-1)*(n-2)//2: return None

    def get_count(min_rank, cur_rank, d):
        delta = findpath_orbits_cached(tree, min_rank, cur_rank, d)
        if k-d >= len(delta):
            return 0
        else:
            return delta[k-d]

    def rec(min_rank, cur_rank, d):
        if d == k:
            copy_tree(sampled_tree, tree)
            return

        
        if cur_rank == min_rank:
            choices = range(min_rank+2, n)
            counts = []
            for new_cur_rank in choices:
                counts.append(get_count(min_rank+1, new_cur_rank, d))
                
            option = random.choices(choices, k=1, weights=counts)[0]
            rec(min_rank+1, option, d)

        else:
            cur_index = cur_rank + n - 1
            r = cur_index - 1
            if nodes[r].parent == cur_index:
                # NNI Moves
                nni_move(tree, r, 0)

                choices = [
                    (0, (min_rank, cur_rank-1, d+1)), (0, (cur_rank-1, cur_rank-1, d+1)),
                    (1, (min_rank, cur_rank-1, d+1)), (1, (cur_rank-1, cur_rank-1, d+1))]
                counts = [0] * 4

                counts[0] = get_count(min_rank, cur_rank-1, d+1)
                if cur_rank - 1 > min_rank:
                    counts[1] = get_count(cur_rank-1, cur_rank-1, d+1)

                nni_move(tree, r, 1)

                counts[2] = get_count(min_rank, cur_rank-1, d+1)
                if cur_rank - 1 > min_rank:
                    counts[3] = get_count(cur_rank-1, cur_rank-1, d+1)

                # Reset
                nni_move(tree, r, 0)
                nodes[r].children[0], nodes[r].children[1] = nodes[r].children[1], nodes[r].children[0]

                # Sample & Reset
                option = random.choices(choices, k=1, weights=counts)[0]

                nni_move(tree, r, option[0])
                rec(*(option[1]))
                nni_move(tree, r, option[0])
                
            else:
                # Rank Move
                rank_move(tree, r)

                choices = [(min_rank, cur_rank-1, d+1),(cur_rank-1, cur_rank-1, d+1)]
                counts = [0] * 2

                counts[0] = get_count(min_rank, cur_rank-1, d+1)
                if cur_rank - 1 > min_rank:
                    counts[1] = get_count(cur_rank-1, cur_rank-1, d+1)
                
                #Sample
                option = random.choices(choices, k=1, weights=counts)[0]
                rec(*option)

                # Reset
                rank_move(tree, r)

    rec(0, 0, 0)
    return sampled_tree

if __name__ == "__main__":
    n=20
    num_samples = 25
    k = (n-1)*(n-2)//3
    initial_d = 2

    cat = list(all_unlabelled_trees(n, 1))[0]
    center = uniform_sample(cat, initial_d)
    free_tree(cat)

    print(f"Center Tree: {print_tree_from_root(center.contents, 2*n-2)};\n{shape(center)}\n")
    occurences = {}
    for i in range(num_samples):
        sampled = uniform_sample(center, k)
        rep = print_tree_from_root(sampled.contents, 2*n-2)
        occurences[rep] = occurences.get(rep, 0) + 1
        print(print_tree_from_root(sampled.contents, 2*n-2))
        free_tree(sampled)
    
    for rep, count in sorted(occurences.items()):
        print(rep, count)
    print()
    print(f"{len(occurences)}/{num_samples} unique samples at distance {k} from {print_tree_from_root(center.contents, 2*n-2)}.")
    print(shape(center))

    free_tree(center)