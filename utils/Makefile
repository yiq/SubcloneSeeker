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

TEST_TREEMERGE = treemerge.test
TEST_TREEMERGE_OBJS = treemerge_test.o \
					  treemerge_p.o

TARGETS=$(SSMAIN) \
		$(SEGTXT2DB) \
		$(TREEMERGE) \
		$(TREEPRINT)

OBJECTS=$(SSMAIN_OBJS) \
		$(SEGTXT2DB_OBJS) \
		$(TREEMERGE_OBJS) \
		$(TREEPRINT_OBJS)

TEST_OBJECTS=$(TEST_TREEMERGE_OBJS)

TESTS=$(TEST_TREEMERGE)



SOURCES=SubcloneSeeker.cc \
		segtxt2db.cc \
		treemerge.cc \
		treemerge_p.cc \
		treeprint.cc

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
