default:
	# gcc -fPIC -Wall -c -g -O2 -fsanitize=address tree.c
	gcc -fPIC -Wall -c -g -O2 tree.c
	gcc -fPIC -Wall -c -g -O2 queue.c
	gcc -fPIC -Wall -c -g -O2 spr.c
	gcc -shared -o tree.so tree.o spr.o queue.o
