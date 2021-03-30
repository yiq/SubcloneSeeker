bin_PROGRAMS=hello hello_static
lib_LTLIBRARIES = libhello.la

libhello_la_SOURCES = core_capi.c core_ccapi.cc
libhello_la_LDFLAGS = -version-info 0:0:0

hello_SOURCES = main.cc
hello_LDADD = libhello.la

hello_static_SOURCES = main.cc
hello_static_LDADD = libhello.la
hello_static_LDFLAGS = -static
