AM_CPPFLAGS = -I$(srcdir)/../src -I$(srcdir)/../contrib
AM_LDFLAGS = -L$(srcdir)/../src

bin_PROGRAMS = ssmain treeprint treemerge
check_PROGRAMS = treemerge_test

TESTS = $(check_PROGRAMS)

ssmain_SOURCES = SubcloneExplore.cc 
ssmain_LDADD = -lss

treeprint_SOURCES = treeprint.cc
treeprint_LDADD = -lss

treemerge_SOURCES = treemerge.cc treemerge_p.cc
treemerge_LDADD = -lss

treemerge_test_SOURCES = treemerge_test.cc treemerge_p.cc
treemerge_test_LDADD = -lss -L$(srcdir)/../contrib/UnitTest++ -lUnitTest++

