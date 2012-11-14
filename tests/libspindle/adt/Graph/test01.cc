//
//  adt/Graph/drivers/tests/test01.cc -- tests undirected graphs
//
//  $Id: test01.cc,v 1.2 2000/02/18 01:32:06 kumfert Exp $
//
//  Gary Kumfert, Old Dominion University
//  Copyright(c) 1998, Old Dominion University.  All rights reserved.
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
// This builds several instances of graph, exercising all the constructors
//

#include <iostream.h>
#include "spindle/Graph.h"
#include "samples/small_mesh.h"

static const int vtx_weights[9] = { 3, 2, 3, 2, 1, 2, 3, 2, 3};
static const int edge_weights[33] =  { 0, 6, 6,
				       6, 0, 6, 2, 
				       6, 0, 6,
				       6, 0, 2, 6,
				       2, 2, 0, 2, 2, 
				       6, 2, 0, 6, 
				       6, 0, 6, 
				       2, 6, 0, 6,
				       6, 6, 0 };

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

int main() {
  ios::sync_with_stdio(); 

  // all arrays in samples/ directory are const pointers.
  // this means the Graph doesn't sieze delete rights over the data
  Graph * graph = new Graph ( nvtxs, Aptr, Aind );
  
  graph->validate(); // perform validation

  if ( ! graph->isValid() ) {
    cerr << "Error: graph is invalid" << endl;
    exit(-1);
  } else { 
    cout << "Graph created successfully." << endl;
  }

  graph->dump( stdout );

  delete graph;
  
  graph = new Graph ( nvtxs, Aptr, Aind, vtx_weights, edge_weights );
  
  graph->validate(); // perform validation

  if ( ! graph->isValid() ) {
    cerr << "Error: graph is invalid" << endl;
    exit(-1);
  } else { 
    cout << "Graph created successfully." << endl;
  }

  graph->dump( stdout );

  delete graph;

  graph = new Graph ( nvtxs, Aptr, Aind, 0, edge_weights );
  
  graph->validate(); // perform validation

  if ( ! graph->isValid() ) {
    cerr << "Error: graph is invalid" << endl;
    exit(-1);
  } else { 
    cout << "Graph created successfully." << endl;
  }

  graph->dump( stdout );

  delete graph;

  graph = new Graph ( nvtxs, Aptr, Aind, vtx_weights, 0 );
  
  graph->validate(); // perform validation

  if ( ! graph->isValid() ) {
    cerr << "Error: graph is invalid" << endl;
    exit(-1);
  } else { 
    cout << "Graph created successfully." << endl;
  }

  graph->dump( stdout );

  delete graph;

}

