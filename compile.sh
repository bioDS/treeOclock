gcc -c -g tree.c -o tree
gcc -W -Wall -O2 -ansi -pedantic -g main.c -o dct-distance tree.c path.c
