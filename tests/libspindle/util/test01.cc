//
// src/util/drivers/test/test01.cc 
//
// $Id: test01.cc,v 1.2 2000/02/18 01:32:11 kumfert Exp $
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
// This exercises some of the BreadthFirstSearch capabilities
//

#include <iostream.h>
#include "spindle/SpindleSystem.h"
#include "spindle/BreadthFirstSearch.h"
#include "samples/small_mesh.h"

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

static void printAll(const BreadthFirstSearch& bfs);

#ifdef __FUNC__
#undef __FUNC__
#endif

int main() {
  Graph graph(nvtxs,Aptr,Aind);
  graph.validate();
  if ( ! graph.isValid() ) {
    cerr << "Cannot validate graph" << endl;
  }

  cout << "Testing the adjacency iterators over the graph" << endl;
  { for(int i=0; i<graph.size(); i++) {
    cout << "adj(" << i << ") :";
    for(Graph::const_iterator cur=graph.begin_adj(i),stop=graph.end_adj(i); 
	cur != stop; ++cur) {
      cout << *cur << ", ";
    }
    cout << endl;
  }}
  
  cout << endl << "Creating the Breadth First Search (BFS) object ... ";
  BreadthFirstSearch bfs(&graph);
  cout << " done. " << endl;
  printAll( bfs );
  
  cout << "Doing BFS from vertex 2 ... ";
  bfs.setRoot(2);
  if ( bfs.execute() ) {
    cout << " done. " << endl;
  } else {
    cout << " problem detected ! " << endl;
  }
  printAll( bfs );
  //  bfs.reset();

  cout << "Doing BFS from vertex 0 with a partition ... ";
  bfs.setRoot(0);
  int partitionMap[] = { 1, 1, 1, 3, 3, 3, 2, 2, 2};
  if (! bfs.enablePartitionRestrictions(partitionMap)) { 
    cout << "problem setting partition vector" << endl;
  } else if ( bfs.execute() ) {
    cout << " done. " << endl;
  } else {
    cout << " problem detected ! " << endl;
    cout << bfs.queryErrMsg() << endl;
  }
  printAll( bfs );
  bfs.disablePartitionRestrictions();
  //  bfs.reset();

  cout << "Doing BFS from vertex 0 with a partition and loose restrictions... ";
  bfs.setRoot(0);
  if (! bfs.enablePartitionRestrictions(partitionMap,BreadthFirstSearch::loose)) { 
    cout << "problem setting partition vector" << endl;
  } else if ( bfs.execute() ) {
    cout << " done. " << endl;
  } else {
    cout << " problem detected ! " << endl;
    cout << bfs.queryErrMsg() << endl;
  }
  printAll( bfs );
  bfs.disablePartitionRestrictions();
  //  bfs.reset();

  int partitionMap2[] = { 1, 1, 1, 3, 3, 3, 2, 2, 1};
  cout << "Doing BFS from vertex 0 with a new partition and loose restrictions... ";
  bfs.setRoot(0);
  if (! bfs.enablePartitionRestrictions(partitionMap2,BreadthFirstSearch::loose)) { 
    cout << "problem setting partition vector" << endl;
  } else if ( bfs.execute() ) {
    cout << " done. " << endl;
  } else {
    cout << " problem detected ! " << endl;
    cout << bfs.queryErrMsg() << endl;
  }
  printAll( bfs );
  bfs.disablePartitionRestrictions();
  //  bfs.reset();

  cout << "Doing BFS from vertex 0 with shortCircuit = 3 ... ";
  bfs.setRoot(0);
  if (! bfs.enableShortCircuiting(3) ) {
    cout << "problem setting shortCircuit = 3" << endl;
    cout << bfs.queryErrMsg() << endl;
  } else if ( bfs.execute() ) {
    if ( bfs.hasShortCircuited() ) {
      cout << "short-circuited!" << endl;
    } else {
      cout << " done. " << endl;
    }
  } else {
    cout << " problem detected ! " << endl;
    cout << bfs.queryErrMsg() << endl;
  }
  printAll( bfs );
  //  bfs.reset();
  
  cout << "Doing BFS from vertex 0 with shortCircuit = 2 ... ";
  bfs.setRoot(0);
  if (! bfs.enableShortCircuiting(2) ) {
    cout << "problem setting shortCircuit = 2" << endl;
    cout << bfs.queryErrMsg() << endl;
  } else if ( bfs.execute() ) {
    if ( bfs.hasShortCircuited() ) {
      cout << "short-circuited!" << endl;
    } else {
      cout << " done. " << endl;
    }
  } else {
    cout << " problem detected ! " << endl;
    cout << bfs.queryErrMsg() << endl;
  }
  printAll( bfs );
}

#define __FUNC__ "static void printAll(const BreadthFirstSearch& bfs)"
static void printAll(const BreadthFirstSearch& bfs) {
  FENTER;
  int nVisited = bfs.queryNVisited();
  static stopwatch timer;

  cout << "  BFS.nVisited    = " << bfs.queryNVisited() << endl;
  cout << "  BFS.nFarthest   = " << bfs.queryNFarthest() << endl;
  cout << "  BFS.height      = " << bfs.queryHeight() << endl;
  cout << "  BFS.width       = " << bfs.queryWidth() << endl;
//  cout << "  BFS.userTime    = " << (float) bfs.getTimer().queryTotalUserTicks()/timer.queryTicksPerSec() << endl;
//  cout << "  BFS.systemTime  = " << (float) bfs.getTimer().queryTotalSystemTicks()/timer.queryTicksPerSec() << endl;
  cout << "  BFS.maxDegree   = " << bfs.queryMaxDegree() << endl;
  cout << "  BFS.maxDegreeVtx= " << bfs.queryMaxDegreeVtx() << endl;
  cout << "  BFS.minDegree   = " << bfs.queryMinDegree() << endl;
  cout << "  BFS.minDegreeVtx= " << bfs.queryMinDegreeVtx() << endl;

  FCALL const SharedArray<int>& perm = bfs.getNew2Old();
  cout << "  BFS.new2old = " << endl;
  { for(int i=0; i<nVisited; i++) {
    cout <<"            [ " << i << "] = " << perm[i] << ",  dist = " << bfs.queryDistance(perm[i]) << endl;
  }}
  cout << endl;
  FEXIT;
}
#undef __FUNC__
