CXX=clang++
CXXFLAGS=-g

all: main

.PHONY: all

main: TreeNode.cc main.cc

clean: 
	rm -f *.o
	rm -f main

.PHONY: clean