# Computing nearest neighbour interchange distances between ranked phylogenetic trees - Code Repository

This repository contains the code for computing the RNNI distance using the algorithm FINDPATH from the paper [Computing nearest neighbour interchange distances between ranked phylogenetic trees](https://doi.org/10.1007/s00285-021-01567-5) by Lena Collienne and Alex Gavryushkin

An R package for computing the RNNI distance can be found [here](https://github.com/bioDS/rrnni).

The implementation here is written in C, wrapped in Python.


## Compilation

`make`


## Reading Trees

Ranked trees can be read from from nexus files or as newick strings with `read_nexus(filename)` or `read_newick(newick_string)`.


## Writing Trees

There is currently no function to return trees in newick format.
We can however return them as strings in cluster representations (see Collienne and Gavryushkin, 2021) using the function `tree_to_cluster_string`.


## Computing RNNI distances or paths

| Functions			|	Description
---			|	---
| rnni_distance | Distance for two ranked trees |
| findpath_path | Shortest path for two trees computed by FindPath -- returned in our C data structure |

