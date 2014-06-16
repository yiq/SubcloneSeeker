#
# Makefile for SubcloneSeeker
# 

CC=gcc
CXX=g++
AR=ar

CFLAGS=-I../vendor -I../src
CXXFLAGS=$(CFLAGS)
TEST_FLAGS=-I../vendor/UnitTest++
LDFLAGS=-L../src
LDADDS=../src/libss.a -lpthread -ldl
LDADDS_TEST=../vendor/UnitTest++/libUnitTest++.a

SSMAIN=ssmain
SSMAIN_OBJS=SubcloneSeeker.o

SEGTXT2DB=segtxt2db
SEGTXT2DB_OBJS=segtxt2db.o

TREEMERGE=treemerge
TREEMERGE_OBJS=treemerge.o \
			   treemerge_p.o

TREEPRINT=treeprint
TREEPRINT_OBJS=treeprint.o

COLOCAL_MATRIX=colocal_matrix
COLOCAL_MATRIX_OBJS=colocal_matrix.o \
					CoexistanceTable.o

CLUSTER2DB=cluster2db 
CLUSTER2DB_OBJS=cluster2db.o

TEST_TREEMERGE = treemerge.test
TEST_TREEMERGE_OBJS = treemerge_test.o \
					  treemerge_p.o

TARGETS=$(SSMAIN) \
		$(SEGTXT2DB) \
		$(TREEMERGE) \
		$(TREEPRINT) \
		$(COLOCAL_MATRIX) \
		$(CLUSTER2DB)

OBJECTS=$(SSMAIN_OBJS) \
		$(SEGTXT2DB_OBJS) \
		$(TREEMERGE_OBJS) \
		$(TREEPRINT_OBJS) \
		$(COLOCAL_MATRIX_OBJS) \
		$(CLUSTER2DB)

TEST_OBJECTS=$(TEST_TREEMERGE_OBJS)

TESTS=$(TEST_TREEMERGE)



SOURCES=SubcloneSeeker.cc \
		segtxt2db.cc \
		treemerge.cc \
		treemerge_p.cc \
		treeprint.cc \
		CoexistanceTable.cpp \
		colocal_matrix.cpp \
		cluster2db.cc

.cc.o:
	$(CXX) $(CFLAGS) -c -o $@ $<


all: $(TARGETS)

$(SSMAIN): $(SSMAIN_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDADDS)

$(SEGTXT2DB): $(SEGTXT2DB_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDADDS)

$(TREEMERGE): $(TREEMERGE_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDADDS)

$(TREEPRINT): $(TREEPRINT_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDADDS)

$(COLOCAL_MATRIX): $(COLOCAL_MATRIX_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDADDS)

$(CLUSTER2DB): $(CLUSTER2DB_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDADDS)


$(TEST_TREEMERGE): $(TEST_TREEMERGE_OBJS)
	$(CXX) $(CXXFLAGS) $(TEST_FLAGS) $(LDFLAGS) -o $@ $^ $(LDADDS) $(LDADDS_TEST)

check: $(TESTS)
	@./$<

clean:
	rm -rf $(TARGETS)
	rm -rf $(OBJECTS)
	rm -rf $(TESTS)
	rm -rf $(TEST_OBJECTS)

.PHONY: all clean
