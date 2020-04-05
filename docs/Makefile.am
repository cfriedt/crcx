if HAVE_DOXYGEN

doxyfile.stamp: Doxyfile
	$(DOXYGEN)
	touch $@

html: doxyfile.stamp

CLEANFILES =           \
	doxyfile.stamp

all-local: doxyfile.stamp

clean-local:
	rm -Rf html latex

endif
