SUBDIRS = libspindle tests sample_apps

EXTRA_DIST = makeIncludes.sh

dist-hook:
	d="$(distdir)/include/samples";\
	test -x $$d \
	|| mkdir -p $$d \
	|| exit 1; \
	cp $(srcdir)/include/samples/*.* $$d/; 
