AM_CPPFLAGS = -I$(srcdir)/../src -I$(srcdir)/../contrib
AM_LDFLAGS = -L$(srcdir)/../src

bin_PROGRAMS = ssmain

ssmain_SOURCES = SubcloneExplore.cc \
				 $(srcdir)/../contrib/sqlite3/sqlite3.c
ssmain_LDADD = -lss

