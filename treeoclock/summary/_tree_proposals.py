from treeoclock.trees.time_trees import TimeTree, TimeTreeSet
from treeoclock.summary.compute_sos import compute_sos_mt
from treeoclock.summary._constants import SELECT_LIST

from random import choice


class NoBetterNeighbourFound(Exception):
    """Raised when no neighbour has a better SOS value"""
    pass


def intelligent_neighbourhood(t: TimeTree, clades):

    neighbours_int = []

    # sample all NNI neighbours that do contain all clades
    nn = t.nni_neighbours()
    for i in nn:
        cur_clades = i.get_clades()
        if all(x in cur_clades for x in clades):
            neighbours_int.append(i)

    # only compute rank neighbours if t contains all clades
    # Todo think about this, does it actually make sense?
    tclades = t.get_clades()
    if all(x in tclades for x in clades):
        neighbours_int.extend(t.rank_neighbours())

    return neighbours_int


def intelligent_neighbourhood_separate(t: TimeTree, clades, rank):

    neighbours_int = []

    if not rank:
        # sample all NNI neighbours that do contain all clades
        nn = t.nni_neighbours()
        for i in nn:
            cur_clades = i.get_clades()
            if all(x in cur_clades for x in clades):
                neighbours_int.append(i)

    else:
        # only compute rank neighbours if t contains all clades
        tclades = t.get_clades()
        if all(x in tclades for x in clades):
            neighbours_int.extend(t.rank_neighbours())

    return neighbours_int


def search_neighbourhood_greedy(t: TimeTree, trees: TimeTreeSet, t_value: int, n_cores=None, select='first'):

    if select not in SELECT_LIST:
        raise ValueError(f"The 'select' parameter should be"
                         f" in {SELECT_LIST} but {select} was given.")

    trees.get_common_clades()

    neighbourhood = intelligent_neighbourhood(t, trees.common_clades)

    better_neighbours = []
    cur_best = t_value
    for n in neighbourhood:
        sos = compute_sos_mt(t=n, trees=trees, n_cores=n_cores)
        if sos < cur_best:
            cur_best = sos
            better_neighbours = [n]
        elif sos == cur_best and sos < t_value:
            better_neighbours.append(n)

    if len(better_neighbours) == 0:
        # No better neighbour was found
        raise NoBetterNeighbourFound
    elif len(better_neighbours) == 1:
        # Only one neighbour with smallest value found
        return better_neighbours[0], cur_best

    # print("Multiple choices!")
    # multiple neighbours with same SOS value found, choosing by the given selection method
    if select == 'first':
        return better_neighbours[0], cur_best
    elif select == 'last':
        return better_neighbours[-1], cur_best
    elif select == 'random':
        return choice(better_neighbours), cur_best


def search_neighbourhood_separate(t: TimeTree, trees: TimeTreeSet, t_value: int, n_cores=None, select='first', rank=False):

    if select not in SELECT_LIST:
        raise ValueError(f"The 'select' parameter should be"
                         f" in {SELECT_LIST} but {select} was given.")

    trees.get_common_clades()

    neighbourhood = intelligent_neighbourhood_separate(t, trees.common_clades, rank)

    better_neighbours = []
    cur_best = t_value
    for n in neighbourhood:
        sos = compute_sos_mt(t=n, trees=trees, n_cores=n_cores)
        if sos < cur_best:
            cur_best = sos
            better_neighbours = [n]
        elif sos == cur_best and sos < t_value:
            better_neighbours.append(n)

    if len(better_neighbours) == 0:
        # No better neighbour was found
        raise NoBetterNeighbourFound
    elif len(better_neighbours) == 1:
        # Only one neighbour with smallest value found
        return better_neighbours[0], cur_best

    # multiple neighbours with same SOS value found, choosing by the given selection method
    if select == 'first':
        return better_neighbours[0], cur_best
    elif select == 'last':
        return better_neighbours[-1], cur_best
    elif select == 'random':
        return choice(better_neighbours), cur_best