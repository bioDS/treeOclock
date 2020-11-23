# README

main.c reads a tree file, computes a shortest path with FINDPATH between the first two trees of that file, and returns all trees on this shortest path in "output/fp.rtree". Please ensure the tree with the largest root time is the second tree in the file.

## Ranked tree file format:
1st line: number of leaves  
2nd line: number of trees in file  
3rd and following lines: trees in list of sets representation, with each set preceded by a time and colon, (e.g. 3:).
All whitespace is ignored.

### Example for an input file:

> 5  
> 2  
> [1:{3,5}, 2:{2,4}, 3:{2,3,4,5}, 4:{1,5,4,3,2}]  
> [1:{1,2}, 2:{1,2,3}, 3:{1,2,3,4}, 4:{1,2,3,4,5}]


## Path matrix format:
Matrix int ** moves with dimension distance x 2.  
The first row contains the distance between the two trees (moves[0][0]).  
Each following row represents a move.  
1st column: rank of lower node bounding the interval on which move happens/  
2nd column: 0/1/2/3/4 if rank move/nni move moving children[1] up/nni move moving children[0] up/increasing length move/decreasing length move.
