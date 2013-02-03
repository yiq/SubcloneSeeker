CXX=clang++
CXXFLAGS=$(FLAGS) -g

all: main

.PHONY: all

main: TreeNode.cc SubcloneExplore.cc main.cc 

clean: 
	rm -f *.o
	rm -f main

.PHONY: clean