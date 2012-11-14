INCLUDES =  -I$(top_srcdir)/include

noinst_LIBRARIES = libjunk.a
libjunk_a_SOURCES = SpindleBaseClass.cc SpindlePersistant.cc SpindleAlgorithm.cc   

includedir = $(prefix)/include/spindle
include_HEADERS = $(libjunk_a_SOURCES:.cc=.h) \
		spindle_algo.h spindle_func.h triple.h \
		SharedPtr.h  SharedArray.h spindle.h
