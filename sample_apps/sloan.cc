//
// src/order/drivers/examples/ex01.cc  -- sloan ordering
//
// $Id: sloan.cc,v 1.1 2000/03/13 22:16:08 kumfert Exp $
//
//  Gary Kumfert, Old Dominion University
//  Copyright(c) 1997-1999, Old Dominion University.  All rights reserved.
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

static char help_string1[] = "Usage:\n\t ";
static char help_string2[] = 
" [ -h | --help ] [ --help-fintype ] [ --help-diam ] [--help-class]\n"
"\t[ --fin <inputfile> ] [-fintype Chaco | MeTiS | HB | Dobrian | Spooles-F | Spooles-B ]\n"
"\t[ --fout <outputfile> ] [ -outtype old2new | new2old ]\n"
"\t[ --fsnodes <file> | <int_str> ] [ -fenodes <file> | <int_str> ]\n"
"\t[ --forceENodesLast ] [ -class 1 | 2 | ? ] \n"
"\t[ -s | --stats ] [ -t | --time ] [--cRatio <float>]\n"
"\t[ --diam Sloan | DRS | RS [<int>] ]\n\n"
"   -h | --help         : display this screen\n"
"   --help-fintype      : extended info about file types\n"
"   --help-diam         : extended info about diam policy\n"
"   --help-class        : extended info about classes of graphs\n"
"   --help-permformat    : extended information about permutation outputs\n"
"   --fin <inputfile>   : specify filename on commandline\n"
"   --fintype <ftype>   : input file type,\n"
"  			  can be \"HB\", \"Dobrian\", \"MeTiS\",or \"Chaco\"(default)\n"
"   --fout <outputfile> : display permutation vector to a file\n"
"   --fout stdout       : display permutation vector to standard out\n"
"  			 (else don\'t display permutation)\n"
"   --permtype <ptype>  : type of permutation\n"
"  			 can be \"old2new\" or \"new2old\" (default)\n"
"   --permformat <pform>: format of permutation vector\n"
"  			 use \"--help-permformat\" for more information\n"
"   --cRatio <float>    : use compressed graph if compression ratio\n"
"  			 (of vertices) is less than <float>.\n"
"                      : 0.0 implies no compression. default==0.9\n"
"   --fsnodes <file>    : manually set start nodes from an input file,\n"
"   --fsnodes <int_str> : manualy set start nodes from a string of ints\n"
"                        (if first char is a digit, it is considered int_str)\n"
"                        disabled by -diam, vertices numbered 0..(n-1)\n"
"   --fenodes <file>    : manually set end nodes from an input file,\n"
"   --fenodes <int_str> : manualy set end nodes from a string of ints\n"
"                        (if first char is a digit, it is considered int_str)\n"
"                        disabled by -diam, vertices numbered 0..(n-1)\n"
"   --forceENodesLast   : try to force enodes to be numbered last\n"
"   --diam <policy>     : specify pseudo-diameter shrinking strategy\n"
"  			 can be \"Sloan\", \"DRS\", or \"RS\" (default)\n"
"  			 \"RS\" has an integer argument with is 5 by default.\n"
"   --class 1 | 2 | ?   : use \"--help-class\" for more information\n"
"   -s | --stats        : show quality statistics\n"
"   -t | --time         : show timing information\n";

static char help_diam[] =
" PseudoDiameter Shrinking Policies\n"
"\t Sloan      : Shrinking strategy in original Sloan paper.\n"
"\t              Sort all the farthest vertices by degree and take\n"
"\t              the last (n+1)/2 of them.\n"
"\t DRS        : Duff Reid and Scott strategy for MC40.\n"
"\t              Choose a vertex of each degree.  Much faster.\n"
"\t RS [<int>] : Reid and Scott strategy for MC60.\n"
"\t              About same speed as DRS, but much narrower structures.\n"
"\t              It uses <int> non-adjacent vertices of smallest degree.\n"
"\t              This number is 5 by default.\n";

static char help_class[] =
" Sloan Ordering Class Distinctions\n"
"\t The Sloan ordering algorithm tends to distinguish between two\n"
"\t classes of graphs:  Those for which a stronger global weight \n"
"\t provides a better ordering, and those for which a stronger local\n"
"\t weight provides a better ordering.  In general, there is no test\n"
"\t to determine the class a priori, but it does seem to be based on \n"
"\t topology.  Problems arizing from similar problems with similar geometries\n"
"\t rarely change class.\n\n"
"\t This implementation can compute both class 1 and class 2 ordering and\n"
"\t choose the better, or it can save some work by allowing the user to\n"
"\t specify exactly which ordering should be used.\n\n"
"\t class 1            : Strong local weights provide better orderings\n"
"\t class 2            : Strong global weights provide better orderings\n"
"\t class ?            : Don't know, run both and pick the best\n";

static char help_permformat[] =
" PermutationMap Output Formats:\n"
"\t C       : \'C style\' Range from zero to n-1\n"
"\t F77     : \'Fortran style\' Range from 1 to n\n"
"\t len+C   : the length of the permutation, then C-style\n"
"\t len+F77 : the length of the permutation, the Fortran-style\n";

#include <iostream.h>
#include <fstream.h>
#include <string.h>
#include <vector.h>
#include <algo.h>
#include "spindle/OptionDatabase.h"
#include "spindle/PermutationMap.h"
#include "spindle/GraphMatrixFileFactory.h"
#include "spindle/Graph.h"
#include "spindle/GraphCompressor.h"
#include "spindle/SloanEngine.h"

bool
readArray( vector<int>& data, const char * filename ) {
  if ( (filename[0] >= '0') && (filename[0] <='9') ) { 
    // parse as a string of ints
    char * numbers = (char * ) filename;
    char * cur = strtok( numbers, " \t\n");
    if ( cur != NULL ) {
      data.push_back( atoi( cur ) );
    }
    while ( ( cur = strtok( NULL, " \t\n") ) != NULL ) {
      data.push_back( atoi( cur ) );
    }
  } else { 
    // parse as a filename
    ifstream fin(filename);
    if ( !fin ) { return false; }
    istream_iterator<int,ptrdiff_t> in(fin);
    istream_iterator<int,ptrdiff_t> eof;
    while( in != eof ) {
      data.push_back( *in );
      ++in;
    }
  }
  ostream_iterator<int> out( cout,"\n" );
  copy( data.begin(), data.end(), out );
  return true;
}


#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

int main(int argc, char *argv[]) {

  ///////////////////////////////////////////////////////////////////////////
  //
  //  1.  Check for help requests
  //
  OptionDatabase cmdLine;
  cmdLine.loadCommandLineOptions( argc, argv );
  bool terminate = false; 
  if ( cmdLine.hasOption("h|help") ) {
    cerr << help_string1 << argv[0] << help_string2;
    terminate = true;
  }
  if ( cmdLine.hasOption("help-fintype") ) {
    cerr << GraphMatrixFileFactory::getHelpString() << endl;
    terminate = true;
  }
  if ( cmdLine.hasOption("help-diam") ) { 
    cerr << help_diam;
    terminate = true;
  }
  if ( cmdLine.hasOption("help-class") ) { 
    cerr << help_class;
    terminate = true;
  }
  if ( cmdLine.hasOption("help-permformat") ) { 
    cerr << help_permformat;
    terminate = true;
  }
  if ( terminate ) { 
    exit( 0 );
  }


  ////////////////////////////////////////////////////////////////////////////
  //
  // 2.  Load Graph from File
  //
  GraphMatrixFileFactory factory;
  string filename;
  string filetype;
  if ( cmdLine.hasOption("fin|-fin") ) { 
    filename = cmdLine.getOption("fin|-fin");
  } else { 
    cerr << "Error: must specify input file using \"--fin\". "<< endl;
    cerr << "       Try \"--help\" for more information." << endl;
    exit(-1);
  }
  if ( cmdLine.hasOption("fintype|-fintype") ) { 
    filetype = cmdLine.getOption("fintype|-fintype");
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
  
  ////////////////////////////////////////////////////////////////////////
  //
  // 4. Create a sloan ordering
  //
  SloanEngine ordering(graph);
  // (set any options here)
  if ( cmdLine.hasOption("cRatio") ) {
    float targetRatio;
    if (!cmdLine.queryOption("cRatio", &targetRatio) ||
	!ordering.setCompressionRatio( targetRatio ) ) { 
      cerr << "Attempt to adjust Compression Ratio to " << targetRatio
	   << " failed." << endl;
    }
  }
  if ( cmdLine.hasOption("diam") ) { 
    const char* option = cmdLine.getOption("diam");
    PseudoDiamShrinkingStrategy* shrink=0;
    if ( !strcasecmp(option,"sloan") ) {
      shrink = new SloanShrinkingStrategy();
    } else if ( !strcasecmp(option,"DRS") ) {
      shrink = new DuffReidScottShrinkingStrategy();
    } else if ( !strcasecmp(option,"RS") ) {
      ReidScottShrinkingStrategy *reidScott = new ReidScottShrinkingStrategy();
      const char * int_arg = cmdLine.getOption("RS");
      if ( (int_arg[0] >= '0') && (int_arg[0] <= '9') ) {
	reidScott->setNVtxsMax( atoi( int_arg ) );
      }
      shrink = reidScott;
    }
    if ( !ordering.setPseudoDiameterShrinkingStrategy( shrink )) { 
      cerr << "Warning: problem setting shrinking strategy" << endl;
    }
  } else { 
    if ( cmdLine.hasOption("fsnodes") ) { 
      const char* filename = cmdLine.getOption("fsnodes");
      vector<int> startVtxs;

      readArray( startVtxs, filename );
      if ( startVtxs.size() > 0 ) {
	ordering.setStartVtxs( startVtxs.begin(), startVtxs.size() );
      }
    }
    if ( cmdLine.hasOption("fenodes") ) { 
      const char* filename = cmdLine.getOption("fenodes");
      vector<int> endVtxs;

      readArray( endVtxs, filename );
      if ( endVtxs.size() > 0 ) {
	ordering.setEndVtxs( endVtxs.begin(), endVtxs.size() );
      }
    }
  }
  if ( cmdLine.hasOption("forceENodesLast") ) {
    ordering.enableForceEndVtxsLast();
  }
  if ( cmdLine.hasOption("class") ) {
    if ( cmdLine.hasOption("class","1") ) {
      ordering.setClass( 1 );
    } else if ( cmdLine.hasOption("class","2") ) { 
      ordering.setClass( 2 );
    }
  }

  ////////////////////////////////////////////////////////////////////////
  //
  // 5. Execute the sloan ordering
  //
  ordering.execute();
 
	 
  //////////////////////////////////////////////////////////////////////
  //
  // 6. Display the permutation vector, if requested
  //
  if ( cmdLine.hasOption("fout") ) {
    FILE* output_file = NULL;
    const PermutationMap * perm = ordering.getPermutation();
    if (! perm->isValid() ) { 
      cerr << "Error in creating valid permutation" << endl;
      exit(-1);
    }
    if ( cmdLine.hasOption("fout","stdout") ) {
      output_file = stdout; 
    } else {
      const char * output_file_name = cmdLine.getOption("fout");
      if (( output_file = fopen( output_file_name,"w") ) == NULL ) {
	cerr << "Error: could not open outputfile\'" << output_file_name 
	     << "\'" << endl;
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
    
    perm->prettyPrint( output_file, printNew2Old, printLength, onesOffset );

    if (output_file != stdout) {
      fclose(output_file);
    }
    delete perm;
  }

  //////////////////////////////////////////////////////////////////////
  //
  // 7. Display timing information, if requested.
  //
  if ( cmdLine.hasOption("t|-time") ) {
    float ticksPerSec = stopwatch::queryTicksPerSec();
    const GraphCompressor * compressor = ordering.getGraphCompressor();
    cout << "Timing Information (seconds) user / system :" << endl;
    if ( ordering.isCompressed() ) {
      cout << "\t Total Compression Time                  : " 
	   << ( compressor->getExecutionTimer().queryTotalUserTicks()
		+ compressor->getCGraphCreationTimer().queryTotalUserTicks() )/ticksPerSec << " / " 
	   << ( compressor->getExecutionTimer().queryTotalSystemTicks()
		+  compressor->getCGraphCreationTimer().queryTotalSystemTicks() )/ticksPerSec << endl;
      cout << "\t   Compression Map Generation            : " 
	   << compressor->getExecutionTimer().queryTotalUserTicks()/ticksPerSec << " / " 
	   << compressor->getExecutionTimer().queryTotalSystemTicks()/ticksPerSec << endl;
      cout << "\t   Compression Graph Creation/Validation : " 
	   << compressor->getCGraphCreationTimer().queryTotalUserTicks()/ticksPerSec << " / " 
	   << compressor->getCGraphCreationTimer().queryTotalSystemTicks()/ticksPerSec << endl;
    } else { 
      cout << "\t Total Compression Time (not used)       : " 
	   << compressor->getExecutionTimer().queryTotalUserTicks()/ticksPerSec << " / " 
	   << compressor->getExecutionTimer().queryTotalSystemTicks()/ticksPerSec << endl;
    }
    cout << "\t Total Sloan Time                        : " 
	 << ordering.getTotalExecutionTimer().queryTotalUserTicks()/ticksPerSec << " / " 
	 << ordering.getTotalExecutionTimer().queryTotalSystemTicks()/ticksPerSec << endl;

    const SloanEngine::BFS* bfs = ordering.getBFS();
    cout << "\t   Total BFS Time                        : " 
	 << bfs->getTimer().queryTotalUserTicks()/ticksPerSec << " / " 
	 << bfs->getTimer().queryTotalSystemTicks()/ticksPerSec << endl;

    cout << "\t   Total PseudoDiam Time                 : " 
	 << ordering.getPseudoDiameter()->getTimer().queryTotalUserTicks()/ticksPerSec << " / " 
	 << ordering.getPseudoDiameter()->getTimer().queryTotalSystemTicks()/ticksPerSec << endl;

    cout << "\t   Sloan Ordering Component Time         : " 
	 << ordering.getOrderComponentTimer().queryTotalUserTicks()/ticksPerSec << " / " 
	 << ordering.getOrderComponentTimer().queryTotalSystemTicks()/ticksPerSec << endl;

    cout << "\t       Sloan Ordering Time (good run)    : " 
	 << ordering.getNumberVerticesKeptTimer().queryTotalUserTicks()/ticksPerSec << " / " 
	 << ordering.getNumberVerticesKeptTimer().queryTotalSystemTicks()/ticksPerSec << endl;

    cout << "\t       Sloan Ordering Time (bad run)     : "
	 << ordering.getNumberVerticesRejectedTimer().queryTotalUserTicks()/ticksPerSec
	 << " / " 
	 << ordering.getNumberVerticesRejectedTimer().queryTotalSystemTicks()/ticksPerSec 
	 << endl;

  }

  //////////////////////////////////////////////////////////////////////
  //
  // 8. Display quality statistics if requested.
  //
  if ( cmdLine.hasOption("s|-stats") ) { 
    float max_wf = 0.0; 
    float sum_wf = 0.0;
    float sum_wf_sq = 0.0;
    SharedArray<int>& wavefront = ordering.getWavefront();
    if ( ordering.isCompressed() ) { 
      const int n = ordering.getCGraph()->queryNVtxs();
      const int * new2old = ordering.getPermutation()->getNew2Old().lend();
      const int * vwgt = ordering.getCGraph()->getVtxWeight().lend();
      for( int i=0; i<n; i++ ) { 
	float wf = wavefront[i];
	int wgt = vwgt[ new2old[ i ] ];
	sum_wf += wf * wgt;
	sum_wf_sq += wf * wf * wgt * wgt; 
	max_wf = ( max_wf < wf ) ? wf : max_wf;
      }
    } else {
      for( int i=0; i<nVtxs; i++ ) { 
	float wf = wavefront[i];
	sum_wf += wf;
	sum_wf_sq += wf * wf; 
	max_wf = ( max_wf < wf ) ? wf : max_wf;
      }
    }
    cout << "Statistics: " << endl;
    if ( ordering.isCompressed() ) {
      cout << "\t Graph Compression             : true" << endl;
      cout << "\t Graph:" << endl;
      cout << "\t\t Number vertices       : " << graph->queryNVtxs() << endl;
      cout << "\t\t Number edges          : " << graph->queryNEdges() << endl;
      cout << "\t\t Number non zeros      : " << graph->queryNNonZeros() << endl;
      cout << "\t Compressed Graph:" << endl;
      cout << "\t\t Number vertices       : " << ordering.getCGraph()->queryNVtxs() << endl;
      cout << "\t\t Number edges          : " << ordering.getCGraph()->queryNEdges() << endl;
      cout << "\t\t Number non zeros      : " << ordering.getCGraph()->queryNNonZeros() << endl;
    } else {
      cout << "\t Graph Compression             : false" << endl;
      cout << "\t Graph:" << endl;
      cout << "\t\t Number vertices       : " << graph->queryNVtxs() << endl;
      cout << "\t\t Number edges          : " << graph->queryNEdges() << endl;
      cout << "\t\t Number non zeros      : " << graph->queryNNonZeros() << endl;
    }
    cout << "\t Sloan:" << endl;
    cout << "\t\t Class                 : " << ordering.getClass() << endl;
    cout << "\t\t Envelope              : " << ( sum_wf - nVtxs ) << endl;
    cout << "\t\t Max Wavefront         : " << max_wf << endl;
    cout << "\t\t Mean Square Wavefront : " << ( sum_wf_sq / nVtxs ) << endl;
  }
  
  if (graph!= 0) { 
    delete graph;
    graph = 0;
  }
}

