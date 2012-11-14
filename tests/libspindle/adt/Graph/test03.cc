//
//  adt/Graph/drivers/tests/test02.cc -- tests Graph Masking
//
//  $Id: test03.cc,v 1.2 2000/02/18 01:32:06 kumfert Exp $
//
//  Gary Kumfert, Old Dominion University
//  Copyright(c) 1997, Old Dominion University.  All rights reserved.
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
// This tests the basic unweighted graph constructor and its iterators
//

#include <iostream.h>
#include "spindle/Graph.h"
#include "spindle/GraphUtils.h"
#include "spindle/ScatterMap.h"
#include "samples/small_mesh.h"

const int mask[] = { 1, 1, 0, 1, 1, 0, 0, 0, 0};

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

int main() {
  ios::sync_with_stdio(); 

  // all arrays in samples/ directory are const pointers.
  // this means the Graph doesn't sieze delete rights over the data
  Graph graph( nvtxs, Aptr, Aind );
  
  graph.validate(); // perform validation

  if ( ! graph.isValid() ) {
    cerr << "Error: graph is invalid" << endl;
    exit(-1);
  } else { 
    cout << "Graph created successfully." << endl;
  }

  Graph* subGraph;
  ScatterMap* scatterMap;

  bool success = GraphUtils::createSubgraph( &graph, mask, 
					     &subGraph, &scatterMap );
  if ( !success ) { 
    cerr << "create Subraph failed" << endl;
  }

  cout << "Original Graph" << endl;
  graph.dump( stdout );

  cout << endl << endl << "Sub Graph" << endl;
  subGraph->dump( stdout );

  cout << endl << endl << "Scatter Map" << endl;
  scatterMap->dump( stdout );

  Graph* subGraph2;
  ScatterMap* scatterMap2;
  success = GraphUtils::createSubgraph( &graph, mask, 
					     &subGraph2, &scatterMap2, true );
  if ( !success ) { 
    cerr << "create Subraph 2 failed" << endl;
  }

  cout << endl << endl << "Sub Graph 2" << endl;
  subGraph2->dump( stdout );

  cout << endl << endl << "Scatter Map 2" << endl;
  scatterMap2->dump( stdout );


  delete subGraph;
  delete scatterMap;
  delete subGraph2;
  delete scatterMap2;

}
