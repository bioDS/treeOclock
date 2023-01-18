#ifndef ORBITS_H_
#define ORBITS_H_

#include "tree.h"
#include "types.h"

#include <vector>

std::vector<count_t> all_orbit_sizes(Tree* tree, int min_rank, int cur_rank);

std::vector<count_t> all_orbit_sizes(Tree* tree);

#endif