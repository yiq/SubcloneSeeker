#
# Makefile for SubcloneSeeker
# 

CC=gcc
CXX=g++
AR=ar

CFLAGS=-I../vendor

SOURCES=Archivable.cc \
		EventCluster.cc \
		RefGenome.cc \
		SNP.cc \
		SegmentalMutation.cc \
		SomaticEvent.cc \
		Subclone.cc \
		TreeNode.cc 

SQLITE3_SOURCES=../vendor/sqlite3/sqlite3.c

OBJECTS=$(SOURCES:.cc=.o)
EXTRA_OBJECTS=$(SQLITE3_SOURCES:.c=.o)

TARGET=libss.a

.cc.o:
	$(CXX) $(CFLAGS) -c -o $@ $<

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

all: $(TARGET)

$(TARGET): $(OBJECTS) $(EXTRA_OBJECTS)
	ar rcs $(TARGET) $(OBJECTS) $(EXTRA_OBJECTS)

clean:
	rm -rf $(OBJECTS) $(TARGET)

.PHONY: all clean
