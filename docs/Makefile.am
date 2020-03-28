if HAVE_DOXYGEN

doxyfile.stamp:
	$(DOXYGEN)
	touch $@

html: doxyfile.stamp

CLEANFILES =           \
	doxyfile.stamp \
	html           \
	latex          \
	Doxyfile

all-local: doxyfile.stamp

endif
