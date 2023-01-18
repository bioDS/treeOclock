#ifndef GENERATE_H_
#define GENERATE_H_

#include "tree.h"

#include "types.h"

#include <vector>

std::vector<std::vector<int>> all_top_rep(int n, limit_t limit);

std::vector<Tree*> all_ul_trees(int n, limit_t limit);

#endif