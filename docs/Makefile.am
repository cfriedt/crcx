if HAVE_DOXYGEN

doxyfile.stamp:
	$(DOXYGEN)
	touch $@

CLEANFILES = \
	doxyfile.stamp

all-local: doxyfile.stamp

endif
