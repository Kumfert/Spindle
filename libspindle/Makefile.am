SUBDIRS = std sys adt io util order
INCLUDES =  -I$(top_srcdir)/include
libdir = $(exec_prefix)/lib/@host@
lib_LIBRARIES = libspindle.a
libspindle_a_SOURCES = 
libspindle_a_LIBADD = \
 std/SpindleBaseClass.o\
 std/SpindlePersistant.o\
 std/SpindleAlgorithm.o\
 sys/OptionDatabase.o\
 sys/PersistanceRegistry.o\
 sys/StackTrace.o\
 sys/ClassMetaData.o\
 sys/SpindleSystem.o\
 sys/CommandLineOptions.o\
 adt/BucketSorter/ArrayBucketSorter.o\
 adt/Forest/GenericForest.o\
 adt/Forest/EliminationForest.o\
 adt/Graph/GraphBase.o\
 adt/Graph/GraphUtils.o\
 adt/Graph/Graph.o\
 adt/Graph/QuotientGraph.o\
 adt/Heap/BinaryHeap.o\
 adt/Map/MapUtils.o\
 adt/Map/CompressionMap.o\
 adt/Map/PermutationMap.o\
 adt/Map/ScatterMap.o\
 adt/Matrix/MatrixBase.o\
 adt/Matrix/MatrixUtils.o\
 adt/Matrix/Matrix.o\
 io/FortranEmulator.o\
 io/Tokenizer.o\
 io/SpindleFile.o\
 io/SpindleArchive.o\
 io/TextDataFile.o\
 io/ChacoFile.o\
 io/HarwellBoeingFile.o\
 io/MatrixMarketFile.o\
 io/SpoolesFile.o\
 io/DobrianFile.o\
 io/ChristensenFile.o\
 io/GraphMatrixFileFactory.o\
 util/GraphCompressor.o\
 util/BreadthFirstSearch.o\
 util/PseudoDiameter.o\
 util/SymbolicFactorization.o\
 order/OrderingAlgorithm.o\
 order/RCMEngine.o\
 order/SloanEngine.o\
 order/MinPriorityEngine.o\
 order/MinPriorityStrategies.o 
