//
// src/order/drivers/examples/ex02C.cc -- MinPriorityEngine
//
// $Id: greedyfillredux.cc,v 1.1 2000/03/13 22:16:07 kumfert Exp $
//
//  Gary Kumfert, Old Dominion University
//  Copyright(c) 1997-1998, Old Dominion University.  All rights reserved.
// 
//  Permission to use, copy, modify, distribute and sell this software and
//  its documentation for any purpose is hereby granted without fee, 
//  provided that the above copyright notice appear in all copies and
//  that both that copyright notice and this permission notice appear
//  in supporting documentation.  Old Dominion University makes no
//  representations about the suitability of this software for any 
//  purpose.  It is provided "as is" without express or implied warranty.
//
///////////////////////////////////////////////////////////////////////
//
//

#include <fstream.h>
#include <iomanip.h>

static char help_string1[] = "Usage:\n\t ";
static char help_string2[] = " [ -h | -help ] [ -help-fintype ] \n"
"    [ -help-priority ] [ -help-permformat ] [ -fin <inputfile> ] \n"
"    [ -fout <outputfile> | stdout \n"
"            [ -permtype new2old | old2new ] \n"
"            [ -permformat  <option> ]\n"
"    ] \n"
"    [ -fintype Chaco | MeTiS | HB | Dobrian ] \n"
"    [ -priority exact | mmd | amd | amf | ammf | mammf | amind | mamind | \n"
"            mmdf | mmmd | hybrid [ <float> ] | advhybrid <int> <int> <policy> ]\n"
"    [ -cRatio <float> ]  [ -s ] [ -t | -time ] [ -log ] \n"
"    [ -heavyVtxRatio <float> ] [ -s ] [ -t | -time ] [ -log ]\n"
"    [ -cNAdjClqBnd <int> ] [ -cMixAdjVtxs on | off ]\n"
"    [ -delta <int> ] [ -dv <vertexfile> [-dw <int>] ]\n"
"    [ -randseed <int> [ <int> [...] ] ] [ -etree <etreeFile> | stdout ]\n"
"    [ -trace [ qgraph ] [ qgraph-upd ]  [ priority-scheme ] \n"
"             [ hybrid-switch ] [advhybrid] \n"
"    ]\n"
"    [ -enableIncrementByOne ]\n"
"--------------------------------------------------------------------------------\n"
"   -h | -help            : display this screen\n"
"   -help-fintype         : extended info about file types\n"
"   -help-priority        : extended info about degree policies\n"
"   -help-permformat      : extended information about permutation outputs\n"
"   -fintype  <fileType>  : specify sparse matrix input file format (default Chaco)\n"
"   -priority <policy>    : specify how priority is computed ( default extern )\n"
"                           ( exact | extern | approx | amf | mmdf | mmmd )\n"
"   -cRatio <float>       : use compressed graph if compression ratio\n"
"  		            (of vertices) is less than <float>.\n"
"                         : 0.0 implies no compression. default==0.9\n"
"   -cNAdjClqBnd <int>    : restrict quotient graph compression of n-adj vtxs\n"
"                           to those <= <int>  (DISABLED)\n"
"   -cMixAdjVtxs <on|off> : override quotient graph compression of mixed adj vtxs\n"
"                           to forced on or off (DISABLED)\n"
"   -heavyVtxRatio <float>: any vertices that have over <float>*n edges\n"
"                           are removed a priori, and numbered last\n"
"   -delta <int>          : specify min priority tolerance ( default 0 ) \n"
"   -fin <inputfile>      : specify filename on commandline\n"
"   -fout <outputfile>    : display permutation vector to a file\n"
"   -fout stdout          : display permutation vector to standard out\n"
"  			    (else don\'t display permutation)\n"
"   -permtype new2old|old2new : force permutation vector to be old-to-new or\n"
"                           new-to-old.  (default is new-to-old).\n"
"   -permformat <option>  : sets permutation to have range 0..(n-1) or 1..n\n"
"                           and allows length of permutation to be prepended\n"
"                           use \'-help-permtype\' option for more details\n"
"   -s | -stats           : show quality statistics\n"
"   -t | -time            : show timing information\n" 
"   -log                  : display (factor-size work time) triple only \n"
"   -dv <vertexListFile>  : delay vertices in file (DISABLED)\n"
"   -dw <int>             : set delay weight (DISABLED)\n"
"   -trace <optionlist>   : enables trace of certain options: (see above )\n"
"                           NOTE: trace may be disabled in optimized compilations\n"
"   -randseed <int> [...] : seed random number generator \n"
"                         : if <int> == -1, then no randomization takes place \n"
"                           NOTE: \"-randseed <int> <int> <int>\" is valid\n"
"   -etree <etreeFile>    : store the etree (parent pointers) in a file\n"
"   -etree stdout         : print the etree (parent pointers) to stdout\n"
"   -enableIncrementByOne : This is an undocumented detail about mmd \n";

static char help_priority[] =
" Priority Policies:\n"
"\t exact  : (m) Multiple Minimum Degree (exact degree) -- George and Liu \n"
"\t mmd    : (m) Multiple Minimum Degree (external degree) -- Liu.\n"
"\t amd    : (s) Approximate Minimum Degree -- Amestoy, Davis, andDuff.\n"
"\t amf    : (m) Approximate Minimum Fill -- Rothberg.\n"
"\t ammf   : (s) Approximate Minimum Mean local Fill -- Rothberg and Eisenstat.\n"
"\t amind  : (s) Approximate Minimum Increase in Neighbor Degree -- Rothberg and Eisenstat.\n"
"\t mammf  : (m) Multiple Approximate Minimum Mean local Fill -- Rothberg and Eisenstat.\n"
"\t mamind : (m) Multiple Approximate Minimum Increase in Neighbor Degree -- Rothberg and Eisenstat.\n"
"\t mmdf   : (m) Multiple Minimum Deficiency -- Ng and Raghavan.\n"
"\t mmmd   : (m) Modified Multiple Minimum Degree -- Ng and Raghavan.\n"
"\t hybrid <float> : starts as MMD and switches to AMD after [float] \n"
"\t                  percent of permutation is determined\n"
"\t advhybrid <int1> <int2> <policy> : \n"
"\t        : an advanced hybrid that starts with a multiple-elimination policy\n"
"\t        : but switched to its single elimination variant after <int2> successive\n"
"\t        : times that the number of supernodes that are mass-eliminated falls\n"
"\t        : less than <int1> supernodes.\n"
"\t        : <policy> can be one of \n"
"\t              md -- minimum degree\n"
"\t              mmf -- minimum mean fill\n"
"\t              mind -- minimum increase in neighbor degree\n"
"\t timedhybrid <float1> <policy> : \n"
"\t        : a hybrid  that starts with a multiple-elimination policy\n"
"\t        : but switches to its single elimination when a condition is met."
"\t        : This hybrid determines the initial ratio of supernodes eliminated in last"
"\t        : mass elimination/time.  If this ratio slips below <float1>*initial ratio"
"\t        : the switching condition has been met.\n"
"\t        : <policy> can be one of \n"
"\t              md -- minimum degree\n"
"\t              mmf -- minimum mean fill\n"
"\t              mind -- minimum increase in neighbor degree\n";

static char help_permformat[] =
" PermutationMap Output Formats:\n"
"\t C       : \'C style\' Range from zero to n-1\n"
"\t F77     : \'Fortran style\' Range from 1 to n\n"
"\t len+C   : the length of the permutation, then C-style\n"
"\t len+F77 : the length of the permutation, the Fortran-style\n";

#include <iostream.h>
#include "spindle/OptionDatabase.h"
#include "spindle/Graph.h"
#include "spindle/GraphCompressor.h"
#include "spindle/SymbolicFactorization.h"
#include "spindle/ArrayBucketSorter.h"
#include "spindle/MinPriorityEngine.h"
#include "spindle/MinPriorityStrategies.h"
#include "spindle/GraphMatrixFileFactory.h"
#include "spindle/EliminationForest.h"
#include "spindle/SloanEngine.h"
#include "spindle/RCMEngine.h"

#ifdef SPINDLE_DISABLE_STOPWATCH
#define stopwatch mystopwatch
#undef SPINDLE_STOPWATCH_H_
#include "spindle/solaris_stopwatch.h"
#endif

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

int main(int argc, char *argv[]) {
  ios::sync_with_stdio();
  SpindleSystem::setErrorFile( stdout );
  SpindleSystem::setTraceFile( stdout );

  ///////////////////////////////////////////////////////////////////////////
  //
  //  1.  Parse command line options
  //
  OptionDatabase cmdLine;
  cmdLine.loadCommandLineOptions( argc, argv );
  bool terminate = false; 
  if ( cmdLine.hasOption("h|help") ) {
    cerr << help_string1 << argv[0] << help_string2;
    terminate = true;
  }
  if ( cmdLine.hasOption("help-fintype") ) {
    cerr << GraphMatrixFileFactory::getHelpString();
    terminate = true;
  }
  if ( cmdLine.hasOption("help-priority") ) { 
    cerr << help_priority;
    terminate = true;
  }
  if ( cmdLine.hasOption("help-permformat") ) { 
    cerr << help_permformat;
    terminate = true;
  }
  if ( terminate ) { 
    exit( 0 );
  }

  if ( cmdLine.hasOption("trace") ) {
    SpindleSystem::setTraceFile( stdout );
    int k=cmdLine.queryNArgs("trace");
    for(int i=0; i<k; ++i ) { 
      const char* option = cmdLine.getOption( "trace", i );
      unsigned int traceLevel = SpindleSystem::getTraceLevel();
      if ( !strcmp( option, "qgraph") ) {
	SpindleSystem::setTraceLevel( traceLevel | SPINDLE_TRACE_QGRAPH );
      } else if ( !strcmp( option, "qgraph-upd" ) ) {
	SpindleSystem::setTraceLevel( traceLevel | SPINDLE_TRACE_QGRAPH_UPD );
      } else if ( !strcmp( option, "priority-scheme" ) ) {
	SpindleSystem::setTraceLevel( traceLevel | SPINDLE_TRACE_MINPRIORITY_SCHEME );
      } else if ( !strcmp( option, "hybrid-switch" ) ) {
	SpindleSystem::setTraceLevel( traceLevel | SPINDLE_TRACE_MINPRIORITY_STRATEGY_SWITCH );
      } else if ( !strcmp( option, "advhybrid" ) ) {
	SpindleSystem::setTraceLevel( traceLevel | SPINDLE_TRACE_MINPRIORITY_STRATEGY_ADVHYBRID );
      } else { 
	cerr << " Error: could not understand -trace option" << option << endl;
      }
    }
  }

  ////////////////////////////////////////////////////////////////////////////
  //
  // 2.  Create graph based on file type
  //

  GraphMatrixFileFactory factory;
  string filename;
  string filetype;
  if ( cmdLine.hasOption("fin") ) { 
    filename = cmdLine.getOption("fin");
  } else { 
    cerr << "Error: must specify input file using \"--fin\". "<< endl;
    cerr << "       Try \"--help\" for more information." << endl;
    exit(-1);
  }
  if ( cmdLine.hasOption("fintype") ) { 
    filetype = cmdLine.getOption("fintype");
  } else { 
    cerr << "Warning: no file type specfied, Chaco assumed, else use \"--fintype\". "<< endl;
    filetype = "Chaco";
  }
  Graph* graph=0;
  if ( factory.loadFile( filename, filetype ) ) { 
    graph = factory.spawnGraph();
  }
	 
  //////////////////////////////////////////////////////////////////////
  //
  // 3. Validate the graph
  //
  if ( graph == 0 ) { 
    cerr << "ERROR: cannot validate a null graph pointer." << endl;
    exit(-1);
  }
  graph->validate();
  if ( ! graph->isValid() ) {
    cerr << "ERROR: cannot create valid graph from this input." << endl;
    exit(-1);
  }

  ////////////////////////////////////////////////////////////
  //
  // iterate
  //
  int nIterations = 1;
  if ( cmdLine.hasOption("randseed") ) {
    nIterations = cmdLine.queryNArgs( "randseed" );
  }
  for ( int iteration = 0; iteration < nIterations; ++iteration ) {
    
    ////////////////////////////////////////////////////////////////////////
    //
    // 5. Create a Minimum degree ordering context.
    //

    MinPriorityEngine ordering(graph);    
    if ( cmdLine.hasOption("cRatio") ) {
      float targetRatio = atof( cmdLine.getOption("cRatio") );
      if (!ordering.setCompressionRatio( targetRatio ) ) { 
	cerr << "Attempt to adjust Compression Ratio to " << targetRatio
	     << " failed." << endl;
      }
    }
    // (set any options here)
    
    int seed = -1;
    if ( cmdLine.hasOption("randseed") ) {
      const char * strseed = cmdLine.getOption("randseed",iteration);
      if ( strseed != 0 ) { 
	if ( !strcmp( strseed, "sloan") ) { 
	  // do a sloan preordering;
	  SloanEngine sloan(graph);
	  sloan.execute();
	  const PermutationMap * perm = sloan.getPermutation();
	  if ( !perm->isValid() ) {
	    cerr << "Error in creating valid sloan pre-ordering, ignoring" << endl;
	  } else { 
	    ordering.setExplicitOrder( *perm );
	  }
	} else if ( !strcmp( strseed, "rcm" ) ) { 
	  // do an rcm preordering;
	  RCMEngine rcm(graph);
	  rcm.execute();	  
	  const PermutationMap * perm = rcm.getPermutation();
	  if ( !perm->isValid() ) {
	    cerr << "Error in creating valid sloan pre-ordering, ignoring" << endl;
	  } else { 
	    ordering.setExplicitOrder( *perm );
	  }
	} else {  // if strseed may be numeric
	  seed = atoi( strseed );
	  if ( seed == -1 ) { 
	    ordering.setRandomizeGraph( false );
	  } else {  // if seed != -1
	    unsigned int u_seed = seed;
#ifdef __STL_NO_DRAND48
	    srand( u_seed );
#else
	    srand48( u_seed );
#endif
	  } // end else (if seed != -1)
	} // end else (if strseed may be numeric)
      } // end if strseed != 0
    } // end if -randseed 
    
    if ( cmdLine.hasOption("enableIncrementByOne") ) { 
      ordering.enableIncrementByOne();
    }
    if ( cmdLine.hasOption("priority") ) {
      if ( cmdLine.hasOption("priority","exact") ) {
	ordering.setPriorityStrategy( new MinPriority_ExactDegree( graph->size() ) );
      } else if ( cmdLine.hasOption("priority", "mmd") ) {
	ordering.setPriorityStrategy( new MinPriority_ExternalDegree( graph->size() ) );
      } else if ( cmdLine.hasOption("priority", "amd") ) {
	ordering.setPriorityStrategy( new MinPriority_ApproximateDegree( graph->size() ) );
      } else if ( cmdLine.hasOption("priority", "amf") ) {
	ordering.setPriorityStrategy( new MinPriority_AMF( graph->size() ) );
      } else if ( cmdLine.hasOption("priority", "ammf") ) {
	ordering.setPriorityStrategy( new MinPriority_AMMF( graph->size() ) );
      } else if ( cmdLine.hasOption("priority", "mmind") ) {
	ordering.setPriorityStrategy( new MinPriority_MMIND( graph->size() ) );
      } else if ( cmdLine.hasOption("priority", "mmmf") ) {
	ordering.setPriorityStrategy( new MinPriority_MMMF( graph->size() ) );
      } else if ( cmdLine.hasOption("priority", "amind") ) {
	ordering.setPriorityStrategy( new MinPriority_AMIND( graph->size() ) );
      } else if ( cmdLine.hasOption("priority", "mmdf") ) {
	ordering.setPriorityStrategy( new MinPriority_MMDF( graph->size() ) );
      } else if ( cmdLine.hasOption("priority", "mmmd") ) {
	ordering.setPriorityStrategy( new MinPriority_MMMD( graph->size() ) );
      } else if ( cmdLine.hasOption("priority", "hybrid") ) {
	float switch_point = 0.5;
	const char * string_switch = cmdLine.getOption("priority",1);
	if ( (string_switch != 0) && (string_switch[0] != '-' ) ) { 
	  switch_point = atof( string_switch );
	}
	ordering.setPriorityStrategy( 
		new  MinPriority_MMD_AMD_SimpleHybrid( graph->size(), switch_point ) );
      } else if ( cmdLine.hasOption("priority", "advhybrid" ) ) { 
	int tol = 5;
	const char * temp_string = cmdLine.getOption("priority",1);
	if ( (temp_string != 0 ) && ( temp_string[0] != '-' ) ) { 
	  tol = atoi( temp_string);
	}
	int nSubopt = 5;
	temp_string = cmdLine.getOption("priority", 2 );
	if ( ( temp_string != 0 ) && ( temp_string[0] != '-' ) ) { 
	  nSubopt = atoi( temp_string );
	}
	MinPriority_AdvancedHybrid * adv_strategy = new 
	  MinPriority_AdvancedHybrid( graph->size(), tol, nSubopt );
	temp_string = cmdLine.getOption("priority", 3 );
	if ( ( temp_string != 0 ) && ( temp_string[0] != '-' ) ) { 
	  ; // do nothing
	} else { 
	  temp_string = "md";
	}
	if ( !strcmp( temp_string, "mmf") ) { 
	  adv_strategy->setMultipleEliminationStrategy( 
			new MinPriority_MMMF( graph->size() ) );
	  adv_strategy->setApproximateEliminationStrategy( 
                        new MinPriority_AMMF( graph->size() ) );
	} else if ( !strcmp( temp_string,"mind" ) ) { 
	  adv_strategy->setMultipleEliminationStrategy( 
			new MinPriority_MMIND( graph->size() ) );
	  adv_strategy->setApproximateEliminationStrategy( 
                        new MinPriority_AMIND( graph->size() ) );
	} else { 
	  adv_strategy->setMultipleEliminationStrategy( 
			new MinPriority_ExternalDegree( graph->size() ) );
	  adv_strategy->setApproximateEliminationStrategy( 
                        new MinPriority_ApproximateDegree( graph->size() ) );
	}
	ordering.setPriorityStrategy( adv_strategy );
      } else if ( cmdLine.hasOption("priority", "advhybrid2" ) ) { 
	float tol = 0.1;
	const char * temp_string = cmdLine.getOption("priority",1);
	if ( (temp_string != 0 ) && ( temp_string[0] != '-' ) ) { 
	  tol = atof( temp_string);
	}
	int history = 5;
	temp_string = cmdLine.getOption("priority", 2 );
	if ( ( temp_string != 0 ) && ( temp_string[0] != '-' ) ) { 
	  history = atoi( temp_string );
	}
	MinPriority_AdvancedHybrid2 * adv_strategy = new 
	  MinPriority_AdvancedHybrid2( graph->size(), tol, history );
	temp_string = cmdLine.getOption("priority", 3 );
	if ( ( temp_string != 0 ) && ( temp_string[0] != '-' ) ) { 
	  ; // do nothing
	} else { 
	  temp_string = "md";
	}
	if ( !strcmp( temp_string, "mmf") ) { 
	  adv_strategy->setMultipleEliminationStrategy( 
			new MinPriority_MMMF( graph->size() ) );
	  adv_strategy->setApproximateEliminationStrategy( 
                        new MinPriority_AMMF( graph->size() ) );
	} else if ( !strcmp( temp_string,"mind" ) ) { 
	  adv_strategy->setMultipleEliminationStrategy( 
			new MinPriority_MMIND( graph->size() ) );
	  adv_strategy->setApproximateEliminationStrategy( 
                        new MinPriority_AMIND( graph->size() ) );
	} else { 
	  adv_strategy->setMultipleEliminationStrategy( 
			new MinPriority_ExternalDegree( graph->size() ) );
	  adv_strategy->setApproximateEliminationStrategy( 
                        new MinPriority_ApproximateDegree( graph->size() ) );
	}
	ordering.setPriorityStrategy( adv_strategy );
      } else if ( cmdLine.hasOption("priority", "timedhybrid" ) ) { 
	float tol = 0.01;
	const char * temp_string = cmdLine.getOption("priority",1);
	if ( (temp_string != 0 ) && ( temp_string[0] != '-' ) ) { 
	  tol = atof( temp_string);
	}
	MinPriority_TimedHybrid * timed_hybrid = new 
	  MinPriority_TimedHybrid( graph->size(), tol );
	temp_string = cmdLine.getOption("priority", 2 );
	if ( ( temp_string != 0 ) && ( temp_string[0] != '-' ) ) { 
	  ; // do nothing
	} else { 
	  temp_string = "md";
	}
	if ( !strcmp( temp_string, "mmf") ) { 
	  timed_hybrid->setMultipleEliminationStrategy( 
			new MinPriority_MMMF( graph->size() ) );
	  timed_hybrid->setApproximateEliminationStrategy( 
                        new MinPriority_AMMF( graph->size() ) );
	} else if ( !strcmp( temp_string,"mind" ) ) { 
	  timed_hybrid->setMultipleEliminationStrategy( 
			new MinPriority_MMIND( graph->size() ) );
	  timed_hybrid->setApproximateEliminationStrategy( 
                        new MinPriority_AMIND( graph->size() ) );
	} else { 
	  timed_hybrid->setMultipleEliminationStrategy( 
			new MinPriority_ExternalDegree( graph->size() ) );
	  timed_hybrid->setApproximateEliminationStrategy( 
                        new MinPriority_ApproximateDegree( graph->size() ) );
	}
	ordering.setPriorityStrategy( timed_hybrid );
      } else {
	const char* unknown = cmdLine.getOption("priority");
	cerr << "Error: Unknown Option \" -priority " << unknown << "\" ignored." << endl;
      }
    }
    if ( cmdLine.hasOption("delta") ) {
      int delta = atoi( cmdLine.getOption("delta") );
      if ( !ordering.setDelta( delta ) ) {
	cout << "Warning: setDelta has failed." << endl;
      }
    }
    
    if ( cmdLine.hasOption( "heavyVtxRatio" ) ) { 
      float heavyVtxRatio = atof( cmdLine.getOption("heavyVtxRatio") );
      ordering.setHeavyVertexTolerance( heavyVtxRatio );
    }
    /*
    if ( cmdLine.hasOption("dv") ) {
      ifstream vertexStream( cmdLine.getOption("dv") );
      if (!vertexStream) {
	cerr << "Error: Could not open file \"" << cmdLine.getOption("dv") 
	     << "\" ... Ignoring delayed Vertex Constraint..." << endl;
      } else {
	MinPriorityEngine::VertexList delayedVertices;
	copy( istream_iterator<int,ptrdiff_t>(vertexStream), 
	      istream_iterator<int,ptrdiff_t>(),
	      back_inserter( delayedVertices ) );
	ordering.enableDelayedVertices( delayedVertices );
	if ( cmdLine.hasOption("dw") ) {
	  int delayWeight = atoi( cmdLine.getOption("dw") );
	  if ( (delayWeight > 0) && (delayWeight < graph->size()) ) {
	    ordering.setDelayWeight( delayWeight );
	  }
	} // end if -dw
      } // end else
    } // end if -d 
  if ( cmdLine.hasOption( "cNAdjClqBnd" ) ) { 
    ordering.setNAdjClqCompressionBound( atoi( cmdLine.getOption( "cNAdjClqBnd" ) ) );
  }
  if ( cmdLine.hasOption( "cMixAdjVtxs" ) ) { 
    if ( cmdLine.hasOption(  "cMixAdjVtxs", "on" ) ) {
      ordering.setMixAdjCompression( 1 );
    } else if ( cmdLine.hasOption(  "cMixAdjVtxs", "off" ) ) {
      ordering.setMixAdjCompression( 0 );
    }
  }
  */

    //////////////////////////////////////////////////////////////////////
    //
    // 5. Compute the ordering
    //
#ifdef SPINDLE_DISABLE_STOPWATCH
    stopwatch timer;
    timer.start();
#endif
    ordering.execute();
#ifdef SPINDLE_DISABLE_STOPWATCH
    timer.stop();
#endif
    if ( ordering.notDone() ) { 
      cerr << "Error, ordering returned prematurely" << endl;
      exit (-1);
    }
    
    //////////////////////////////////////////////////////////////////////
    //
    // 6. Create the uncompressed permutation 
    //
    const PermutationMap * perm = ordering.getPermutation();
    if ( !perm->isValid() ) {
      cerr << "Error in creating valid permutation from ordering" << endl;
      continue;
    }
    stopwatch symbFactTotalTimer;
    stopwatch symbFactSetGraphTimer;
    stopwatch symbFactSetPermTimer;
    stopwatch symbFactExecutionTimer;

    
    //////////////////////////////////////////////////////////////////////
    //
    // 7. Do the Symbolic factorization
    //
    SymbolicFactorization symbfact;  // create the SymbolicFactorization object
    {
      symbFactTotalTimer.start();
      // set the graph
      symbFactSetGraphTimer.start();
      if ( ! symbfact.setGraph( graph ) ) {
	cerr << "Error setting Graph" << endl;
	cerr << symbfact.queryErrMsg() << endl;
	continue;
      }
      symbFactSetGraphTimer.stop();
      
      // set the uncompressed permutation
      symbFactSetPermTimer.start();
      if ( ! symbfact.setPermutationMap( perm ) ) {
	cerr << "Error seting permutation in symbolic factorization" << endl;
	cerr << symbfact.queryErrMsg() << endl;
	continue;
      }
      symbFactSetPermTimer.stop();
      
      symbFactExecutionTimer.start();
      if ( ! symbfact.execute() ) {
	cerr << "Error doing symbolic factorization on unordered graph." << endl;
	cerr << symbfact.queryErrMsg() << endl;
	continue;
      }
      symbFactExecutionTimer.stop();
      symbFactTotalTimer.stop();
    }
    // perm no longer needed.  symfact creates its own
    // permutation based on the postorder of the tree.
    
    //////////////////////////////////////////////////////////////////////
    //
    // 8. Get the parent pointers of the etree (old ordering)
    //
    if ( cmdLine.hasOption("etree") ) {
      FILE* output_file = NULL;
      const int* etree = symbfact.getETree()->getParent().lend();
      
      if ( cmdLine.hasOption("etree","stdout") ) {
	output_file = stdout;
      } else {
	const char* output_file_name = cmdLine.getOption("etree");
	if (( output_file = fopen( output_file_name,"w")) == NULL ) { 
	  cerr << "Error: could not open output file \'" << output_file_name << "\'" << endl;
	}
      }
      const int nvtxs = graph->size();
      
      for (int i=0; i<nvtxs; i++) {
	fprintf(output_file,"%d\n",etree[i]);
      } 
      if (output_file != stdout) {
	fclose(output_file);
      }
    }
    
    //////////////////////////////////////////////////////////////////////
    //
    // 9. Display the permutation vector, if requested
    //
    if ( cmdLine.hasOption("fout") ) {
      FILE* output_file = NULL;
      const PermutationMap * new_perm = symbfact.getPermutationMap().lend();
      
      if ( cmdLine.hasOption("fout","stdout") ) {
	output_file = stdout;
      } else {
	const char* output_file_name = cmdLine.getOption("fout");
	if (( output_file = fopen( output_file_name,"w")) == NULL ) { 
	  cerr << "Error: could not open output file \'" << output_file_name << "\'" << endl;
	}
      }
      
      bool printNew2Old = ( cmdLine.hasOption("permtype","old2new") ) ?  false : true;
      bool onesOffset = false;
      bool printLength = false;
      if ( cmdLine.hasOption("permformat","F77") ) {
	onesOffset = true;
      } else if ( cmdLine.hasOption("permformat","len+C") ) {
	printLength = true;
      } else if ( cmdLine.hasOption("permformat","len+F77") ) {
	onesOffset = true;
	printLength = true;
      }

      new_perm->prettyPrint( output_file, printNew2Old, printLength, onesOffset );

      if (output_file != stdout) {
	fclose(output_file);
      }
    }
    
    //////////////////////////////////////////////////////////////////////
    //
    // 10. Display timing information, if requested.
    //
    if ( cmdLine.hasOption("t|time") ) {
#ifdef SPINDLE_DISABLE_STOPWATCH
      cout << "Timing disabled.  Use -log for minimal statistics" << endl;
#else
      int detailLevel = 6;
      if ( cmdLine.hasOption("t|time", "5" ) ) {
	   detailLevel = 5;
      } else if ( cmdLine.hasOption("t|time", "4" ) ) {
	   detailLevel = 4;
      } else if ( cmdLine.hasOption("t|time", "3" ) ) {
	   detailLevel = 3;
      } else if ( cmdLine.hasOption("t|time", "2" ) ) {
	   detailLevel = 2;
      } else if ( cmdLine.hasOption("t|time", "1" ) ) {
	   detailLevel = 1;
      } else if ( cmdLine.hasOption("t|time", "0" ) ) {
	   detailLevel = 0;
      } 
      const QuotientGraph& qgraph = ordering.lendQuotientGraph();
      const GraphCompressor * compressor = ordering.getGraphCompressor();
      stopwatch timer;
      float ticksPerSec = timer.queryTicksPerSec();
      cout << "Timing Information (seconds) user / system :" << endl;
      if ( ordering.isCompressed() ) { 
	if ( detailLevel > 1 )
	  cout << "   compression                               : "
	       << ( compressor->getExecutionTimer().queryTotalUserTicks() +
		    compressor->getCGraphCreationTimer().queryTotalUserTicks() )/ticksPerSec << " / " 
	       << ( compressor->getExecutionTimer().queryTotalSystemTicks() +
		    compressor->getCGraphCreationTimer().queryTotalSystemTicks() )/ticksPerSec << endl;
	if ( detailLevel > 1 )
	  cout << "      map generation                            : " 
	       << compressor->getExecutionTimer().queryTotalUserTicks()/ticksPerSec << " / " 
	       << compressor->getExecutionTimer().queryTotalSystemTicks()/ticksPerSec << endl;
	if ( detailLevel > 1 )
	  cout << "      graph creation/validation                 : " 
	       << compressor->getCGraphCreationTimer().queryTotalUserTicks()/ticksPerSec << " / " 
	       << compressor->getCGraphCreationTimer().queryTotalSystemTicks()/ticksPerSec << endl;
      } else { 
	if ( detailLevel > 0 )
	  cout << "   compression (not used)                    : " 
	       << compressor->getExecutionTimer().queryTotalUserTicks()/ticksPerSec << " / " 
	       << compressor->getExecutionTimer().queryTotalSystemTicks()/ticksPerSec << endl;
      }
      if ( detailLevel > 0 )
	cout << "   minimum priority ordering                 : " 
	     << ordering.getTimer().queryTotalUserTicks()/ticksPerSec << " / " 
	     << ordering.getTimer().queryTotalSystemTicks()/ticksPerSec << endl;
      if ( detailLevel > 1 )
	cout << "      priority computation                      : "
	     << ordering.lendDegreeComputationTimer().queryTotalUserTicks()/ticksPerSec 
	     << " / "
	     << ordering.lendDegreeComputationTimer().queryTotalSystemTicks()/ticksPerSec 
	     << endl;
      if ( detailLevel > 1 )
	cout << "      step time  (1 snode elimination/step)     : "
	     << ordering.lendStepTimer().queryTotalUserTicks()/ticksPerSec << " / "
	     << ordering.lendStepTimer().queryTotalSystemTicks()/ticksPerSec << endl;
      if ( detailLevel > 2 )
	cout << "         qgraph->eliminateSupernodes()             : "
	     << ( qgraph.lendEliminateSupernodeTimer()[0].queryTotalUserTicks() + 
		  qgraph.lendEliminateSupernodeTimer()[1].queryTotalUserTicks() + 
		  qgraph.lendEliminateSupernodeTimer()[2].queryTotalUserTicks() )/ticksPerSec 
	     << " / " 
	     << ( qgraph.lendEliminateSupernodeTimer()[0].queryTotalSystemTicks() + 
		  qgraph.lendEliminateSupernodeTimer()[1].queryTotalSystemTicks() + 
		  qgraph.lendEliminateSupernodeTimer()[2].queryTotalSystemTicks() )/ticksPerSec 
	     << endl;
      if ( detailLevel > 3 )
	cout << "            construct new elements                   : "
	     << qgraph.lendEliminateSupernodeTimer()[0].queryTotalUserTicks()/ticksPerSec 
	     << " / " 
	     << qgraph.lendEliminateSupernodeTimer()[0].queryTotalSystemTicks()/ticksPerSec 
	     << endl;
      if ( detailLevel > 3 )
	cout << "            number snode and update eforest          : "
	     << qgraph.lendEliminateSupernodeTimer()[1].queryTotalUserTicks()/ticksPerSec 
	     << " / " 
	     << qgraph.lendEliminateSupernodeTimer()[1].queryTotalSystemTicks()/ticksPerSec 
	     << endl;
      if ( detailLevel > 3 )
	cout << "            add reach set to updateList              : "
	     << qgraph.lendEliminateSupernodeTimer()[2].queryTotalUserTicks()/ticksPerSec 
	     << " / " 
	     << qgraph.lendEliminateSupernodeTimer()[2].queryTotalSystemTicks()/ticksPerSec 
	     << endl;
      if ( detailLevel > 1 )
	cout << "      stage time (1 qgraph update/stage)        : "
	     << ordering.lendStageTimer().queryTotalUserTicks()/ticksPerSec << " / "
	     << ordering.lendStageTimer().queryTotalSystemTicks()/ticksPerSec << endl;
      if ( detailLevel > 2 )
	cout << "         qgraph->update()                          : "
	     << qgraph.lendUpdateTimer().queryTotalUserTicks()/ticksPerSec << " / " 
	     << qgraph.lendUpdateTimer().queryTotalSystemTicks()/ticksPerSec << endl;
      if ( detailLevel > 3 )
	cout << "            single elimination update                : "
	     << ( qgraph.lendSingleUpdateTimer()[0].queryTotalUserTicks() + 
		  qgraph.lendSingleUpdateTimer()[1].queryTotalUserTicks() +  
		  qgraph.lendSingleUpdateTimer()[2].queryTotalUserTicks() +  
		  qgraph.lendSingleUpdateTimer()[3].queryTotalUserTicks() +  
		  qgraph.lendSingleUpdateTimer()[4].queryTotalUserTicks() )/ticksPerSec 
	     << " / " 
	     << ( qgraph.lendSingleUpdateTimer()[0].queryTotalSystemTicks() + 
		  qgraph.lendSingleUpdateTimer()[1].queryTotalSystemTicks() + 
		  qgraph.lendSingleUpdateTimer()[2].queryTotalSystemTicks() + 
		  qgraph.lendSingleUpdateTimer()[3].queryTotalSystemTicks() + 
		  qgraph.lendSingleUpdateTimer()[4].queryTotalSystemTicks() )/ticksPerSec 
	     << endl;
      if ( detailLevel > 4 )
	cout << "              computeSetDiffs()                         : "
	     << qgraph.lendSingleUpdateTimer()[0].queryTotalUserTicks()/ticksPerSec 
	     << " / " 
	   << qgraph.lendSingleUpdateTimer()[0].queryTotalSystemTicks()/ticksPerSec 
	     << endl;
      if ( detailLevel > 4 ) 
	cout << "              stripOldEntriesAndPackIntoBucketSorter()  : "
	     << qgraph.lendSingleUpdateTimer()[1].queryTotalUserTicks()/ticksPerSec 
	     << " / " 
	     << qgraph.lendSingleUpdateTimer()[1].queryTotalSystemTicks()/ticksPerSec 
	     << endl;
      if ( detailLevel > 4 )
	cout << "              scanBucketSorterForSupernodes()           : "
	     << qgraph.lendSingleUpdateTimer()[2].queryTotalUserTicks()/ticksPerSec 
	     << " / " 
	     << qgraph.lendSingleUpdateTimer()[2].queryTotalSystemTicks()/ticksPerSec 
	     << endl;
      if ( detailLevel > 4 )
	cout << "              pruneNonPrincipalSupernodes()             : "
	     << qgraph.lendSingleUpdateTimer()[3].queryTotalUserTicks()/ticksPerSec 
	     << " / " 
	     << qgraph.lendSingleUpdateTimer()[3].queryTotalSystemTicks()/ticksPerSec 
	     << endl;
      if ( detailLevel > 4 )
	cout << "              resetUpdateList()                         : "
	     << qgraph.lendSingleUpdateTimer()[4].queryTotalUserTicks()/ticksPerSec 
	     << " / " 
	     << qgraph.lendSingleUpdateTimer()[4].queryTotalSystemTicks()/ticksPerSec 
	     << endl;
      if ( detailLevel > 3 )
	cout << "            multiple elimination update              : "
	     << ( qgraph.lendMultipleUpdateTimer()[0].queryTotalUserTicks() + 
		  qgraph.lendMultipleUpdateTimer()[1].queryTotalUserTicks() +  
		  qgraph.lendMultipleUpdateTimer()[2].queryTotalUserTicks() +  
		  qgraph.lendMultipleUpdateTimer()[3].queryTotalUserTicks() )/ticksPerSec 
	   << " / " 
	     << ( qgraph.lendMultipleUpdateTimer()[0].queryTotalSystemTicks() + 
		  qgraph.lendMultipleUpdateTimer()[1].queryTotalSystemTicks() + 
		  qgraph.lendMultipleUpdateTimer()[2].queryTotalSystemTicks() + 
		  qgraph.lendMultipleUpdateTimer()[3].queryTotalSystemTicks() )/ticksPerSec 
	     << endl;
      if ( detailLevel > 4 )
	cout << "              xferNewEnodesToEnodeListsOfReachSet()     : "
	     << qgraph.lendMultipleUpdateTimer()[0].queryTotalUserTicks()/ticksPerSec 
	     << " / " 
	     << qgraph.lendMultipleUpdateTimer()[0].queryTotalSystemTicks()/ticksPerSec 
	     << endl;
      if ( detailLevel > 4 )
	cout << "              stripOldEntriesFromAdjListsOfReachSet()   : "
	     << qgraph.lendMultipleUpdateTimer()[1].queryTotalUserTicks()/ticksPerSec 
	     << " / " 
	     << qgraph.lendMultipleUpdateTimer()[1].queryTotalSystemTicks()/ticksPerSec 
	     << endl;
      if ( detailLevel > 4 )
	cout << "              compressAndOutmatchReachSet()             : "
	     << qgraph.lendMultipleUpdateTimer()[2].queryTotalUserTicks()/ticksPerSec 
	     << " / " 
	     << qgraph.lendMultipleUpdateTimer()[2].queryTotalSystemTicks()/ticksPerSec 
	     << endl;
      if ( detailLevel > 4 )
	cout << "              resetReachSet()                           : "
	     << qgraph.lendMultipleUpdateTimer()[3].queryTotalUserTicks()/ticksPerSec 
	     << " / " 
	     << qgraph.lendMultipleUpdateTimer()[3].queryTotalSystemTicks()/ticksPerSec 
	     << endl;
      if ( detailLevel > 5 ) {
	cout << "                   1adj updates                         : "
	     << qgraph.lendResetReachSetTimer()[0].queryTotalUserTicks()/ticksPerSec 
	     << " / " 
	     << qgraph.lendResetReachSetTimer()[0].queryTotalSystemTicks()/ticksPerSec 
	     << endl;
	cout << "                   2adj updates                         : "
	     << qgraph.lendResetReachSetTimer()[1].queryTotalUserTicks()/ticksPerSec 
	     << " / " 
	     << qgraph.lendResetReachSetTimer()[1].queryTotalSystemTicks()/ticksPerSec 
	     << endl;
	cout << "                   other updates                        : "
	     << qgraph.lendResetReachSetTimer()[2].queryTotalUserTicks()/ticksPerSec 
	     << " / " 
	     << qgraph.lendResetReachSetTimer()[2].queryTotalSystemTicks()/ticksPerSec 
	     << endl;
      }
      /*
      if ( detailLevel > 500 )
	cout << "      delayed vertices                          : "
	     << ordering.lendDelayedVtxsTimer().queryTotalUserTicks()/ticksPerSec  << " / " 
	     << ordering.lendDelayedVtxsTimer().queryTotalSystemTicks()/ticksPerSec  << endl;
      */
      if ( detailLevel > 0 )
	cout << "   symbolic factorization                    : " 
	     << symbFactTotalTimer.queryTotalUserTicks()/ticksPerSec << " / " 
	     << symbFactTotalTimer.queryTotalSystemTicks()/ticksPerSec << endl;
      if ( detailLevel > 1 )
	cout << "       set graph                                : "
	     << symbFactSetGraphTimer.queryTotalUserTicks()/ticksPerSec << " / "
	     << symbFactSetGraphTimer.queryTotalSystemTicks()/ticksPerSec << endl;
      if ( detailLevel > 1 )
	cout << "       set permutation map                      : "
	     << symbFactSetPermTimer.queryTotalUserTicks()/ticksPerSec << " / "
	     << symbFactSetPermTimer.queryTotalSystemTicks()/ticksPerSec << endl;
      if ( detailLevel > 1 )
	cout << "       actual factorization                     : "
	     << symbFactExecutionTimer.queryTotalUserTicks()/ticksPerSec << " / "
	     << symbFactExecutionTimer.queryTotalSystemTicks()/ticksPerSec << endl;
      
#endif
    }

    //////////////////////////////////////////////////////////////////////
    //
    // 11. Display quality statistics if requested.
    //
    
    if ( cmdLine.hasOption("s|stats") ) {
      const QuotientGraph& qgraph = ordering.lendQuotientGraph();
      cout << "Run Statistics : " << endl;
      if ( ordering.isCompressed() ) { 
	cout << "    Graph Compression            : true" << endl;
	cout << "    Graph:" << endl;
	cout << "        Number vertices          : " << graph->queryNVtxs() << endl;
	cout << "        Number edges             : " << graph->queryNEdges() << endl;
	cout << "        Number non zeros         : " << graph->queryNNonZeros() << endl;
	cout << "    Compressed Graph:" << endl;
	cout << "        Number vertices          : " << ordering.getCGraph()->queryNVtxs() << endl;
	cout << "        Number edges             : " << ordering.getCGraph()->queryNEdges() << endl;
	cout << "        Number non zeros         : " << ordering.getCGraph()->queryNNonZeros() << endl;
      } else {
	cout << "    Graph Compression            : false" << endl;
	cout << "    Graph:" << endl;
	cout << "        Number vertices          : " << graph->queryNVtxs() << endl;
	cout << "        Number edges             : " << graph->queryNEdges() << endl;
	cout << "        Number non zeros         : " << graph->queryNNonZeros() << endl;
      }
      cout << "    Fill                     : " << symbfact.queryFill() << endl;
      cout << "    Size of factor           : " << symbfact.querySize() << endl;
      cout << "    Work                     : " << symbfact.queryWork() << endl;
      cout << "    # of Stages              : " 
	   << ordering.lendStageTimer().queryNLaps()
	   << endl;
      cout << "    # of Steps               : "
	   << ordering.lendStepTimer().queryNLaps()
	   << endl;
      cout << "    # of vertices compressed : " 
	   << qgraph.queryTotCompressed()
	   << endl;
      cout << "    # of outmatched vertices : "
	   << qgraph.queryTotOutmatched()
	   << endl;      
      //    cout << "    # of heavy vertices removed : "
      //	 << ordering.queryNPurgedVtxs()
      //	 << endl;      
      cout << "    # of 1adj degree updates : " 
	   << qgraph.lendResetReachSetTimer()[0].queryNLaps()
	   << endl;
      cout << "    # of 2adj degree updates : " 
	   << qgraph.lendResetReachSetTimer()[1].queryNLaps()
	   << endl;
      cout << "    # of other degree updates: " 
	   << qgraph.lendResetReachSetTimer()[2].queryNLaps()
	   << endl;
      cout << "    # of defrags             : "
	   << qgraph.queryNDefrags()
	   << endl;      
      cout << "    delta                    : "
	   << ordering.getDelta() 
	   << endl;
    }
    
    //////////////////////////////////////////////////////
    //
    // cleanup 
    //
    if ( cmdLine.hasOption("log") ) {
      float ticksPerSec = stopwatch::queryTicksPerSec();
      stopwatch::ticks ticks = 0;
      const GraphCompressor * compressor = ordering.getGraphCompressor();
#ifdef SPINDLE_DISABLE_STOPWATCH
      ticks += timer.queryTotalUserTicks();
      ticks += timer.queryTotalSystemTicks();
#else
      ticks +=  compressor->getExecutionTimer().queryTotalUserTicks();
      ticks +=  compressor->getExecutionTimer().queryTotalSystemTicks();
      if ( ordering.isCompressed() ) { 
	ticks +=  compressor->getCGraphCreationTimer().queryTotalUserTicks();
	ticks +=  compressor->getCGraphCreationTimer().queryTotalSystemTicks();
      }
      ticks +=  ordering.getTimer().queryTotalUserTicks();
      ticks += ordering.getTimer().queryTotalSystemTicks();
#endif
      cout << "  " << setw(15) << symbfact.querySize() 
	   << "  " << setw(15) << symbfact.queryWork() 
	   << "  " << setw(15) << ticks/ticksPerSec << endl;
    }
    
  } // end for niters

  delete graph;
}




