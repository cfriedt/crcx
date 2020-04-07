include $(top_srcdir)/aminclude_static.am

if HAVE_GTEST

noinst_PROGRAMS =

AM_CPPFLAGS = \
	-I@top_srcdir@/src/ \
	$(CODE_COVERAGE_CPPFLAGS)

AM_CFLAGS = \
	@GTEST_CFLAGS@ \
	$(CODE_COVERAGE_CFLAGS)

AM_CXXFLAGS = \
	@GTEST_CFLAGS@ \
	$(CODE_COVERAGE_CXXFLAGS)

AM_LDFLAGS = \
	$(CODE_COVERAGE_LDFLAGS)

AM_LDADD = \
	@GTEST_LIBS@ \
	$(CODE_COVERAGE_LIBS)

noinst_PROGRAMS += crcx-test
crcx_test_SOURCES = crcx-test.cpp
crcx_test_LDADD = $(top_builddir)/src/libcrcx.la $(AM_LDADD)

if HAVE_CXX
noinst_PROGRAMS += crc3x-test
crc3x_test_SOURCES = crc3x-test.cpp
crc3x_test_LDADD = $(top_builddir)/src/libcrc3x.la $(AM_LDADD)
endif

.PHONY: gtest gcov

check-local: gtest gcov
clean-local:
	rm -f *.gcov

gtest: $(noinst_PROGRAMS)
	$(QUIET)if [ -z "$(strip $(noinst_PROGRAMS))" ]; then \
		exit 0; \
	fi; \
	NTEST=0; \
	NPASS=0; \
	if [ -z "$(strip $^)" ]; then \
		exit 0; \
	fi; \
	for i in $^; do \
		./$$i; \
		if [ $$? -eq 0 ]; then \
			NPASS=$$((NPASS+1)); \
		fi; \
		NTEST=$$((NTEST+1)); \
	done; \
	if [ $$NPASS -eq $$NTEST ]; then \
		exit 0; \
	else \
		exit 1; \
	fi

if CODE_COVERAGE_ENABLED

gcov: gtest
	cd $(top_builddir)/src/.libs && ln -sf ../crcx.c . && gcov -r libcrcx_la-crcx.c
	gcov -r crc3x-test.cpp

else

gcov:
	@true

endif

endif
