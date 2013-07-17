AM_CPPFLAGS = -I$(srcdir)/../contrib -I$(srcdir)/../src
AM_LDFLAGS = -L$(srcdir)/../src -L$(srcdir)/../contrib/UnitTest++
LDADD = -lss -lUnitTest++

check_PROGRAMS = TestGenomicLocation \
				 TestGenomicRange

TESTS = $(check_PROGRAMS)

TestGenomicLocation_SOURCES = TestGenomicLocation.cc
TestGenomicRange_SOURCES = TestGenomicRange.cc
