INCLUDES = -I$(top_srcdir)/include

noinst_LIBRARIES = libjunk.a
libjunk_a_SOURCES = \
	GraphBase.cc \
	GraphUtils.cc \
	Graph.cc \
	QuotientGraph.cc

includedir = $(prefix)/include/spindle
include_HEADERS = $(libjunk_a_SOURCES:.cc=.h)
