# Computing nearest neighbour interchange distances between ranked phylogenetic trees - Code Repository

This repository contains the code for computing the RNNI distance using the algorithm FINDPATH from the paper [Computing nearest neighbour interchange distances between ranked phylogenetic trees](https://doi.org/10.1007/s00285-021-01567-5) by Lena Collienne and Alex Gavryushkin

An R package for computing the RNNI distance can be found [here](https://github.com/bioDS/rrnni).

The implementation here is written in C, wrapped in Python.


## Compile

Download repository and compile:


    git clone https://github.com/bioDS/treeOclock.git
    cd treeOclock
    make


## Functions executable from Python

### Reading Trees

`read_nexus(filename)`
-- Returns `Tree_Array` containing all trees from nexus file

`read_newick(newick_string)`
-- Returns `Tree` given by newick_string


### Writing Trees

There is currently no function to return trees in newick format

`tree_to_cluster_string(tree)`
-- Returns input tree (in Tree format) as string in cluster representation


## Computing RNNI distances or paths

| Functions			|	Description
---			|	---
| rnni_distance | Distance for two ranked trees |
| findpath_path | Shortest path for two trees computed by FindPath -- returned in our C data structure |

