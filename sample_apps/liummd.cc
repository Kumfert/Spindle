//
// src/order/drivers/examples/liummd.cc -- uses liu's GENMMD 
//
// $Id: liummd.cc,v 1.2 2000/02/18 01:31:55 kumfert Exp $
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
// DO NOT REDISTRIBUTE:  For comparative purposes only.
//
//
#include <fstream.h>
#include <iomanip.h>

static char help_string1[] = "Usage:\n\t ";
static char help_string2[] = " [ -h | -help ] [ -help-fintype ] \n"
"            [ -help-foutttype ] [ -fin <inputfile> ] \n"
"            [ -fout <outputfile> | stdout [ -fouttype  <option> ] [ -perm new2old | old2new ] ] \n"
"            [ -fintype Chaco | MeTiS | HB | Dobrian | Spooles-B | Spooles-F ] \n"
"            [ -s ] [ -t | -time ] [ -log ] [ -randseed <int> [ <int [..] ] ]\n"
"            [ -delta <int> ] [ -etree <etreeFile> | stdout ]\n "
"   -h | -help            : display this screen\n"
"   -help-fintype         : extended info about file types\n"
"   -help-fouttype        : extended information about permutation outputs\n"
"   -fintype  <fileType>  : specify sparse matrix input file format (default Chaco )\n"
"   -delta <int>          : specify min priority tolerance ( default 0 ) \n"
"   -fin <inputfile>      : specify filename on commandline\n"
"  			    (else done interactively)\n" 
"   -fout <outputfile>    : display permutation vector to a file\n"
"   -fout stdout          : display permutation vector to standard out\n"
"  			    (else don\'t display permutation)\n"
"   -perm new2old|old2new : force permutation vector to be old-to-new or\n"
"                           new-to-old.  (default is new-to-old).\n"
"   -fouttype <option>    : sets permutation to have range 0..(n-1) or 1..n\n"
"                           and allows length of permutation to be prepended\n"
"                           use \'-help-fouttype\' option for more details\n"
"   -s | -stats           : show quality statistics\n"
"   -t | -time            : show timing information\n" 
"   -log                  : display (factor-size work time) triple only \n"
"   -randseed <int> [...] : seed a random number generator to permute the\n"
"                           input matrix.\n"
"                           If <int>==-1, then do not permute (default)\n"
"                           NOTE: \"-niters 3 -randseed <int> <int> <int>\" is valid\n"
"   -etree <etreeFile>    : store the etree (parent pointers) in a file\n"
"   -etree stdout         : print the etree (parent pointers) to stdout\n";

static char help_fouttypes[] =
" PermutationMap Output Formats:\n"
"\t C       : \'C style\' Range from zero to n-1\n"
"\t F77     : \'Fortran style\' Range from 1 to n\n"
"\t len+C   : the length of the permutation, then C-style\n"
"\t len+F77 : the length of the permutation, the Fortran-style\n";

#include <iostream.h>
#include "spindle/OptionDatabase.h"
#include "spindle/Graph.h"
#include "spindle/GraphCompressor.h"
#include "spindle/CompressionMap.h"
#include "spindle/MapUtils.h"
#include "spindle/EliminationForest.h"
#include "spindle/SymbolicFactorization.h"
#include "spindle/ArrayBucketSorter.h"
#include "spindle/MinPriorityEngine.h"
#include "spindle/GraphMatrixFileFactory.h"
#include "genmmd.h"
#include "algo.h"
#include "spindle/GraphUtils.h"

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
  if ( cmdLine.hasOption("help-fouttype") ) { 
    cerr << help_fouttypes;
    terminate = true;
  }
  if ( terminate ) { 
    exit( 0 );
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
  const int nVtxs = graph->queryNVtxs();

  ////////////////////////////////////////////////////////////
  //
  // iterate
  //
  int nIterations = 1;
  if ( cmdLine.hasOption("randseed") ) {
    nIterations = cmdLine.queryNArgs( "randseed" );
  }
  for ( int iteration = 0; iteration < nIterations; ++iteration ) {

    if ( cmdLine.hasOption("randseed") ) {
      int seed = atoi( cmdLine.getOption("randseed",iteration) );
      if ( seed != -1 ) { 
	unsigned int u_seed = seed;
#ifdef __STL_NO_DRAND48
	srand( u_seed );
#else
	srand48( u_seed );
#endif
	SharedArray<int> temp(nVtxs);     // create an array for the permutation
	temp.init(0,1);                   // initialize from 0 to n by 1 increments.
	random_shuffle( temp.begin(), temp.end() ); // create random permutation
	PermutationMap perm(nVtxs);       // create permutation object
	perm.getNew2Old().take( temp );             // adopt the new permutation.
	perm.validate();
	if ( ! perm.isValid() ) { 
	  cerr << "ERROR: cannot create valid random permutation." << endl;
	  exit(-1);
	}
	Graph * perm_graph = GraphUtils::createPermutedGraph( graph, &perm );
	perm_graph->validate();
	if ( ! perm_graph->isValid() ) { 
	  cerr << "ERROR: cannot create valid permuted graph from this input." << endl;
	  exit(-1);
	}
	delete graph;
	graph = perm_graph;
      }
    } // end if -randseed 

    const Graph * c_graph = graph; // now access everything through the const pointer
    
    //  const int nEdges = graph->queryNEdges();
    //  MatrixBase mat;
    //  mat.resize(nVtxs,nVtxs,2*nEdges);
    //  mat.getAdjHead().import( graph->getAdjHead() );
    //  mat.getAdjList().import( graph->getAdjList() );
    //  MatrixUtils::makeDiagonalsExplicit( &mat );
    
    ////////////////////////////////////////////
    //
    // 4. allocate the storage
    //
    
    int * Aptr = c_graph->getAdjHead().export();
    int * Aind = c_graph->getAdjList().export();
    int * forperm = new int[ nVtxs ];
    int * invp = new int[ nVtxs ];
    int * head = new int[ nVtxs ];
    int * qsize = new int[ nVtxs ];
    int * listp = new int[ nVtxs ];
    int * marker = new int[ nVtxs ];
    int delta = 0;
    int ncsub;
    int maxint = 100000;
    
    if ( cmdLine.hasOption("delta") ) {
      delta = atoi( cmdLine.getOption("delta") );
    }
    
    //////////////////////////////////////////////////////////////////////
    //
    // 5. Compute the ordering
    //
    stopwatch timer;
    timer.start();
    genmmd(nVtxs,Aptr,Aind,invp,forperm,delta,head,qsize,listp,marker,maxint,&ncsub);
    timer.stop();
    
    
    ////////////////////////////////////////////////
    //
    // 6
    //
    // now decrement the permutation
    for ( int k = 0; k < nVtxs; ++k ) { 
      --invp[k];
    }
    PermutationMap perm(nVtxs);
    perm.getOld2New().import( invp, nVtxs );
    perm.validate();
    if (! perm.isValid() ) { 
      cerr << "Error creating valid permutation" << endl;
      exit(-1);
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
      if ( ! symbfact.setGraph( c_graph ) ) {
	cerr << "Error setting Graph" << endl;
	cerr << symbfact.queryErrMsg() << endl;
	exit(-1);
      }
      symbFactSetGraphTimer.stop();
    
      // set the uncompressed permutation
      symbFactSetPermTimer.start();
      if ( ! symbfact.setPermutationMap( &perm ) ) {
	cerr << "Error seting permutation in symbolic factorization" << endl;
	cerr << symbfact.queryErrMsg() << endl;
	exit(-1);
      }
      symbFactSetPermTimer.stop();
    
      symbFactExecutionTimer.start();
      if ( ! symbfact.execute() ) {
	cerr << "Error doing symbolic factorization on unordered graph." << endl;
	cerr << symbfact.queryErrMsg() << endl;
	exit(-1);
      }
      symbFactExecutionTimer.stop();
      symbFactTotalTimer.stop();
    }
    
  
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
      const int nvtxs = c_graph->size();
      
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
    
      const int *p;
      if ( cmdLine.hasOption("perm","old2new") ) {
	p = new_perm->getOld2New().lend();
      } else {
	p = new_perm->getNew2Old().lend();
      }

      int offset = 0;
      bool printLength = false;
      if ( cmdLine.hasOption("fouttype","F77") ) {
	offset = 1;
      } else if ( cmdLine.hasOption("fouttype","len+C") ) {
	printLength = true;
      } else if ( cmdLine.hasOption("fouttype","len+F77") ) {
	offset = 1;
	printLength = true;
      }

      if ( printLength == true ) {
	fprintf( output_file,"%d\n", new_perm->size() );
      }
      
      for(const int* cur = p, *stop=p+new_perm->size(); cur <stop; ++cur ) {
	fprintf( output_file, "%d\n", *cur + offset);
      }
      if (output_file != stdout) {
	fclose(output_file);
      }
    }
  
    //////////////////////////////////////////////////////////////////////
    //
    // 11. Display quality statistics if requested.
    //

    if ( cmdLine.hasOption("s|stats") ) {
      cout << "Run Statistics : " << endl;
      cout << "    Graph Compression            : false" << endl;
      cout << "    Graph:" << endl;
      cout << "        Number vertices          : " << c_graph->queryNVtxs() << endl;
      cout << "        Number edges             : " << c_graph->queryNEdges() << endl;
      cout << "        Number non zeros         : " << c_graph->queryNNonZeros() << endl;
      cout << "    Fill                     : " << symbfact.queryFill() << endl;
      cout << "    Size of factor           : " << symbfact.querySize() << endl;
      cout << "    Work                     : " << symbfact.queryWork() << endl;
      cout << "    delta                    : "
	   << delta
	   << endl;
    }
  
    //////////////////////////////////////////////////////
    //
    // cleanup 
    //
    if ( cmdLine.hasOption("log") ) {
      float ticksPerSec = stopwatch::queryTicksPerSec();
      stopwatch::ticks ticks = 0;
      ticks += timer.queryTotalUserTicks();
      ticks += timer.queryTotalSystemTicks();
      cout << "  " << setw(15) << symbfact.querySize() 
	   << "  " << setw(15) << symbfact.queryWork() 
	   << "  " << setw(15) << ticks/ticksPerSec << endl;
    }
    
    SharedArray<int> my_array;
    // to prevent freeing memory mismatch.  Its'
    // better to give the memory back to a SharedArray
    // even though delete[] will work most of the time.
    {
      int nNonZeros = Aptr[nVtxs];
      my_array.take( Aptr, nVtxs+1);
      my_array.take( Aind, nNonZeros );
    }  
    delete[] forperm;
    delete[] invp;
    delete[] head;
    delete[] qsize;
    delete[] listp;
    delete[] marker;
  }
  delete graph;
}



