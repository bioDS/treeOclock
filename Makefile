default:
	# gcc -fPIC -Wall -c -g -O2 -fsanitize=address tree.c
	gcc -fPIC -Wall -c -g -O2 tree.c
	gcc -shared -o tree.so tree.o
