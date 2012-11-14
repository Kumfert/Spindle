INCLUDES =  -I$(top_srcdir)/include

noinst_LIBRARIES = libjunk.a
libjunk_a_SOURCES = \
	GraphCompressor.cc \
	BreadthFirstSearch.cc \
	PseudoDiameter.cc \
	SymbolicFactorization.cc 

includedir = $(prefix)/include/spindle
include_HEADERS = $(libjunk_a_SOURCES:.cc=.h)
