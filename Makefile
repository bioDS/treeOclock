default:
	# gcc -fPIC -Wall -c -g -O2 -fsanitize=address tree.c
	gcc -fPIC -Wall -c -g -O2 tree.c
	gcc -fPIC -Wall -c -g -O2 rnni.c
	gcc -fPIC -Wall -c -g -O2 queue.c
	gcc -fPIC -Wall -c -g -O2 spr.c
	gcc -fPIC -Wall -c -g -O2 unlabelled_rnni.c
	gcc -shared -o tree.so tree.o rnni.o spr.o queue.o unlabelled_rnni.o
