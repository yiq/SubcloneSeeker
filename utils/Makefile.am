AM_CPPFLAGS = -I$(srcdir)/../src -I$(srcdir)/../contrib
AM_LDFLAGS = -L$(srcdir)/../src
LIBS = -lss

bin_PROGRAMS = ssmain treeprint treemerge segtxt2db
check_PROGRAMS = treemerge_test

TESTS = $(check_PROGRAMS)

ssmain_SOURCES = SubcloneExplore.cc 

treeprint_SOURCES = treeprint.cc

treemerge_SOURCES = treemerge.cc treemerge_p.cc

treemerge_test_SOURCES = treemerge_test.cc treemerge_p.cc
treemerge_test_LDADD = -L$(srcdir)/../contrib/UnitTest++ -lUnitTest++

segtxt2db_SOURCES = segtxt2db.cc
