default: tree.so
	# gcc -fPIC -Wall -c -g -O2 -fsanitize=address tree.c

tree.so: tree.o rnni.o spr.o exploring_rnni.o orbits.o
	gcc -shared -g -o tree.so tree.o rnni.o spr.o exploring_rnni.o orbits.o

tree.o: tree.c tree.h
	gcc -fPIC -Wall -c -g -O2 tree.c

rnni.o: rnni.c rnni.h
	gcc -fPIC -Wall -c -g -O2 rnni.c

spr.o: spr.c spr.h
	gcc -fPIC -Wall -c -g -O2 spr.c

exploring_rnni.o: exploring_rnni.c exploring_rnni.h
	gcc -fPIC -Wall -c -g -O2 exploring_rnni.c

orbits.o: orbits.c orbits.h tree.h rnni.h
	gcc -fPIC -Wall -c -g -O2 orbits.c

.PHONY: clean
clean:
	rm *.o *.so