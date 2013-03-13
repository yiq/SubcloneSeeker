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
		  SubcloneExplore.cc \
		  RefGenome.cc

HEADERS = GenomicLocation.h \
		  GenomicRange.h \
		  Archivable.h \
		  EventCluster.h \
		  TreeNode.h \
		  Subclone.h \
		  SubcloneExplore.h \
		  RefGenome.h

OBJS = $(SOURCES:%.cc=%.o)

all: stexp utils

.PHONY: all

sqlite3.o: ../lib/sqlite3/sqlite3.c
	$(CC) -c -o $@ $<

.cc.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<

stexp: $(OBJS) $(HEADERS) main.cc sqlite3.o
	$(CXX) $(CXXFLAGS) $(OBJS) main.cc sqlite3.o -o stexp

utils: cluster segtxt2db treemerge

cluster: $(OBJS) $(HEADERS) cluster.cc sqlite3.o
	$(CXX) $(CXXFLAGS) $(OBJS) cluster.cc sqlite3.o -o cluster

segtxt2db: $(OBJS) $(HEADERS) segtxt2db.cc sqlite3.o
	$(CXX) $(CXXFLAGS) $(OBJS) segtxt2db.cc sqlite3.o -o segtxt2db

treemerge: $(OBJS) $(HEADERS) treemerge.cc sqlite3.o
	$(CXX) $(CXXFLAGS) $(OBJS) treemerge.cc sqlite3.o -o treemerge

check: $(OBJS) $(HEADERS) sqlite3.o
	rm -f test.sqlite
	$(CXX) -DKEEP_TEST_DB $(CXXFLAGS) $(OBJS) sqlite3.o test/*.cc -o runTest
	./runTest
	rm -rf runTest runTest.dSYM

clean: 
	rm -f *.o
	rm -rf *.dSYM

.PHONY: clean
