#ifndef HELPERS_H_
#define HELPERS_H_

#include "tree.h"

#include "types.h"

#include <vector>

int index_of_rank(Tree* tree, int r);

std::vector<int> shape(Tree* tree, int start_rank);

void add_to(std::vector<count_t> &array, std::vector<count_t> &added, int offset);

#endif