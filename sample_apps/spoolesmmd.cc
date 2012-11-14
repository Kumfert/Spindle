//
// src/order/drivers/examples/spoolesmmd.cc -- uses spooles' MSMD
//
// $Id: spoolesmmd.cc,v 1.2 2000/02/18 01:31:55 kumfert Exp $
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
//  modified from orderViaMMD.c  in spooles 2.2

static char help_string1[] = "Usage:\n\t ";
static char help_string2[] = " [ -h | --help ] [ --help-fintype ] \n"
"            [ --help-foutttype ] [ --fin <inputfile> ] \n"
"            [ --fout <outputfile> | stdout [ -fouttype  <option> ] [ -perm new2old | old2new ] ] \n"
"            [ --fintype Chaco | MeTiS | HB | Spooles-F | Spooles-B | Dobrian ] \n"
"            [ --msglvl <int> ] [ --msgFile <filename> ] [ --compressFlag <flag> ]\n"
"            [ --prioType <int> ] [ --stepType <int> ] \n"
"            [ -s | --stats ] [ -t | --time ] [ --log ] [ --randseed <int> [ <int> [...] ] ]\n"
"            [ --delta <int> ] [ --etree <etreeFile> | stdout ]\n "
"   -h | --help            : display this screen\n"
"   --help-fintype         : extended info about file types\n"
"   --help-fouttype        : extended information about permutation outputs\n"
"   --fintype  <fileType>  : specify sparse matrix input file format (default Chaco )\n"
"   --delta <int>          : specify min priority tolerance ( default 0 ) \n"
"   --fin <inputfile>      : specify filename on commandline\n"
"  	 		     (else done interactively)\n" 
"   --fout <outputfile>    : display permutation vector to a file\n"
"   --fout stdout          : display permutation vector to standard out\n"
"  			     (else don\'t display permutation)\n"
"   --perm new2old|old2new : force permutation vector to be old-to-new or\n"
"                            new-to-old.  (default is new-to-old).\n"
"   --fouttype <option>    : sets permutation to have range 0..(n-1) or 1..n\n"
"                            and allows length of permutation to be prepended\n"
"                            use \'-help-fouttype\' option for more details\n"
"   --msglvl <int>         : message level\n"
"   --msgFile <filename>   : message file\n"
"   --compressFlag <int>   : compression flag\n"
"                            <int> / 4 >= 1 -->  compress before elimination\n"
"                            <int> %% 4 == 2 --> compress at each elimination step,\n"
"                                                consider all nodes\n"
"                            <int> %% 4 == 1 --> compress at each elimination step,\n"
"                     	 	                 but only consider 2-adj nodes\n"
"                            <int> %% 4 == 0 --> do not perform any compression\n"
"  --prioType <int>        : update type\n"
"                            <int> == 1 --> true updates\n"
"                            <int> == 2 --> approximate updates\n"
"                            <int> == 3 --> half and half\n"
"                            <int> == 4 --> maximal independent set\n"
"  --stepType <int>        : degree extent for independent set elimination\n"
"                            <int> < 1 --> only one node is eliminated at a step,\n"
"                                          e.g. QMD from SPARSPAK and YSMP\n"
"                            <int> == 1 --> regular multiple elimination, e.g., GENMMD\n"
"                            <int> >  1 --> extended multiple elimination\n"
"                            an independent set of nodes is selected for elimination\n"
"                            whose degree satisfies mindeg <= degree <= stepType*mindegree\n"
"   -s | --stats           : show quality statistics\n"
"   -t | --time            : show timing information\n" 
"   --log                  : display (factor-size work time) triple only \n"
"   --randseed <int>       : seed a random number generator to permute the\n"
"                            input matrix.\n"
"                            If <int>==-1, then do not permute (default)\n"
"                           NOTE: \"-randseed <int> <int> <int>\" is valid\n"
"   --etree <etreeFile>    : store the etree (parent pointers) in a file\n"
"   --etree stdout         : print the etree (parent pointers) to stdout\n";

static char help_fouttypes[] =
" PermutationMap Output Formats:\n"
"\t C       : \'C style\' Range from zero to n-1\n"
"\t F77     : \'Fortran style\' Range from 1 to n\n"
"\t len+C   : the length of the permutation, then C-style\n"
"\t len+F77 : the length of the permutation, the Fortran-style\n";



#include <fstream.h>
#include <iomanip.h>

extern "C" { 
#include "MSMD.h"       // from spooles 2.2
#include "DSTree.h"     // from spooles 2.2
#include "timings.h"    // from spooles 2.2
}

#include <iostream.h>
#include "spindle/OptionDatabase.h"
#include "spindle/Graph.h"
#include "spindle/GraphMatrixFileFactory.h"
#include "spindle/HarwellBoeingFile.h"
#include "spindle/SpoolesFile.h"
#include "spindle/DobrianFile.h"
#include "algo.h"

template < class T > struct my_counter : public unary_function< T, void > { 
  my_counter() : count(0), sum(0) {}
  void operator() (T x ) { ++count; sum+=x; }
  int count;
  T sum;
};

int main ( int argc, char *argv[] ) {
  ///////////////////////////////////////////////////////////////////////////
  //
  //  1.  Check for help requests
  //
  SPINDLE_NAMESPACE::OptionDatabase cmdLine;
  cmdLine.loadCommandLineOptions( argc, argv );
  bool terminate = false; 
  if ( cmdLine.hasOption("h|help") ) {
    cerr << help_string1 << argv[0] << help_string2;
    terminate = true;
  }
  if ( cmdLine.hasOption("help-fintype") ) {
    cerr << SPINDLE_NAMESPACE::GraphMatrixFileFactory::getHelpString();
    terminate = true;
  }
  if ( terminate ) { 
    exit( 0 );
  }


  //////////////////////////////////////////////////////////////////////
  //
  // 2. Open File and create Graph
  //
  SPINDLE_NAMESPACE::GraphMatrixFileFactory factory;
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
  SPINDLE_NAMESPACE::Graph* graph=0;
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

  // use a const pointer to the graph from now on
  const SPINDLE_NAMESPACE::Graph* const_graph = graph;

  //////////////////////////////////////////////////////////////////////
  //
  // 4. Get info for MSMD object
  //  
  FILE * msgFile = 0 ;
  int msgLvl = ( cmdLine.hasOption("msgLvl") ) ? 
    atoi( cmdLine.getOption("msgLvl")) : 0;
  int compressFlag =  ( cmdLine.hasOption("compressFlag" ) ) ? 
    atoi( cmdLine.getOption("compressFlag")) : 5 ;
  int prioType =  ( cmdLine.hasOption("prioType" ) ) ? 
    atoi( cmdLine.getOption("prioType")) : 4 ;
  int stepType =  ( cmdLine.hasOption("stepType" ) ) ? 
    atoi( cmdLine.getOption("stepType")) : 1 ;
  const char * msgFileName = "stdout";
  if ( cmdLine.hasOption("msgFileName")) { 
    msgFileName = cmdLine.getOption("msgFileName");
  }
  if ( !strcmp( msgFileName, "stdout" ) ) { 
    msgFile = stdout;
  } else if ( (msgFile = fopen(msgFileName, "a")) == 0 ) {
    cerr << "Error in " << argv[0] << " unble to open file \'" << msgFile << "\'" << endl;
    return(-1) ;
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
    int randseed = ( cmdLine.hasOption("randseed") ) ? 
      atoi( cmdLine.getOption("randseed",iteration)) : 0 ;

    //  char        *inGraphFileName;
    double      cpu, ops, t1, t2 ;
    ETree       *etree ;
    Graph       *g ;
    int        nfind, nvtx, nzf ;
    IV          *newToOldIV = 0, *oldToNewIV = 0;
    MSMD        *msmd = 0;
    MSMDinfo    *msmdinfo = 0;
  
  
    //  outETreeFileName      = argv[8] ;
    //  outOldToNewIVfileName = argv[9] ;
    //utNewToOldIVfileName = argv[10] ;
    fprintf(msgFile, 
	    "\n %s : "
	    "\n msgLvl          -- %d" 
	    "\n msgFile         -- %s" 
	    "\n seed            -- %d" 
	    "\n compressFlag    -- %d" 
	    "\n prioType        -- %d" 
	    "\n stepType        -- %d" 
	    "\n", argv[0], msgLvl, msgFileName, randseed, 
	    compressFlag, prioType, stepType) ;
    fflush(msgFile) ;

  //   initialize the MSMDinfo information object
    msmdinfo                = MSMDinfo_new() ;
    msmdinfo->seed          = randseed ;
    msmdinfo->compressFlag  = compressFlag ;
    msmdinfo->prioType      = prioType ;
    msmdinfo->stepType      = stepType ;
    msmdinfo->msglvl        = msgLvl        ;
    msmdinfo->msgFile       = msgFile       ;


    ////////////////////////////////////////////////////////////////
    //
    //   create a spooles graph object
    //
    MARKTIME(t1) ;
    g = Graph_new() ;
    Graph_setDefaultFields(g) ;
  
    int type =   (const_graph->getVtxWeight().notNull()) + 
      2*(const_graph->getEdgeWeight().notNull()); // vtxweighted + 2* edgeweighted

  //
  // create adjIVL
  //
    IVL * adjIVL = IVL_new();        // create space
    IVL_setDefaultFields( adjIVL );  // initialize empty IVL
    // create sizes array
    const int nVtxs = const_graph->queryNVtxs(); 
    int * sizes = const_graph->getAdjHead().export();
    for ( int i=nVtxs; i>0; --i ) { 
      sizes[i] -= sizes[i-1];
    }
    IVL_init3( adjIVL, IVL_CHUNKED, nVtxs, sizes+1 );
    copy( const_graph->getAdjList().begin(), const_graph->getAdjList().end(), 
	  adjIVL->chunk->base );
    /*
      // Now we need to change the indicies.
      {
      int * idx = adjIVL->chunk->base;
      int stop = const_graph->getAdjList().size();
      for ( int i=0; i<stop; ++i ) { 
      idx[i]++;
      }
      }
    */

    int totVWgt = 0;
    if ( const_graph->getVtxWeight().notNull() ) { 
      my_counter<int> counter = for_each( const_graph->getVtxWeight().begin(), 
					  const_graph->getVtxWeight().end(), 
					  my_counter<int>() );
      totVWgt = counter.sum;
    }
    int totEWgt = 0;
    IVL * ewgtIVL = 0;
    if ( const_graph->getEdgeWeight().notNull() ) { 
      my_counter<int> counter  = for_each( const_graph->getEdgeWeight().begin(), 
					   const_graph->getEdgeWeight().end(), 
					   my_counter<int>() );
      totVWgt = counter.sum;
      ewgtIVL = IVL_new();
      IVL_setDefaultFields( ewgtIVL );  // initialize empty IVL
      IVL_init3( ewgtIVL, IVL_CHUNKED, nVtxs, sizes );
      copy( const_graph->getEdgeWeight().begin(), const_graph->getEdgeWeight().end(), 
	    ewgtIVL->chunk->base );
    }
    Graph_init2( g, type, 
		 const_graph->queryNVtxs(), 
		 const_graph->queryNBoundVtxs(), 
		 const_graph->queryNEdges(), 
		 totVWgt, totEWgt, adjIVL, 
		 const_graph->getVtxWeight().export(),
		 ewgtIVL );

    MARKTIME(t2) ;
    fprintf(msgFile, "\n CPU %9.5f : read in graph from file %s", 
	    t2 - t1, cmdLine.getOption("fin") ) ;
    nvtx = g->nvtx ;
    if ( msgLvl < 4 ) {
      Graph_writeStats(g, msgFile) ;
      fflush(msgFile) ;
    } else {
      Graph_writeForHumanEye(g, msgFile) ;
      fflush(msgFile) ;
    }

    //   create the stages vector
    int * stages = 0 ;

    // order via msmd

    MARKTIME(t1) ;
    msmd = MSMD_new() ;
    MSMD_order(msmd, g, stages, msmdinfo) ;
    MARKTIME(t2) ;
    cpu = t2 - t1 ;
    fprintf(msgFile, "\n CPU %9.5f : order the graph", cpu) ;
    fflush(msgFile) ;
    MSMDinfo_print(msmdinfo, msgFile) ;
    fflush(msgFile) ;

    //   extract the front tree
    MARKTIME(t1) ;
    etree = MSMD_frontETree(msmd) ;
    nfind = ETree_nFactorIndices(etree) ;
    nzf   = ETree_nFactorEntries(etree, SPOOLES_SYMMETRIC) ;
    ops   = ETree_nFactorOps(etree, SPOOLES_REAL, SPOOLES_SYMMETRIC) ;
    MARKTIME(t2) ;
    fprintf(msgFile,  
	    "\n\n CPU %9.5f : make the front tree", t2 - t1) ;
    fprintf(msgFile,  
	    "\n FACTOR : %9d indices, %9d entries, %9.0f operations", 
	    nfind, nzf, ops) ;
    if ( msgLvl < 3 ) {
      ETree_writeStats(etree, msgFile) ;
      fflush(msgFile) ;
    } else {
      ETree_writeForHumanEye(etree, msgFile) ;
      fflush(msgFile) ;
    }
    fprintf(msgFile, "\n STATSMMD %10d %10.0f %8.3f", nzf, ops, cpu) ;
    /*
      //   write the front tree to a file if requested
      if ( strcmp(outETreeFileName, "none") != 0 ) {
      ETree_writeToFile(etree, outETreeFileName) ;
      }

      //   generate the permutation vectors and write to a file if requested
      if ( strcmp(outOldToNewIVfileName, "none") != 0 ) {
      oldToNewIV = IV_new() ;
      } else {
      oldToNewIV = 0 ;
      }
      if ( strcmp(outNewToOldIVfileName, "none") != 0 ) {
      newToOldIV = IV_new() ;
      } else {
      newToOldIV = 0 ;
      }
      if ( oldToNewIV != 0 || newToOldIV != 0 ) {
      MSMD_fillPerms(msmd, newToOldIV, oldToNewIV) ;
      }
      if ( oldToNewIV != 0 ) {
      IV_writeToFile(oldToNewIV, outOldToNewIVfileName) ;
      }
      if ( newToOldIV != 0 ) {
      IV_writeToFile(newToOldIV, outNewToOldIVfileName) ;
      }
    */
    //   free all the working storage
    Graph_free(g) ;
    MSMD_free(msmd) ;
    MSMDinfo_free(msmdinfo) ;
    ETree_free(etree) ;
    if ( newToOldIV != 0 ) {
      IV_free(newToOldIV) ;
    }
    if ( oldToNewIV != 0 ) {
    IV_free(oldToNewIV) ;
  }
  }  
  delete graph;
  fprintf(msgFile, "\n") ;
  fclose(msgFile) ;

  return(1) ; 
}
