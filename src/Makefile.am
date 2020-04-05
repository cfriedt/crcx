AM_CFLAGS = \
	-Wall -Wextra -Werror
AM_CXXFLAGS = \
	-Wall -Wextra -Werror

lib_LTLIBRARIES =  \
	libcrcx.la     \
	libcrc3x.la

libcrc3x_la_SOURCES = \
	crc3x.cpp

libcrcx_la_SOURCES = \
	crcx.c

nobase_include_HEADERS = \
	crcx/crcx.h          \
	crc3x/crc3x.h

pkgconfig_DATA = \
	crcx.pc      \
	crc3x.pc
