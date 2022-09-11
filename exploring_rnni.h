#ifndef EXPLORINGRNNI_H_
#define EXPLORINGRNNI_H_

# include "rnni.h"


long random_walk(Tree * tree, long k);
int first_iteration_fp(Tree_Array* treelist, long node1, long node2, long r);
long sos(Tree_Array* treelist, Tree* focal_tree);

#endif
