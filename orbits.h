#ifndef ORBITS_H_
#define ORBITS_H_

#include "tree.h"

long* orbit_sizes(Tree tree, long k);

void free_results_array(long* array);

#endif