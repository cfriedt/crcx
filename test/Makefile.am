noinst_PROGRAMS =

AM_CPPFLAGS = -I@top_srcdir@/src/

AM_CFLAGS = @GTEST_CFLAGS@

AM_LDADD = \
	@GTEST_LIBS@

if HAVE_GTEST

.PHONY: gtest gcov clangtidy format

noinst_PROGRAMS += crcx-test
crcx_test_SOURCES = crcx-test.cpp
crcx_test_LDADD= @top_srcdir@/src/lib@PACKAGE_NAME@.la $(AM_LDADD)

if HAVE_CXX
noinst_PROGRAMS += crcxxx-test
crcxxx_test_SOURCES = crcxxx-test.cpp
crcxxx_test_LDADD = @top_srcdir@/src/lib@PACKAGE_NAME@xx.la $(AM_LDADD)
endif

check-local:
	$(MAKE) gtest

gtest: $(noinst_PROGRAMS)
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

endif
