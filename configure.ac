#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([crcx], [0.1], [cfriedt.crcx@github.com])
AC_CONFIG_SRCDIR([src/crcx.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIRS([m4])

LT_INIT

AM_INIT_AUTOMAKE([foreign])

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

AC_PROG_CC_C11
AX_CXX_COMPILE_STDCXX(17, noext)

PKG_INSTALLDIR

# Checks for libraries.

# gtest_main is required for unit and integration tests
with_gtest="auto"
AC_ARG_WITH(
        [gtest],
        [AS_HELP_STRING(
                [--with-gtest],
                [Enable gtest for unit and integration testing (auto)]
        )]
)
AS_IF(
        [test "x$with_gtest" != "xno"],

        [PKG_CHECK_MODULES(
                [GTEST],
                [gtest_main],
                [have_gtest=yes],
                [have_gtest=no]
        )],

        [have_gtest=no]
)
AS_IF(
        [test "x$have_gtest" = "xyes"],
        [
                AC_SUBST([GTEST_LIBS])
                AC_SUBST([GTEST_CFLAGS])
        ],
        [AS_IF([test "x$with_gtest" = "xyes"],
                [AC_MSG_ERROR([gtest requested but not found])
        ])
])
AM_CONDITIONAL([HAVE_GTEST],[test "x$have_gtest" = "xyes"])

with_cxx="auto"
AC_ARG_WITH(
        [cxx],
        [AS_HELP_STRING(
                [--with-cxx],
                [Build C++ Library (auto)]
        )]
)
AS_IF(
        [test "x$with_cxx" != "xno"],
        [have_cxx=yes],
        [have_cxx=no]
)
AM_CONDITIONAL([HAVE_CXX],[test "x$have_cxx" = "xyes"])

# Checks for header files.
AC_CHECK_HEADERS([inttypes.h stddef.h stdint.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_CHECK_FUNCS([memset strtoull])

AC_CHECK_PROGS([DOXYGEN], [doxygen])
if test -z "$DOXYGEN"; then
   AC_MSG_WARN([Doxygen not found - continue without Doxygen support])
else
AC_CHECK_PROGS([DOT], [dot])
if test -z "$DOT"; then
   AC_MSG_ERROR([Doxygen needs dot, please install dot first])
fi
#AC_CHECK_PROGS([PDFLATEX], [pdflatex])
#if test -z "$PDFLATEX"; then
#   AC_MSG_ERROR([Doxygen needs pdflatex program, it is part of TeX http://www.tug.org/texlive/acquire-netinstall.html])
#fi
fi
AM_CONDITIONAL([HAVE_DOXYGEN], [test -n "$DOXYGEN"])
AM_COND_IF([HAVE_DOXYGEN], [AC_CONFIG_FILES([docs/Doxyfile])])
AS_IF([test "x$have_cxx" = "xyes"], [AC_CONFIG_FILES([src/crcxxx.pc])])

AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/crcx.pc
	test/Makefile
	docs/Makefile
])

AC_OUTPUT
