#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# The MIT License (MIT)
# 
# Copyright (c) 2013 Yi Qiao
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

AC_PREREQ([2.69])
AC_INIT([SubcloneSeeker], [1.0], [yi.qiao.2@bc.edu])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/Subclone.cc])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_RANLIB

AC_CHECK_PROGS([DOXYGEN], [doxygen])
if test -z "$DOXYGEN";
then AC_MSG_WARN([Doxygen not found - continuing without Doxygen support])
fi
AM_CONDITIONAL([HAVE_DOXYGEN],
			   [test -n "$DOXYGEN"])
AM_COND_IF([HAVE_DOXYGEN], [AC_CONFIG_FILES([doc/Doxyfile])])

# Checks for libraries.

AC_SEARCH_LIBS([pthread_mutex_trylock], [pthread])
AC_SEARCH_LIBS([dlopen], [dl])
AC_SEARCH_LIBS([fabs], [m])

# Checks for header files.

AC_CHECK_HEADERS([pthread.h])
AC_CHECK_HEADERS([math.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([pow strdup fabs])

AC_CONFIG_FILES([Makefile 
				 vendor/Makefile
				 src/Makefile 
				 utils/Makefile 
				 test/Makefile])
AC_OUTPUT
