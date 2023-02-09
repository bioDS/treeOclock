__author__ = "Joseph Grace"

from tree_functions import *
from external_functions import num_unlabelled_trees, print_tree_from_root
from helper_functions import max_remaining_reverse_steps

from generate import all_unlabelled_trees
from orbits import all_destinations_findpath_orbit_sizes_cached, shape

import random


# Samplers

def uniform_sample_orbit(tree, k):
    """
    Returns a tree uniformly sampled from the orbit at distance k from tree.
    Only works efficiently for trees similar to catepillar trees in their higher ranks.
    """
    n = tree.contents.num_leaves
    nodes = tree.contents.node_array
    sampled_tree = get_empty_tree(n)

    if k > (n-1)*(n-2)//2: return None

    def get_count(min_rank, cur_rank, d):
        delta = all_destinations_findpath_orbit_sizes_cached(tree, min_rank, cur_rank, d)
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

def uniform_sample_ball(tree, k):
    """
    Returns a tree uniformly sampled from the k-ball of tree.
    Only works efficiently for trees similar to catepillar trees in their higher ranks.
    """
    n = tree.contents.num_leaves
    nodes = tree.contents.node_array
    sampled_tree = get_empty_tree(n)

    def get_count(min_rank, cur_rank, d):
        delta = all_destinations_findpath_orbit_sizes_cached(tree, min_rank, cur_rank, d)
        return sum(delta[:k-d+1])

    def rec(min_rank, cur_rank, d):
        
        if cur_rank == min_rank:
            choices = list(range(min_rank+2, n))
            counts = []
            for new_cur_rank in choices:
                counts.append(get_count(min_rank+1, new_cur_rank, d))
            
            choices.append(None)
            counts.append(1)

            option = random.choices(choices, k=1, weights=counts)[0]
            if option is None:
                copy_tree(sampled_tree, tree)
            else:
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

                choices.append(None)
                counts.append(1)

                # Sample & Reset
                option = random.choices(choices, k=1, weights=counts)[0]

                if option is None:
                    copy_tree(sampled_tree, tree)
                else:
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
                
                choices.append(None)
                counts.append(1)

                #Sample
                option = random.choices(choices, k=1, weights=counts)[0]

                if option is None:
                    # Reset
                    rank_move(tree, r)

                    copy_tree(sampled_tree, tree)
                else:
                    rec(*option)

                    # Reset
                    rank_move(tree, r)

    rec(0, 0, 0)
    return sampled_tree

def generate_orbit(tree, k):
    """
    Returns a generator over the entire k-orbit centered at tree.
    Runs in O(size of k-ball) time and O(k) space.
    """
    n = tree.contents.num_leaves
    nodes = tree.contents.node_array

    def rec(max_rank, cur_rank, d):
        if d == k:
            new_tree = get_empty_tree(n)
            copy_tree(new_tree, tree)
            yield new_tree
            return

        if max_rank - 1 > 0 and max_remaining_reverse_steps(n, max_rank-1, max_rank-1) >= k-d:
            yield from rec(max_rank-1, max_rank-1, d)

        
        if cur_rank < n-1 and max_remaining_reverse_steps(n, max_rank, cur_rank+1) >= k-(d+1):
            # Node can be moved up
            r = cur_rank + n - 1

            if nodes[r].parent==r+1:
                # NNI Moves

                nni_move(tree, r, 0)
                yield from rec(max_rank, cur_rank+1, d+1)
                
                nni_move(tree, r, 1)
                yield from rec(max_rank, cur_rank+1, d+1)
                
                # Reset
                nni_move(tree, r, 0)
                nodes[r].children[0], nodes[r].children[1] = nodes[r].children[1], nodes[r].children[0]

            else:
                # Rank Move

                rank_move(tree, r)
                yield from rec(max_rank, cur_rank+1, d+1)
                
                # Reset
                rank_move(tree, r)

    yield from rec(n-2, n-2, 0)
