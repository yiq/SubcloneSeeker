#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([SubcloneSeeker], [1.0], [yi.qiao.2@bc.edu])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/Subclone.cc])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_RANLIB

# Checks for libraries.

AC_SEARCH_LIBS([fabs], [m])

# Checks for header files.

AC_CHECK_HEADERS([math.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([pow strdup fabs])

AC_CONFIG_FILES([Makefile 
				 src/Makefile 
				 utils/Makefile 
				 test/Makefile
				 doc/Doxyfile])
AC_OUTPUT
