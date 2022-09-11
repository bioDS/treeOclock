default: tree.so
	# gcc -fPIC -Wall -c -g -O2 -fsanitize=address tree.c

tree.so: tree.o rnni.o spr.o queue.o unlabelled_tree.o unlabelled_rnni.o exploring_rnni.o
	gcc -shared -g -o tree.so tree.o rnni.o spr.o queue.o unlabelled_tree.o unlabelled_rnni.o exploring_rnni.o

tree.o: tree.c tree.h
	gcc -fPIC -Wall -c -g -O2 tree.c

rnni.o: rnni.c rnni.h
	gcc -fPIC -Wall -c -g -O2 rnni.c

queue.o: queue.c queue.h
	gcc -fPIC -Wall -c -g -O2 queue.c

spr.o: spr.c spr.h
	gcc -fPIC -Wall -c -g -O2 spr.c

unlabelled_tree.o: unlabelled_tree.c unlabelled_tree.h
	gcc -fPIC -Wall -c -g -O2 unlabelled_tree.c

unlabelled_rnni.o: unlabelled_rnni.c unlabelled_rnni.h
	gcc -fPIC -Wall -c -g -O2 unlabelled_rnni.c

exploring_rnni.o: exploring_rnni.c exploring_rnni.h
	gcc -fPIC -Wall -c -g -O2 exploring_rnni.c
