SUBDIRS =    \
	src  \
	test \
	docs

.PHONY: format

format:
	if [ x"`which clang-format`" != x"" ]; then \
		clang-format -i `find @top_srcdir@ -name '*.c' -o -name '*.cpp' -o -name '*.h'`; \
	fi
