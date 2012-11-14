//
// src/util/drivers/tests/test02.cc
//
// $Id: test02.cc,v 1.2 2000/02/18 01:32:11 kumfert Exp $
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


#include <iostream.h>
#include "spindle/Graph.h"
#include "spindle/BreadthFirstSearch.h"
#include "spindle/PseudoDiameter.h"
#include "samples/small_mesh.h"

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

static void printAll(const PseudoDiameter& diam);

int main() {
  Graph graph(nvtxs,Aptr,Aind);
  graph.validate();
  if ( ! graph.isValid() ) {
    cerr << "Cannot create valid graph." << endl;
  }

  cout << "Testing the adjacency iterators over the graph" << endl;
  for(int i=0; i<graph.size(); i++) {
    cout << "adj(" << i << ") :";
    for(Graph::const_iterator cur=graph.begin_adj(i),stop=graph.end_adj(i); 
	cur != stop; ++cur) {
      cout << *cur << ", ";
    }
    cout << endl;
  }
  
  cout << endl << "Creating the Pseudo Diameter objects";
  PseudoDiameter diam(&graph);
  cout << " done. " << endl;
  printAll( diam );
  
  cout << "Computing Diam from vertex 0 ... ";
  diam.setRoot(0);
  if ( diam.execute() ) {
    cout << " done. " << endl;
  } else {
    cout << " problem detected ! " << endl;
  }
  printAll( diam );
  diam.reset();  // needed to reset timers and nBFS

  cout << "Computing Diam from vertex 2 ... ";
  diam.setGraph( &graph );
  diam.setRoot(2);
  if ( diam.execute() ) {
    cout << " done. " << endl;
  } else {
    cout << " problem detected ! " << endl;
  }
  printAll( diam );
  diam.reset();  // needed to reset timers and nBFS

  cout << "Computing Diam from vertex 4 ... ";
  diam.setGraph( &graph );
  diam.setRoot(4);
  if ( diam.execute() ) {
    cout << " done. " << endl;
  } else {
    cout << " problem detected ! " << endl;
  }
  printAll( diam );
  diam.reset();  // needed to reset timers and nBFS

  int partitionMap[] = { 1, 1, 1, 3, 3, 3, 2, 2, 2};

  cout << "Computing Diam with strict partition restrictions from 0... ";
  diam.setGraph( &graph );
  diam.enablePartitionRestrictions( partitionMap, PseudoDiameter::strict );
  diam.setRoot( 0 );
  if ( diam.execute() ) {
    cout << " done. " << endl;
  } else {
    cout << " problem detected ! " << endl;
  }
  printAll( diam );
  diam.reset();  // needed to reset timers and nBFS

  cout << "Computing Diam with strict partition restrictions from 4... ";
  diam.setGraph( &graph );
  diam.enablePartitionRestrictions( partitionMap, PseudoDiameter::strict );
  diam.setRoot( 4 );
  if ( diam.execute() ) {
    cout << " done. " << endl;
  } else {
    cout << " problem detected ! " << endl;
  }
  printAll( diam );
  diam.reset();  // needed to reset timers and nBFS

  cout << "Computing Diam with loose partition restrictions from 0... ";
  diam.setGraph( &graph );
  diam.enablePartitionRestrictions( partitionMap, PseudoDiameter::loose );
  diam.setRoot( 0 );
  if ( diam.execute() ) {
    cout << " done. " << endl;
  } else {
    cout << " problem detected ! " << endl;
  }
  printAll( diam );
  diam.reset();  // needed to reset timers and nBFS

  cout << "Computing Diam with loose partition restrictions from 4... ";
  diam.setGraph( &graph );
  diam.enablePartitionRestrictions( partitionMap, PseudoDiameter::loose );
  diam.setRoot( 4 );
  if ( diam.execute() ) {
    cout << " done. " << endl;
  } else {
    cout << " problem detected ! " << endl;
  }
  printAll( diam );
  diam.reset();  // needed to reset timers and nBFS

  int partitionMap2[] = { 1, 1, 1, 3, 3, 3, 1, 2, 2};
  
  cout << "Computing Diam with loose partition restrictions from 0 (using paritionMap2)... ";
  diam.setGraph( &graph );
  diam.enablePartitionRestrictions( partitionMap2, PseudoDiameter::loose );
  diam.setRoot( 0 );
  if ( diam.execute() ) {
    cout << " done. " << endl;
  } else {
    cout << " problem detected ! " << endl;
  }
  printAll( diam );
  diam.reset();  // needed to reset timers and nBFS

  int partitionMap3[] = { 3, 1, 3, 2, 3, 2, 3, 1, 3};

  cout << "Computing Diam with loose partition restrictions from 4 (using partitionMap3)... ";
  diam.setGraph( &graph );
  diam.enablePartitionRestrictions( partitionMap3, PseudoDiameter::loose );
  diam.setRoot( 4 );
  if ( diam.execute() ) {
    cout << " done. " << endl;
  } else {
    cout << " problem detected ! " << endl;
  }
  printAll( diam );
  diam.reset();  // needed to reset timers and nBFS

}


static void printAll(const PseudoDiameter &diam) {
  static stopwatch timer;
  pair<int, int> endpoint = diam.queryEndpoints();

  cout << "  diam.endpoints  = " << "<" << endpoint.first << ", " << endpoint.second << ">" << endl;
  cout << "  diam.diameter   = " << diam.queryDiameter() << endl;
  cout << "  diam.nVisited   = " << diam.queryNVisited() << endl;
  cout << "  diam.nBFS       = " << diam.queryNBFS() << endl;
//  cout << "  diam.userTime   = " << (float) diam.getTimer().queryTotalUserTicks()/timer.queryTicksPerSec() << endl;
//  cout << "  diam.systemTime = " << (float) diam.getTimer().queryTotalSystemTicks()/timer.queryTicksPerSec() << endl;

  const stopwatch& forwardTimer = diam.getForwardBFS()->getTimer();
  const stopwatch& reverseTimer = diam.getReverseBFS()->getTimer();
 /*
  cout << "  diam.BFS.Time   = "  
       << ( forwardTimer.queryTotalUserTicks() + reverseTimer.queryTotalUserTicks()  + 
	    forwardTimer.queryTotalSystemTicks() + reverseTimer.queryTotalSystemTicks() )/timer.queryTicksPerSec() 
       << endl << endl;;
 */
}  
