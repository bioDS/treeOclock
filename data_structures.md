# Data structures

## Nodes
- int parent -- index of parent
- int children[2] -- index of children

## Trees
- Node * tree of length 2*n-1
- leaves:
    - in first n positions of array
    - children of leaves: -1, int parent = n + rank of parent - 1
- internal nodes:
    - in last n-1 positions of array => internal node of rank r has index n + r - 1
    - parent of root: -1

## Tree_List
- Node ** -- list of trees (as described above)
- int num_leaves
- int num_trees -- length of the tree list

## Paths
- int path[path_length][2]
- The first row contains only the distance value of the path.
- Each following row path[i] represents a move (i + 1)th move on path
- in first column: index of rank of lower node bounding interval of move: path[i][0] = k if move i happens on interval [k,k+1]. If the move was a length move, this column represents the node index of the move.
- in second column: path[i][1] in {0,1,2,3,4} ; 0 if rank move, 1 if NNI move where children[0] stays child of lower node, 2 if NNI move where children[1] stays child of lower node, 3 if length move where k.time was increased, 4 if length move where k.time was decreased.
