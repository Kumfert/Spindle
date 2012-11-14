INCLUDES =  -I$(top_srcdir)/include

noinst_LIBRARIES = libjunk.a
libjunk_a_SOURCES = 	\
	FortranEmulator.cc \
	Tokenizer.cc \
	SpindleFile.cc \
	SpindleArchive.cc \
	TextDataFile.cc \
	ChacoFile.cc \
	HarwellBoeingFile.cc \
	MatrixMarketFile.cc \
	SpoolesFile.cc \
	DobrianFile.cc \
	ChristensenFile.cc\
	GraphMatrixFileFactory.cc 

includedir = $(prefix)/include/spindle
include_HEADERS = $(libjunk_a_SOURCES:.cc=.h)
