include $(top_srcdir)/aminclude_static.am

AM_CFLAGS = \
	-Wall -Wextra -Werror
AM_CXXFLAGS = \
	-Wall -Wextra -Werror

lib_LTLIBRARIES =  \
	libcrcx.la     \
	libcrc3x.la

libcrc3x_la_SOURCES = \
	crc3x.cpp
libcrc3x_la_CPPFLAGS = \
	$(CODE_COVERAGE_CPPFLAGS)
libcrc3x_la_CFLAGS = \
	$(CODE_COVERAGE_CFLAGS)
libcrc3x_la_LIBADD = \
	$(CODE_COVERAGE_LIBS)

libcrcx_la_SOURCES = \
	crcx.c
libcrcx_la_CPPFLAGS = \
	$(CODE_COVERAGE_CPPFLAGS)
libcrcx_la_CFLAGS = \
	$(CODE_COVERAGE_CFLAGS)
libcrcx_la_LIBADD = \
	$(CODE_COVERAGE_LIBS)

nobase_include_HEADERS = \
	crcx/crcx.h          \
	crc3x/crc3x.h

pkgconfig_DATA = \
	crcx.pc      \
	crc3x.pc
