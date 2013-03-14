CC=clang
CXX=clang++
CXXFLAGS=$(FLAGS) -g -I../lib -I../lib/boost
UTILSFLAGS=$(FLAGS) -g -I../lib -I../lib/boost -I./

SOURCES = Archivable.cc \
		  SomaticEvent.cc \
		  SegmentalMutation.cc \
		  SNP.cc \
		  EventCluster.cc \
		  TreeNode.cc \
		  Subclone.cc \
		  RefGenome.cc

HEADERS = GenomicLocation.h \
		  GenomicRange.h \
		  Archivable.h \
		  EventCluster.h \
		  TreeNode.h \
		  Subclone.h \
		  RefGenome.h

OBJS = $(SOURCES:%.cc=%.o)

all: stexp utils

.PHONY: all

sqlite3.o: ../lib/sqlite3/sqlite3.c
	$(CC) -c -o $@ $<

.cc.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<

stexp: $(OBJS) $(HEADERS) utils/SubcloneExplore.cc sqlite3.o
	$(CXX) $(UTILSFLAGS) $(OBJS) utils/SubcloneExplore.cc sqlite3.o -o stexp

utils: cluster segtxt2db treemerge

cluster: $(OBJS) $(HEADERS) utils/cluster.cc sqlite3.o
	$(CXX) $(UTILSFLAGS) $(OBJS) utils/cluster.cc sqlite3.o -o cluster

segtxt2db: $(OBJS) $(HEADERS) utils/segtxt2db.cc sqlite3.o
	$(CXX) $(UTILSFLAGS) $(OBJS) utils/segtxt2db.cc sqlite3.o -o segtxt2db

treemerge: $(OBJS) $(HEADERS) utils/treemerge.cc sqlite3.o
	$(CXX) -DTREEMERGE_MAIN $(UTILSFLAGS) $(OBJS) utils/treemerge.cc sqlite3.o -o treemerge

treemerge_check: $(OBJS) $(HEADERS) utils/treemerge.cc sqlite3.o
	$(CXX) -DTREEMERGE_TEST $(UTILSFLAGS) $(OBJS) utils/treemerge.cc sqlite3.o -o treemerge_check
	./treemerge_check
	rm treemerge_check

check: $(OBJS) $(HEADERS) sqlite3.o
	rm -f test.sqlite
	$(CXX) -DKEEP_TEST_DB $(CXXFLAGS) $(OBJS) sqlite3.o test/*.cc -o runTest
	./runTest
	rm -rf runTest runTest.dSYM

clean: 
	rm -f *.o
	rm -rf *.dSYM

.PHONY: clean
