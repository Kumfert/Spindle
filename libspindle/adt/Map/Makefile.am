INCLUDES = -I$(top_srcdir)/include 

noinst_LIBRARIES = libjunk.a
libjunk_a_SOURCES = \
	MapUtils.cc \
	CompressionMap.cc \
	PermutationMap.cc \
	ScatterMap.cc 
	 
includedir = $(prefix)/include/spindle
include_HEADERS = $(libjunk_a_SOURCES:.cc=.h)
