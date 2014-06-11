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
LDADDS=../src/libss.a
LDADDS_TEST=../vendor/UnitTest++/libUnitTest++.a

TEST_SOURCES=TestEventCluster.cc \
			 TestGenomicLocation.cc \
			 TestGenomicRange.cc \
			 TestSomaticEvent.cc \
			 TestSubclone.cc \
			 TestTreeNode.cc

TESTS=$(TEST_SOURCES:.cc=.test)
TEST_STUBS=$(TESTS:.test=.stub)

.SUFFIXES: .test .stub

.cc.test:
	$(CXX) $(CXXFLAGS) $(TEST_FLAGS) -o $@ $< $(LDADDS) $(LDADDS_TEST)

%.stub: %.test
	@echo "Running $(<:.test=)..."
	@./$<

all: check

check: $(TEST_STUBS)

clean:
	rm -rf $(TESTS)

.PHONY: all clean check
