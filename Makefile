CC=clang
CXX=clang++
CXXFLAGS=$(FLAGS) -g -I../lib

all: main

.PHONY: all

sqlite3.o: ../lib/sqlite3/sqlite3.c
	$(CC) -c -o $@ $?

main: TreeNode.cc SubcloneExplore.cc main.cc sqlite3.o

clean: 
	rm -f *.o
	rm -f main

.PHONY: clean
