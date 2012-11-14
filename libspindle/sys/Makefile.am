INCLUDES =  -I$(top_srcdir)/include

noinst_LIBRARIES = libjunk.a
libjunk_a_SOURCES = \
	ClassMetaData.cc \
	SpindleSystem.cc \
	PersistanceRegistry.cc \
	CommandLineOptions.cc \
	OptionDatabase.cc \
	StackTrace.cc

includedir = $(prefix)/include/spindle
include_HEADERS = $(libjunk_a_SOURCES:.cc=.h) SpindleErrorCodes.h SpindleTraceCodes.h stopwatch.h
