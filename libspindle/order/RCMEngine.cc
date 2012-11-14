//
// RCMEngine.cc
//
// $Id: RCMEngine.cc,v 1.2 2000/02/18 01:31:53 kumfert Exp $
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
//

#include "spindle/RCMEngine.h"

#ifdef REQUIRE_OLD_CXX_HEADER_SUFFIX
#include <iostream.h>
#else
#include <iostream>
using namespace std;
#endif

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

//static char invalidGraph[] = "Error: invalid graph detected.";

RCMEngine::RCMEngine() {
  incrementInstanceCount( RCMEngine::MetaData );
  bfs = new BreadthFirstSearch();
  diam = new PseudoDiameter();
  reset();
}


RCMEngine::RCMEngine( const Graph* graph ) {
  incrementInstanceCount( RCMEngine::MetaData );
  bfs = new BreadthFirstSearch();
  diam = new PseudoDiameter();
  reset();
  setGraph( graph );
}

RCMEngine::~RCMEngine() {
  delete diam; 
  delete bfs;
  // permutation is an SharedPtr and will delete itself if need be
  decrementInstanceCount( RCMEngine::MetaData );
}

bool 
RCMEngine::setGraph( const Graph* graph ) {
  return  OrderingAlgorithm::setGraph( graph );
}

bool
RCMEngine::reset() {
  //
  // disable all options.
  // 
  //  disablePartitionRestrictions();

  // erase all information from previous runs.
  nVtxsNumbered = 0; // set number of vertices numbered back to 0
  ordering_timer.reset();
  diagnostic_timer.reset();
  bfs->reset();
  diam->reset();
  return OrderingAlgorithm::reset();
}

bool 
RCMEngine::setPseudoDiamShrinkingStrategy( PseudoDiamShrinkingStrategy * shrink ) {
  if ( algorithmicState == INVALID ) { return false; }
  if ( shrink == 0 ) { return false; }
  if ( diam == 0 ){
    diam = new PseudoDiameter();
  }
  return diam->setShrinkingStrategy( shrink );
}
bool 
RCMEngine::execute() {
  if ( algorithmicState != READY ) { return false; }
  
  const Graph* g;
  int * old2New;
  if ( useGraphCompression ) { 
    g = coarseGraph;
    old2New = coarsePermutation->getOld2New().begin();
  } else { 
    g = fineGraph;
    old2New = finePermutation->getOld2New().begin();
  }
    
  diagnostic_timer.start();

  bfs->setGraph( g );
  diam->setGraph( g );

  //
  // 1. use RCM to number the rest
  //
  const int n = g->size();
  {for (int i=0; i<n ; ++i) {       
    if (old2New[i] != -1) { continue; }   // for all vertices not yet numbered...
    if (g->deg(i) < 1 ) {                 //    if its degree < 1
      old2New[i] = nVtxsNumbered++;       //        number it now
      if ( nVtxsNumbered >= n ) {         //        and check if we're done
	break; 
      }
    } else {                              //    else if degree > 1
      //  find the connected component
      bfs->setRoot(i);			
      bfs->execute();                     

      //  find vtx of minimum degree in component
      const int* vertexList = bfs->getNew2Old().lend();  
      int cur_min_idx = vertexList[0];
      int cur_min = g->deg( cur_min_idx );
      {for(int j=1; j<bfs->queryNVisited(); ++j) {
	if ( g->deg( vertexList[j] ) < cur_min ) {
	  cur_min_idx = vertexList[j];
	  cur_min = g->deg( cur_min_idx );
	} 
      }}
    
      // now compute a psuedo-diameter on that component
      // starting from the vertex of minimum degree
      diam->setRoot( cur_min_idx );
      diam->execute();
      pair<int, int> endpoint = diam->queryEndpoints();

      // perform the ordering on that component
      executeFrom( endpoint.second , g, old2New);
      if (nVtxsNumbered == n) { // if I've numbered them all
	diagnostic_timer.stop(); 
	algorithmicState = DONE;
	return OrderingAlgorithm::recomputePermutations();
      }
    }
  }}
  // gets here if we number individual
  // vertices last
  algorithmicState = DONE;
  diagnostic_timer.stop();
  return OrderingAlgorithm::recomputePermutations();
}

bool RCMEngine::executeFrom( const int endVtx, const Graph* g, int * old2New ) {
  //
  // 1. Verify that startVtx and endVtx are valid vertices 
  //
  if ( (endVtx<0) || (endVtx>g->size()) ) { return false; }
  if ( old2New[endVtx] != -1 ) { return false; }
  ordering_timer.start();

  //
  // 2. do breadth-first-search from endVtx.
  // 
  bfs->setRoot( endVtx );
  bfs->execute();

  const int* vtxList = bfs->getNew2Old().lend();
  int nVtxs = bfs->queryNVisited();
  
  {for(int i=nVtxs-1; i>=0; i--) { 
    old2New[vtxList[i]] = nVtxsNumbered++;
  }}
  ordering_timer.stop();
  return true;
}

SPINDLE_IMPLEMENT_DYNAMIC( RCMEngine, SpindleAlgorithm )

