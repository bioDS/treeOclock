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