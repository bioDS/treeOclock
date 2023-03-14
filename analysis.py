from generate import *

from tree_functions import *
from orbits import all_destinations_findpath_orbit_sizes_cached, all_destinations_findpath_orbit_sizes_cached_cache
from unlabelled import unlabelled_distance_hybrid
import math

def all_orbit_sizes(n):
    orbit_sizes_list = []
    for tree in all_unlabelled_trees(n):
        orbit_sizes = all_destinations_findpath_orbit_sizes_cached(tree)
        orbit_sizes_list.append(orbit_sizes)
    return orbit_sizes_list

def balls_from_orbits(orbit_sizes):
    ball_sizes = [1]
    for i in range(1, len(orbit_sizes)):
        ball_sizes.append(ball_sizes[i-1]+orbit_sizes[i])
    return ball_sizes

def max_orbit_size(n, orbit_sizes_list):
    max_orbit = 0
    max_indices = [0]
    max_pos=[0]
    for i in range (len(orbit_sizes_list)):
        for j in range(len(orbit_sizes_list[i])):
            cand = orbit_sizes_list[i][j]
            if cand > max_orbit:
                max_orbit = cand
                max_indices = [i]
                max_pos = [j]
            elif cand == max_orbit_size:
                max_indices.append(i)
                max_pos.append(j)
    
    shapes = list(all_shapes(n))
    max_tops = [shapes[i] for i in max_indices]

    return max_orbit, max_tops, max_pos


def verify_orbits(n, orbit_sizes_list):
    assert len(orbit_sizes_list[0]) == ((n-1)*(n-2)//2)+1
    num_shapes = len(orbit_sizes_list)
    threshold1 = (n-1)*(n-2)//3
    threshold2 = math.ceil((n-1)*(n-2)*3/8)
    diameter = (n-1)*(n-2)//2

    for i in range(1, num_shapes):
        for k in range(1, threshold1+1):
            if orbit_sizes_list[i][k] >= orbit_sizes_list[0][k]:
                return False
        for k in range(threshold2, diameter+1):
            if orbit_sizes_list[i][k] <= orbit_sizes_list[0][k]:
                return False
    return True

def verify_balls(n, ball_sizes_list):
    num_shapes = len(ball_sizes_list)
    diameter = (n-1)*(n-2)//2
    assert len(ball_sizes_list[0]) == diameter+1

    for i in range(1, num_shapes):
        for k in range(1, diameter):
            if ball_sizes_list[i][k] >= ball_sizes_list[0][k]:
                return False
        assert ball_sizes_list[i][diameter] == ball_sizes_list[0][diameter]
    return True

def verify_orbits_and_balls(start, end):
    for n in range(start, end+1):
        print(f"Generating n={n}...", end="", flush=True)
        orbit_sizes_list = all_orbit_sizes(n)
        print(f"Done. {len(orbit_sizes_list)} shapes.", flush=True)

        if n==end:
            all_destinations_findpath_orbit_sizes_cached_cache.clear()

        print("Checking orbits...", end="", flush=True)
        assert verify_orbits(n, orbit_sizes_list)
        print(" Done.", flush=True)

        print(f"Computing balls...", end="", flush=True)
        ball_sizes_list = [balls_from_orbits(orbit_sizes) for orbit_sizes in orbit_sizes_list]
        print(" Done.", flush=True)

        print("Checking balls...", end="", flush=True)
        assert verify_balls(n, ball_sizes_list)
        print(" Done.", flush=True)

        print("Computing max orbit...",flush=True)
        print(max_orbit_size(n, orbit_sizes_list))
        print("Done.", flush=True)


        print(flush=True)

def verify_unlabelled_diameter(n):
    print(f"Generating tree shapes n={n}...", end="", flush=True)
    trees1 = list(all_unlabelled_trees(n))
    num_shapes = len(trees1)
    trees2 = [get_empty_tree(n) for i in range(num_shapes)]
    for i in range(num_shapes):
        copy_tree(trees2[i], trees1[i])
    print(f"Done. {num_shapes} shapes.", flush=True)

    expected_max = sum(min(i, n-2-i) for i in range(1, n-2))
    actual_max=0
    print("Computing Distances...", flush=True)
    for i in range(1, num_shapes):
        for j in range(i):
            dist = unlabelled_distance_hybrid(trees1[i], trees2[j], upper_bound=expected_max)
            actual_max = max(actual_max, dist)
        #     print(dist, end=" ", flush=True)
        # print(flush=True)
        print(f"{i}/{num_shapes}", flush=True)
    
    is_maximal = (unlabelled_distance_hybrid(trees1[0], trees2[-1]) == actual_max)
    print("Done.", flush=True)

    for i in range(num_shapes):
        free_tree(trees1[i])
        free_tree(trees2[i])
    
    assert expected_max == actual_max
    assert is_maximal
    



if __name__ == "__main__":
    verify_orbits_and_balls(3, 10)

    # for n in range(3, 10+1):
    #     verify_unlabelled_diameter(n)
    #     print()
