# dct_parser

This repository contains a parser that reads trees as nexus files and returns them as discrete coalescent tree (dct) data structure.

## dct data structure

Data structure used here is the following:

| Structure			|	Properties |
---			|	---
| Node | parent, children, time |
| Tree | node_array (array of nodes), num_leaves, root_time, sos_d |
| Tree_Array | trees (array of trees), num_trees |
| Path | moves ((2 x length) matrix: each row is a move: moves[i][0] -- time of lower node bounding the interval on which move happens, moves[i][1] = 0/1/2 -- rank move, NNI move where children[0] stays the same/ NNI move where children[1] stays the same), length |