//
// src/order/drivers/examples/ex03.cc  -- constrained wavefront ordering
//
// $$
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
//  This driver takes a matrix K  = [ K11, K12; K12' K22 ]
//  and produces a wavefront reducing ordering subject to the 
//  constraint that all the columns of K11 must be numbered _before_ K22
//

static char help_string1[] = "Usage:\n\t ";
static char help_string2[] = 
" [ -h | -help ] [ -help-fintype ] [ -help-permtype ] [ -help-class ]\n"
"\t[ -fin <inputfile> ] [-fintype Chaco | MeTiS | HB | Dobrian | Christensen ]\n"
"\t[ -fout <outputfile> ] [ -permtype old2new | new2old ] [ -permformat <option> ]\n"
"\t[ -delay-cols <file> | <int_str> ]\n"
"   -h | -help         : display this screen\n"
"   -help-fintype      : extended info about file types\n"
"   -help-permformat   : information about different outputs\n"
"   -fin <inputfile>   : specify filename on commandline\n"
"   -fintype <ftype>   : input file type,\n"
"  			  can be \"HB\", \"Dobrian\", \"MeTiS\",or \"Chaco\"(default)\n"
"   -fout <outputfile> : display permutation vector to a file\n"
"   -fout stdout       : display permutation vector to standard out\n"
"  			 (else don\'t display permutation)\n"
"   -permtype <p>      : can be \"old2new\" or \"new2old\" (default)\n"
"   -permformat <opt>  : sets permutation to have range 0..(n-1) or 1..n\n"
"                        and allows length of permutation to be prepended\n"
"                        use \'-help-permtype\' option for more details\n"
"   -class0 1 | 2 | ?  : set class of subgraph of undelayed vtxs\n"
"   -class0 1 | 2 | ?    set class of subgraph of delayed vtxs\n"
"                        (use \"-help-class\" for more information)\n"
"   -delay-cols <file> : manually set columns in K22 block,\n"
"   -delay-cols <int_str> : manualy set columns in K22 block, \n"
"                        (if first char is a digit, it is considered int_str)\n"
"                        vertices numbered 0..(n-1)\n";

static char help_fintype[] = 
" Input File types:\n"
"\t Chaco   : Chaco Software package by Bruce Hendrickson and Robert Leland\n"
"\t           Sandia National Labs, New Mexico.\n"
"\t MeTiS   : MeTiS Partitioning Software by George Karypis and Vipin Kumar\n"
"\t           University of Minnesota.\n"
"\t           Use Chaco to handle MeTiS files. Chaco is more general.\n"
"\t HB      : Harwell-Boeing Format.\n"
"\t Dobrian : Florin Dobrian's indefinite solver, Old Dominion University,\n"
"\t           Norfolk, Va\n"
"\t Christensen : Peter Christensen's matrix, has list of columns that must\n"
"\t             : be delayed\n";

static char help_permformat[] =
" PermutationMap Output Formats:\n"
"\t C       : \'C style\' Range from zero to n-1\n"
"\t F77     : \'Fortran style\' Range from 1 to n\n"
"\t len+C   : the length of the permutation, then C-style\n"
"\t len+F77 : the length of the permutation, the Fortran-style\n";

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

#include <iostream.h>
#include <fstream.h>
#include <string.h>
#include <vector.h>
#include <algo.h>
#include "spindle/CommandLineOptions.h"
#include "spindle/PermutationMap.h"
#include "spindle/CompressionMap.h"
#include "spindle/MapUtils.h"
#include "spindle/ChacoFile.h"
#include "spindle/HarwellBoeingFile.h"
#include "spindle/DobrianFile.h"
#include "spindle/ChristensenFile.h"
#include "spindle/SpoolesFile.h"
#include "spindle/Graph.h"
#include "spindle/GraphCompressor.h"
#include "spindle/SloanEngine.h"
#include "spindle/ChristensenFile.h"
#include "spindle/ScatterMap.h"
#include "spindle/GraphUtils.h"

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
  CommandLineOptions cmdLine( argc, argv );
  bool terminate = false; 
  if ( cmdLine.hasFlag("-h|-help") ) {
    cerr << help_string1 << argv[0] << help_string2;
    terminate = true;
  }
  if ( cmdLine.hasFlag("-help-fintype") ) {
    cerr << help_fintype;
    terminate = true;
  }
  if ( cmdLine.hasFlag("-help-permformat") ) { 
    cerr << help_permformat;
    terminate = true;
  }
  if ( cmdLine.hasFlag("-help-class") ) { 
    cerr << help_class;
    terminate = true;
  }
  if ( terminate ) { 
    exit( 0 );
  }


  //////////////////////////////////////////////////////////////////////
  //
  // 2. Open File
  //

  ////////////////////////////////////////////////////////////////////////////
  //
  // 3.  Create graph based on file type
  //
  const char* input_file_name =0;
  if ( cmdLine.hasFlag("-fin") ) {
    input_file_name = cmdLine.getOption("-fin");
  } else {                       // else prompt user for input file
    cerr << "Error: must specify input file using \"-fin\". "<< endl;
    cerr << "       Try \"-help\" for more information." << endl;
  }
  Graph* graph=0;
  vector<int> delayedCols;
  if ( cmdLine.hasFlag("-fintype") ) {
    const char* filetype = cmdLine.getOption("-fintype");
    if ( !strcasecmp(filetype,"Christensen") ) {
      ChristensenFile input;
      if (!input.open( input_file_name,"r") ) {
	cerr << "Error:  could not read \'" << input_file_name << "\'" << endl;
	exit(-1);
      }
      input.read();
      delayedCols.reserve( input.queryNColsInBlock22() );
      delayedCols.insert( delayedCols.begin(), input.block22_begin(), input.block22_end() );
      graph = new Graph( &input );
    } else if ( !strcasecmp(filetype,"Chaco") ) {
      ChacoFile input;
      if (!input.open( input_file_name,"r") ) {
	cerr << "Error:  could not read \'" << input_file_name << "\'" << endl;
	exit(-1);
      }
      input.read();
      graph = new Graph( &input );
    } else if ( !strcasecmp(filetype,"MeTiS") ) {
      ChacoFile input;
      if (!input.open( input_file_name,"r") ) {
	cerr << "Error:  could not read \'" << input_file_name << "\'" << endl;
	exit(-1);
      }
     input.read();
      graph = new Graph( &input );
    } else if ( !strcasecmp(filetype,"HB") ) {
      HarwellBoeingFile input;
      if (!input.open( input_file_name,"r") ) {
	cerr << "Error:  could not read \'" << input_file_name << "\'" << endl;
	exit(-1);
      }
      input.read();
      graph = new Graph( &input );
    } else if ( !strcasecmp(filetype,"Dobrian") ) {
      DobrianFile input;
      if (!input.open( input_file_name,"r") ) {
	cerr << "Error:  could not read \'" << input_file_name << "\'" << endl;
	exit(-1);
      }
      input.read();
      graph = new Graph( &input );
    } else if ( !strcasecmp(filetype,"Spooles-F") ) {
      SpoolesFile input;
      if (!input.open( input_file_name,"r") ) {
	cerr << "Error:  could not read \'" << input_file_name << "\'" << endl;
	exit(-1);
      }
      input.read();
      graph = new Graph( &input );
    } else if ( !strcasecmp(filetype,"Spooles-B") ) {
      SpoolesFile input;
      if (!input.open( input_file_name,"rb") ) {
	cerr << "Error:  could not read \'" << input_file_name << "\'" << endl;
	exit(-1);
      }
      input.read();
      graph = new Graph( &input );
    } else { 
      cerr << "Error: do not understand \'" << filetype << "\' files." << endl;
      exit(-1);
    }
  } else { 
    // assume chaco file
    ChacoFile input;
    if (!input.open( input_file_name,"r") ) {
      cerr << "Error:  could not read \'" << input_file_name << "\'" << endl;
      exit(-1);
    }    
    input.read();
    graph = new Graph( &input );
  }

  if ( delayedCols.size() == 0 ) {
    if ( cmdLine.hasFlag("-delay-cols") ) {
      const char* filename = cmdLine.getOption("-delay-cols");
      readArray( delayedCols, filename );
    }	
    for ( vector<int>::iterator it = delayedCols.begin(); it != delayedCols.end(); ++it ) { 
      --(*it);
    }
  }
	 
  //////////////////////////////////////////////////////////////////////
  //
  // 4. Validate the graph
  //
  graph->validate();
  if ( ! graph->isValid() ) {
    cerr << "ERROR: cannot create valid graph from this input." << endl;
    exit(-1);
  }
  const int nVtxs = graph->queryNVtxs();

 
  //////////////////////////////////////////////////////////////////////
  //
  // 5. Now create two subgraphs. 
  //
  // K11 - which is the first block and all the adjacent nodes in K22.
  // K22 - strictly the second block
  
  // first, we need to create the masks.
  vector<int> mask0( nVtxs, 1 ); // mask0... true for non-delayed cols.
  vector<int> mask1( nVtxs, 0 ); // mask1... true for delayed cols
  for ( vector<int>::const_iterator it = delayedCols.begin(), 
	  stop = delayedCols.end();
	it != stop; ++it ) { 
    mask0[ *it ] = 0;
    mask1[ *it ] = 1;
  }
  // Now, we must find delayed columns that are adjacent to
  // non delayed ones.
  vector<int> boundaryCols;
  boundaryCols.reserve( delayedCols.size()); // preallocate max space
  // for all the delayed columns
  for( vector<int>::const_iterator it = delayedCols.begin(), 
	  stop = delayedCols.end();
	it != stop; ++it ) { 
    int i = *it;
    // for all adj vertices.
    for ( Graph::const_iterator adj_i = graph->begin_adj(i), 
	    stop_adj = graph->end_adj(i); adj_i != stop_adj; ++adj_i ) { 
      if ( !mask1[ *adj_i ] ) {  // if adj vertex is not delayed.
	// then adj_i must be a boundary vtx
	mask0[ i ] = 1;
	boundaryCols.push_back( i );
	break;
      }
    }
  }
  sort( boundaryCols.begin(), boundaryCols.end() );
  vector<int>::iterator new_end = unique( boundaryCols.begin(), boundaryCols.end() );
  boundaryCols.erase( new_end, boundaryCols.end() );

  //
  // Now we can create the subgraphs.
  //
  Graph * K[2];
  ScatterMap * scatterMap[2];
  
  if ( !GraphUtils::createSubgraph( graph, mask0.begin(), &(K[0]), &(scatterMap[0]), 
				    false )) {
    cerr << "ERROR: Failure to create first subgraph" << endl;
  } else if ( !  GraphUtils::createSubgraph( graph, mask1.begin(), &(K[1]), 
					     &(scatterMap[1]), false ) ) { 
    cerr << "ERROR: Failure to create second subgraph" << endl;
  }
  
  ////////////////////////////////////////////////////////////////////////
  //
  // 6. Create sloan orderings
  //
  SloanEngine ordering0( K[0] );
  SloanEngine ordering1( K[1] ); 

  if ( cmdLine.hasFlag("-class0") ) {
    if ( cmdLine.hasOption("-class0","1") ) {
      ordering0.setClass( 1 );
    } else if ( cmdLine.hasOption("-class0","2") ) { 
      ordering0.setClass( 2 );
    }
  }  
  if ( cmdLine.hasFlag("-class1") ) {
    if ( cmdLine.hasOption("-class1","1") ) {
      ordering1.setClass( 1 );
    } else if ( cmdLine.hasOption("-class1","2") ) { 
      ordering1.setClass( 2 );
    }
  }


  const ScatterMap ** c_scatter = const_cast<const ScatterMap**>(scatterMap);

  // Now we need to set the start and end vtxs, but the boundary
  // columns are in terms of the global indices.  We need to 
  // convert them to indicies local to each graph.
  {
    int * graph0EndVtxs = new int [ boundaryCols.size() ];
    int * graph1StartVtxs = new int [ boundaryCols.size() ];
    const ScatterMap::glob2loc_t & graph0glob2loc = *(c_scatter[0]->getGlob2Loc());
    const ScatterMap::glob2loc_t & graph1glob2loc = *(c_scatter[1]->getGlob2Loc());
    int nBoundaryCols = boundaryCols.size();
    ScatterMap::glob2loc_t::const_iterator end0 = graph0glob2loc.end();
    ScatterMap::glob2loc_t::const_iterator end1 = graph1glob2loc.end();
    for ( int i =0; i<nBoundaryCols; ++i ) { 
      int vtx = boundaryCols[i];
      ScatterMap::glob2loc_t::const_iterator it0 = graph0glob2loc.find(vtx);
      ScatterMap::glob2loc_t::const_iterator it1 = graph1glob2loc.find(vtx);
      if ( it0 != end0 ) { 
	graph0EndVtxs[i] = (*it0).second;
      } else { 
	cerr << "Warning: no local alias for global vtx " << vtx << " in graph0." << endl;
      }
      if ( it1 != end1 ) { 
	graph1StartVtxs[i] = (*it1).second; 
      } else { 
	cerr << "Warning: no local alias for global vtx " << vtx << " in graph1." << endl;
      }
    }
    ordering0.setEndVtxs( graph0EndVtxs, nBoundaryCols );
    ordering0.enableForceEndVtxsLast();
    ordering1.setStartVtxs( graph1StartVtxs, nBoundaryCols );
    ordering1.enableMakeStartVtxsInWavefront();
    delete[] graph0EndVtxs;
    delete[] graph1StartVtxs;
  }

  ////////////////////////////////////////////////////////////////////////
  //
  // 7. Execute the sloan ordering
  //
  ordering0.execute();
  ordering1.execute();
	 
  //////////////////////////////////////////////////////////////////////
  //
  // 8. Construct global permutation vector from two sub permutations.
  //

  PermutationMap perm( nVtxs );
  {
    PermutationMap * perm0, *perm1;
    perm0 = ordering0.givePermutation();
    //    perm0 = new PermutationMap( K[0]->queryNVtxs() );
    //    perm0->getNew2Old().take( ordering0.getNew2Old() );
    perm0->validate();
    if (! perm0->isValid() ) { 
      cerr << "Error in creating valid permutation 0" << endl;
      exit(-1);
    }
    
    perm1 = ordering1.givePermutation();
    //    perm1 = new PermutationMap( K[1]->queryNVtxs() );
    //    perm1->getNew2Old().take( ordering1.getNew2Old() );
    perm1->validate();
    if (! perm1->isValid() ) { 
      cerr << "Error in creating valid permutation 1" << endl;
      exit(-1);
    }
    
    // Now begin the global permutation.
    int * globperm = new int[ nVtxs ]; // to be "taken" by PermutationMap
    const int * new2old = perm0->getNew2Old().lend();
    const int * graph0loc2glob = c_scatter[0]->getLoc2Glob().lend();
    const int * graph1loc2glob = c_scatter[1]->getLoc2Glob().lend();
    int nUndelayedCols = nVtxs - delayedCols.size();
    
    int nDelayedCols = delayedCols.size();
    for ( int i = 0; i<nUndelayedCols; ++i ) { 
      globperm[i] = graph0loc2glob[ new2old[i] ];
    }
   
    new2old = perm1->getNew2Old().lend(); // reset new2old to second graph
    for (int i = 0; i < nDelayedCols; ++i ) { 
      globperm[ i + nUndelayedCols ] = graph1loc2glob[ new2old[ i ] ];
    }

    perm.getNew2Old().take( globperm, nVtxs ); // no need to explicitly delete
    perm.validate();
    if (! perm.isValid() ) { 
      cerr << "Error in creating valid permutation 1" << endl;
      exit(-1);
    }
    delete perm0;
    delete perm1;
  }
  
  //////////////////////////////////////////////////
  //
  // Now print out the permutation, if requested
  // 
  if ( cmdLine.hasFlag("-fout") ) {
    FILE* output_file = NULL;
    if ( cmdLine.hasOption("-fout","stdout") ) {
      output_file = stdout; 
    } else {
      const char * output_file_name = cmdLine.getOption("-fout");
      if (( output_file = fopen( output_file_name,"w") ) == NULL ) {
	cerr << "Error: could not open outputfile\'" << output_file_name 
	     << "\'" << endl;
      }
    }
    
    bool printNew2Old = ( cmdLine.hasOption("-permtype","old2new") ) ?  false : true;
    bool onesOffset = false;
    bool printLength = false;
    if ( cmdLine.hasOption("-permformat","F77") ) {
	onesOffset = true;
    } else if ( cmdLine.hasOption("-permformat","len+C") ) {
      printLength = true;
    } else if ( cmdLine.hasOption("-permformat","len+F77") ) {
      onesOffset = true;
      printLength = true;
    }
 
    perm.prettyPrint( output_file, printNew2Old, printLength, onesOffset );
    
    if (output_file != stdout) {
      fclose(output_file);
    }
  }
  //////////////////////////////////////////////////////////////////////
  //
  // 10. Display quality statistics if requested.
  //
  if ( cmdLine.hasFlag("-s|-stats") ) { 
    float max_wf = 0.0; 
    float sum_wf = 0.0;
    float sum_wf_sq = 0.0;
    SharedArray<int>& wavefront0 = ordering0.getWavefront();
    for( unsigned int i=0; i<nVtxs-delayedCols.size(); i++ ) { 
      float wf = wavefront0[i];
      sum_wf += wf;
      sum_wf_sq += wf * wf; 
      max_wf = ( max_wf < wf ) ? wf : max_wf;
    }
    SharedArray<int>& wavefront1 = ordering1.getWavefront();
    for( unsigned int i=0; i<delayedCols.size(); i++ ) { 
      float wf = wavefront1[i];
      sum_wf += wf;
      sum_wf_sq += wf * wf; 
      max_wf = ( max_wf < wf ) ? wf : max_wf;
    }
    cout << "Statistics: " << endl;
    
    cout << "\t Graph:" << endl;
    cout << "\t\t Number vertices       : " << graph->queryNVtxs() << endl;
    cout << "\t\t Number edges          : " << graph->queryNEdges() << endl;
    cout << "\t\t Number non zeros      : " << graph->queryNNonZeros() << endl;
    cout << "\t\t Number Delayed Cols   : " << delayedCols.size() << endl;
    cout << "\t Graph0:" << endl;
    cout << "\t\t Number vertices       : " << K[0]->queryNVtxs() << endl;
    cout << "\t\t Number edges          : " << K[0]->queryNEdges() << endl;
    cout << "\t\t Number non zeros      : " << K[0]->queryNNonZeros() << endl;
    cout << "\t Graph1:" << endl;		   
    cout << "\t\t Number vertices       : " << K[1]->queryNVtxs() << endl;
    cout << "\t\t Number edges          : " << K[1]->queryNEdges() << endl;
    cout << "\t\t Number non zeros      : " << K[1]->queryNNonZeros() << endl;
    cout << "\t Sloan0 Class \t         : " << ordering0.getClass() << endl;
    cout << "\t Sloan1 Class \t         : " << ordering1.getClass() << endl;
    cout << "\t Envelope \t             : " << ( sum_wf - nVtxs ) << endl;
    cout << "\t Max Wavefront \t        : " << max_wf << endl;
    cout << "\t Mean Square Wavefront\t : " << ( sum_wf_sq / nVtxs ) << endl;
  }
  
  if (graph!= 0) { 
    delete graph;
    graph = 0;
  }

  delete K[0];
  delete K[1];
  delete scatterMap[0];
  delete scatterMap[1];
}

