# Computing nearest neighbour interchange distances between ranked phylogenetic trees - Code Repository

This repository contains the code for computing the Ranked Nearest Neighbour Interchange (RNNI) distance using the algorithm FindPath from the paper [Computing nearest neighbour interchange distances between ranked phylogenetic trees](https://doi.org/10.1007/s00285-021-01567-5) by Lena Collienne and Alex Gavryushkin.

An R package for computing the RNNI distance can be found [here](https://github.com/bioDS/rrnni).

Rooted trees with branch lengths (given in newick format) can be read through a Python interface and converted to ranked trees.
Then distances and shortest paths between those ranked trees in the RNNI space can be computed.


## Compile

Download repository and compile:

    git clone https://github.com/bioDS/treeOclock.git
    cd treeOclock
    make


## Functions executable from Python

Function | Return value
--- | ---
**Reading trees**
`read_nexus(filename)` |`Tree_Array` containing all trees from nexus file
`read_newick(newick_string)` | `Tree` given by newick_string
**Writing Trees**
`tree_to_cluster_string(tree)` | string of cluster representation of input `Tree`
**RNNI**
`rnni_distance(tree1, tree2)` | RNNI distance between `Tree`s tree1 and tree2
`findpath(tree1, tree2)` | `Tree_Array` containing all trees on shortest path from `Tree` tree1 to tree2 computed by FindPath

### Example

```
from tree_parser.tree_io import *
from tree_functions import *

tree1 = read_newick("((1:1,2:1):2,(3:2,4:2):1);")
tree2 = read_newick("(((3:1,4:1):1,2:2):1,1:3);")

print(rnni_distance(tree1, tree2))

path = findpath_path(tree1, tree2)
for i in range(0, path.num_trees):
    print(tree_to_cluster_string(path.trees[i]))
```

## Data Structures

Struct | Members
--- | ---
**struct Node** (tree.h)
long parent | parent (-1 if node root)
long children[2] | two children (-1 if node leaf)
long time | time of this node (=rank for ranked trees); leaves have time 0
**struct Tree** (tree.h)
Node* node_array | array containing 2 * num_leaves - 1 nodes;<br> first num_leaves nodes are leaves, last num_leaves - 1 nodes are internal nodes
long num_leaves | number of leaves
**struct Tree_Array** (tree.h)
Tree* trees | array of trees
long num_trees | number of trees
**struct Path** (rnni.h)
long** moves | encoding RNNI moves in a matrix where each moves[i] is one move; <br> moves[i][0]: rank of lower node of interval on which move is performed <br> moves[i][1]: 0 -> rank moves, 1 -> NNI move where children[0] moves up, 2-> NNI move where children[1] moves up
long length | number of moves

## Most important C functions

This is a list of the (probably) most important functions in C code

Function | Return value
--- | ---
**tree.c**
`void print_tree(Tree* tree)` | prints for every node in *tree.node_array* parent, children, and time
`int same_tree(Tree* tree1, Tree* tree2)` | returns 1 if tree1 and tree2 are isomorphic
**rnni.c**
`Tree_Array rnni_neighbourhood(Tree* tree)` | returns `Tree_Array` containing all RNNI neighbours of *tree*
`void uniform_neighbour(Tree* tree)` | performs RNNI move on *tree*, uniformly chosen from all possible moves
`long rnni_distance(Tree* start_tree, Tree* dest_tree)` | returns RNNI distance between *start_tree* and *dest_tree*
`Path findpath_moves(Tree* start_tree, Tree* dest_tree)` | returns FindPath path in matrix encoding (*Path*) -- preserves running time O(n^2) while saving all moves
`Tree_Array findpath(Tree* start_tree, Tree* dest_tree)` | returns `Tree_Array` of all trees on FindPath path -- running time in O(n^3)
**exploring_rnni.c**
`long random_walk(Tree* tree, long k)` | Performs *k* RNNI moves (uniformly chosen among all possible ones in each step) and returns RNNI distance between initial tree and tree after k moves