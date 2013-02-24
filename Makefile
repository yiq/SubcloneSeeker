CC=clang
CXX=clang++
CXXFLAGS=$(FLAGS) -g -I../lib -I../lib/boost

SOURCES = TreeNode.cc
HEADERS = TreeNode.h \
		  MutationTreeNode.h \
		  GenomicLocation.h \
		  GenomicRange.h \
		  Archivable.h

MAIN_SRC = main.cc

OBJS = $(SOURCES:%.cc=%.o)

all: main

.PHONY: all

sqlite3.o: ../lib/sqlite3/sqlite3.c
	$(CC) -c -o $@ $<

.cc.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<

main: $(OBJS) $(HEADERS) $(MAIN_SRC) sqlite3.o
	$(CXX) $(CXXFLAGS) $(OBJS) $(MAIN_SRC) sqlite3.o -o main

check: $(OBJS) $(HEADERS) sqlite3.o
	$(CXX) $(CXXFLAGS) $(OBJS) sqlite3.o test/*.cc -o runTest
	./runTest

clean: 
	rm -f *.o
	rm -f main runTest
	rm -rf *.dSYM

.PHONY: clean
