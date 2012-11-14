//
//  adt/Graph/drivers/tests/test02.cc -- tests undirected graphs
//
//  $Id: test02.cc,v 1.2 2000/02/18 01:32:06 kumfert Exp $
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
#include "samples/small_mesh.h"

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

  graph.dump( stdout );

  cout << "Testing the adjacency iterators over the graph" << endl;
  const int n = graph.size();
  {for(int i=0; i<n; i++) {
    cout << "adj(" << i << ") :";
    for(Graph::const_iterator cur=graph.begin_adj(i),stop=graph.end_adj(i); 
	cur != stop; ++cur) {
      cout << *cur << ", ";
    }
    cout << endl;
  }}

  cout << "Testing the high adjacency iterators over the graph" << endl;
  {for(int i=0; i<n; i++) {
    cout << "hadj(" << i << ") :";
    for(Graph::const_iterator cur=graph.begin_hadj(i),stop=graph.end_hadj(i); 
	cur != stop; ++cur) {
      cout << *cur << ", ";
    }
    cout << endl;
  }}

  cout << "Testing the low adjacency iterators over the graph" << endl;
  {for(int i=0; i<n; i++) {
    cout << "ladj(" << i << ") :";
    for(Graph::const_iterator cur=graph.begin_ladj(i),stop=graph.end_ladj(i); 
	cur != stop; ++cur) {
      cout << *cur << ", ";
    }
    cout << endl;
  }}

}
