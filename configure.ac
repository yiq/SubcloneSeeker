# Perlude.

AC_INIT([subcloneseeker], [2.0.0], [atrusqiao@gmail.com])
AC_PREREQ([2.70])  # Let's forget about the old days
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

## some common options
AX_CHECK_ENABLE_DEBUG()
AM_CONDITIONAL([DEBUG], [test x"$ax_enable_debug" = x"yes"])
# set log level, meant to be used with YiCppLib::Log
AM_COND_IF([DEBUG], [AC_DEFINE([LOGLVL], [LOGLV_DEBUG], [Compile time log level])])

AC_PROG_CC
AC_PROG_CXX

# C++14
AX_CXX_COMPILE_STDCXX_14

# Automake
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])

AM_PROG_AR

# Libtool
LT_PREREQ([2.4])
LT_INIT()

AC_ARG_WITH([htslib],
            [AS_HELP_STRING([--with-htslib=<path-to-htslib>],
                            [Specify the path to which htslib this program will link against])])

AS_IF([test "x$with_htslib" != x], [CPPFLAGS="$CPPFLAGS -I$with_htslib -I$with_htslib/include"])
AS_IF([test "x$with_htslib" != x], [LDFLAGS="$LDFLAGS -L$with_htslib -L$with_htslib/lib -L$with_htslib/lib64"])
AC_CHECK_LIB([hts], [bcf_read], [], [AC_MSG_ERROR([Could not find htslib shared library])])


# Config files to output
# Makefile: top level makefile
# src/Makefile: actual targets for the project
# test/Makefile: any test targets for the project
# contrib/Makefile: how to handle 3rd party projects
AC_CONFIG_HEADERS([config.h])
#AC_CONFIG_FILES([Makefile src/Makefile test/Makefile contrib/Makefile])
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
