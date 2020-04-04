AM_CFLAGS = \
	-Wall -Wextra -Werror
AM_CXXFLAGS = \
	-Wall -Wextra -Werror

lib_LTLIBRARIES =  \
	libcrcx.la     \
	libcrcxxx.la

libcrcxxx_la_SOURCES = \
	crcxxx.cpp

libcrcx_la_SOURCES = \
	crcx.c

nobase_include_HEADERS = \
	crcx/crcx.h          \
	crcx/crcxxx.h

pkgconfig_DATA = \
	crcx.pc      \
	crcxxx.pc
