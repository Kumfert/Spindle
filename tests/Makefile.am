SUBDIRS = libspindle

# The perl directory
PERL=@PERL@

# perl include paths.
PERLINCL= -I$(srcdir) -I. -I..

# the command to run all the tests
RUNPERLTEST= $(PERL) $(PERLINCL) -e 'use petf::Auditor qw(&runtests $$srcdir); $$srcdir="$(srcdir)"; runtests( @ARGV );'

# perl modules for the Perl Enabled Testing Framework (PETF)
PETF_MODULES = petf/Auditor.pm petf/Reporter.pm petf/Runner.pm

# all test scripts that are run with 'make check'
ALLTESTS = $(OLDUNITTESTS)

# all scripts that are in distribution
ALLSCRIPTS = $(ALLTESTS) $(PETF_MODULES)

OLDUNITTESTS =  libspindle/std/old_unit.sh\
 libspindle/sys/old_unit.sh\
 libspindle/adt/BucketSorter/old_unit.sh\
 libspindle/adt/Forest/old_unit.sh\
 libspindle/adt/Graph/old_unit.sh\
 libspindle/adt/Map/old_unit.sh\
 libspindle/adt/Matrix/old_unit.sh \
 libspindle/util/old_unit.sh\
 libspindle/order/old_unit.sh

check-local: #verify-scripts
	if test "X$(PERL)" = X; then \
 	  echo "WARNING: Cannot run tests without perl 5.004 or greater" ;\
	else \
	  echo "$(RUNPERLTEST) check \$$(ALLTESTS)"; \
	  $(RUNPERLTEST) check $(ALLTESTS) ;\
	fi 	

#
# I don't want want to put Makefile.am's everywhere, and
# I'll handle 'make check' from here.  So, I just need to
# make sure that the necessary files are copied into the 
# distribution
#
dist-hook:	
	 for scriptname in $(PETF_MODULES) ; do \
	   d=`dirname $(distdir)/$$scriptname`; \
	   test -x $$d \
	   || mkdir -p $$d \
	   || exit 1; \
	   cp -p $(srcdir)/$$scriptname $(distdir)/$$scriptname; \
	 done


#
# For building outside the source directory: Need to confirm
# that all the scripts are in place.
#
#verify-scripts:
#	 olddirname=junk; \
#	 for scriptname in $(ALLSCRIPTS) ; do \
#	   d=`dirname $(builddir)/tests/$$scriptname` ; \
#	   if test $$d != $$olddirname; then \
#	     if test ! -x $$d ; then \
#	       mkdir -p  $$d ; \
#	     fi ;\
#	   fi; \
#	   if test ! -s  $(builddir)/tests/$$scriptname ; then \
#	     cp -p $(srcdir)/$$scriptname $(builddir)/tests/$$scriptname; \
#	   fi ; \
#	 done ; \
#	 find $(srcdir) -name "test*.out" -exec cp {} $(builddir)/tests \; \
#	 find $(srcdir) -name "test*.in" -exec cp {} $(builddir)/tests \; 
#
