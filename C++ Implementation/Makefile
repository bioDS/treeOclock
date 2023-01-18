CC = g++
#CFLAGS = -Wall -Wextra -Werror -std=c++17 -O2 -lm -g -fsanitize=address,undefined
CFLAGS = -Wall -Wextra -Werror -std=c++17 -O2 -lm
#CFLAGS = -std=c++17 -g -O2 -lm

default: main.out

tree.o: tree.cpp tree.h
	$(CC) $(CFLAGS) -c $<

rnni.o: rnni.cpp rnni.h
	$(CC) $(CFLAGS) -c $<

generate.o: generate.cpp tree.h types.h generate.h
	$(CC) $(CFLAGS) -c $<

helpers.o: helpers.cpp tree.h types.h helpers.h
	$(CC) $(CFLAGS) -c $<

types.o: types.cpp types.h
	$(CC) $(CFLAGS) -c $<

orbits.o: orbits.cpp tree.h rnni.h generate.h helpers.h types.h orbits.h
	$(CC) $(CFLAGS) -c $<

limited_orbits.o: limited_orbits.cpp tree.h rnni.h generate.h helpers.h types.h  limited_orbits.h
	$(CC) $(CFLAGS) -c $<

main.out: main.cpp orbits.o limited_orbits.o tree.o rnni.o generate.o helpers.o types.o
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm *.o *.out