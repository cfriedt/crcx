if HAVE_DOXYGEN

doxyfile.stamp:
	$(DOXYGEN)
	touch $@

html: doxyfile.stamp

CLEANFILES =           \
	doxyfile.stamp \
	html           \
	latex

all-local: doxyfile.stamp

endif
