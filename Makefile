CC=clang
CXX=clang++
CXXFLAGS=$(FLAGS) -g -I../lib -I../lib/boost

SOURCES = Archivable.cc \
		  SomaticEvent.cc \
		  SegmentalMutation.cc \
		  SNP.cc \
		  EventCluster.cc \
		  TreeNode.cc \
		  Subclone.cc \
		  SubcloneExplore.cc

HEADERS = GenomicLocation.h \
		  GenomicRange.h \
		  Archivable.h \
		  EventCluster.h \
		  TreeNode.h \
		  Subclone.h \
		  SubcloneExplore.h

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
	rm -f test.sqlite
	$(CXX) -DKEEP_TEST_DB $(CXXFLAGS) $(OBJS) sqlite3.o test/*.cc -o runTest
	./runTest
	rm -rf runTest runTest.dSYM

clean: 
	rm -f *.o
	rm -f main
	rm -rf *.dSYM

.PHONY: clean
