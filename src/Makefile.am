lib_LTLIBRARIES =  \
	libcrcx.la     \
	libcrcxxx.la

libcrcxxx_la_SOURCES = \
	crcxxx.cpp
crcxxx_cpp_CXXFLAGS = \
	-Wall -Wextra -Werror

libcrcx_la_SOURCES = \
	crcx.c
crcx_c_CFLAGS = \
	-Wall -Wextra -Werror

nobase_include_HEADERS = \
	crcx/crcx.h          \
	crcx/crcxxx.h

pkgconfig_DATA = \
	crcx.pc      \
	crcxxx.pc
