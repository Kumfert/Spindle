//
// SloanEngine.cc
//
// $Id: SloanEngine.cc,v 1.2 2000/02/18 01:31:54 kumfert Exp $
//
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
//

#include "spindle/SloanEngine.h"

#ifndef SPINDLE_GRAPH_COMPRESSOR_H_
#include "spindle/GraphCompressor.h"
#endif

#ifdef REQUIRE_OLD_CXX_HEADER_SUFFIX
#include <iostream.h>
#include <limits.h>
#include <algo.h>
#else
#include <iostream>
#include <climits>
#include <algorithm>
using namespace std;
#endif

#ifndef LARGE_INT
#  ifndef MAX_INT
//#    define LARGE_INT 2147483648 // 2^31
#      define LARGE_INT 2000000000 // 2,000,000,000 large enough
#  else
#    define LARGE_INT MAX_INT
#  endif
#endif

#ifndef SMALL_INT
#  ifndef MIN_INT
#    define SMALL_INT ( -(LARGE_INT-1) )
#  else
#    define SMALL_INT MIN_INT
#  endif
#endif

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif


  //static char invalidGraph[] = "Invalid graph";

SloanEngine::SloanEngine() {
  incrementInstanceCount( SloanEngine::MetaData );
  bfs = 0;
  heap = 0;
  reset();
}

SloanEngine::SloanEngine( const Graph* graph ) {          // unweighted Graph constructor
  incrementInstanceCount( SloanEngine::MetaData );
  bfs = 0;
  heap = 0;
  reset();
  setGraph( graph );
}  


SloanEngine::~SloanEngine() {
  delete heap;
  delete bfs;
  decrementInstanceCount( SloanEngine::MetaData );
}

bool
SloanEngine::setGraph( const Graph* graph ) {
  return OrderingAlgorithm::setGraph( graph );
}

bool
SloanEngine::reset() {
  //
  // disable all options.
  // 
  g = 0;
  nVtxsTotal = 0;
  setClass( 0 );
  VertexWeight = 0;
  forceEndVtxsLast = false;
  makeStartVtxsInWavefront = false;

  disablePartitionRestrictions();
  disableRefinement();
  disableUserGlobalPriority();

  // erase all information from previous runs.
  nVtxsNumbered = 0; // set number of vertices numbered back to 0

  if ( bfs != 0 ) {
    bfs->reset();
  }
  totalExecutionTimer.reset();
  orderComponentTimer.reset();
  numberVerticesKeptTimer.reset();
  numberVerticesRejectedTimer.reset();
  if ( diam.notNull() ) { diam->reset(); }

  startVtxs.resize(0);
  endVtxs.resize(0);
  algorithmicState = EMPTY;
  return true;
}

bool 
SloanEngine::setPseudoDiameterShrinkingStrategy( PseudoDiamShrinkingStrategy * shrink ) {
  if ( algorithmicState == INVALID ) { return false; }
  if ( shrink == 0 ) { return false; }
  if ( diam.isNull() ) {
    diam.take( new PseudoDiameter() );
  }
  return diam->setShrinkingStrategy( shrink );
}

bool 
SloanEngine::setClass( const int classNumber ) {
  switch (classNumber) {
  case 0:
    classOfGraph = DONT_KNOW;
    break;
  case 1:
    classOfGraph = CLASS_1;
    break;
  case 2:
    classOfGraph = CLASS_2;
    break;
  default:
    return false;
  }
  weightingScheme = DETERMINED_BY_CLASS;
  normalizedWeighting = ENABLED;
  return true;
}

int 
SloanEngine::getClass() { 
  switch ( classOfGraph ) {
  case DONT_KNOW:
    return 0;
    break;
  case CLASS_1:
    return 1;
    break;
  case CLASS_2:
    return 2;
    break;
  default:
    return -1;
    break;
  }
}

bool 
SloanEngine::setStartVtx( const int newStartVtx ) { 
  if ( (newStartVtx < 0) || (newStartVtx>=nVtxsTotal )) { return false; }
  startVtxs.push_back( newStartVtx ); 
  return true; 
}

bool 
SloanEngine::setStartVtxs( const int *newStartVtxs, const int nNewStartVtxs) { 
  if ( newStartVtxs == 0 ) { return false; }
  if ( (nNewStartVtxs < 0 ) || (nNewStartVtxs >= nVtxsTotal ) ) { return false; }
  startVtxs.insert( startVtxs.end(), newStartVtxs, newStartVtxs + nNewStartVtxs ); 
  return true; 
}

bool 
SloanEngine::setEndVtx( const int newEndVtx ) { 
  if ( ( newEndVtx < 0 ) || ( newEndVtx >= nVtxsTotal ) ) { return false; }
  endVtxs.push_back( newEndVtx ); 
  return true; 
}

bool 
SloanEngine::setEndVtxs( const int *newEndVtxs, const int nNewEndVtxs) { 
  if ( newEndVtxs == 0 ) { return false; }
  if ( ( nNewEndVtxs < 0 ) || ( nNewEndVtxs >= nVtxsTotal ) ) { return false; }
  endVtxs.insert( endVtxs.end(), newEndVtxs, newEndVtxs + nNewEndVtxs );
  return true; 
}

bool 
SloanEngine::resetStartVtxs() { 
  startVtxs.resize(0);
  return true; 
}

bool 
SloanEngine::resetEndVtxs() { 
  endVtxs.resize(0); 
  return true; 
}


void 
SloanEngine::findStartVtxs() {
  // user has specified end vtxs, but we also need start vtxs
  if ( bfs == 0 ) {
    bfs = new BreadthFirstSearch( g );
  }
  bfs->resetRoots();
  bfs->setRoots( endVtxs.begin(), endVtxs.size() );
  bfs->execute();
  int nVisited = bfs->queryNVisited();
  int nFarthest = bfs->queryNFarthest();
  const int* new2old = bfs->getNew2Old().lend();
  setStartVtxs( new2old + nVisited - nFarthest, nFarthest );
}

void 
SloanEngine::findEndVtxs() {
  // user has specified end vtxs, but we also need start vtxs
  if ( bfs == 0 ) {
    bfs = new BreadthFirstSearch( g );
  }
  bfs->resetRoots();
  bfs->setRoots( startVtxs.begin(), startVtxs.size() );
  bfs->execute();
  int nVisited = bfs->queryNVisited();
  int nFarthest = bfs->queryNFarthest();
  const int* new2old = bfs->getNew2Old().lend();
  setEndVtxs( new2old + nVisited - nFarthest, nFarthest );
}

#define __FUNC__ "bool SloanEngine::execute()"
bool 
SloanEngine::execute() {
  if ( algorithmicState != READY ) { return false; }

  //
  // assign the engine to work on the correct graph and permutation
  //
  //  int * old2New;
  if ( useGraphCompression ) { 
    g = coarseGraph;
    //old2New = coarsePermutation->getOld2New().begin();
  } else { 
    g = fineGraph;
    //    old2New = finePermutation->getOld2New().begin();
  }
    
  // if this is the first run for this graph, then
  // there are some additional details to take care of
  if ( nVtxsNumbered == 0 ) { 
    const int n = g->queryNVtxs();
    if ( bfs == 0 ) {
      bfs = new BreadthFirstSearch( g );
    } else {
      bfs->setGraph( g );
    }
    if ( heap == 0 ) {
      heap = new Heap( n, n );
    }
    if ( diam.isNull() ) {
      diam.take( new PseudoDiameter( g ) );
    } else {
      diam->setGraph( g );
    }
    if ( n != nVtxsTotal ) {
      nVtxsTotal = n;
      heap->resize(n,n);
      status.resize( n );
      wavefront.resize( n );
      oldWavefront.resize( n );
      initial_priority.resize( n );
      new2Old_1.resize(n);
      new2Old_2.resize(n);
    }
    if (g->getVtxWeight().size() != 0) {
      vtxWeight.borrow( g->getVtxWeight() );
    }
    nodeStatus *sp;    
    sp = status.begin(); {for(int i=0; i<n; i++) { *sp++ = INACTIVE; }} 
    wavefront.init(-1);
    oldWavefront.init(-1);
  } // end if nVtxsNumbered ==0

  // finally, if we are using the compressed graph, the user doesn't know
  // and we should remap startVtxs and endVtxs to the compressed vtxs
  if ( useGraphCompression ) { 
    const GraphCompressor * c_compressor = compressor;
    const int * cmap = c_compressor->getFine2Coarse()->lend();
    if ( startVtxs.size() > 0 ) { 
      for ( vector<int>::iterator cur = startVtxs.begin(), stop = startVtxs.end(); 
	    cur != stop; ++cur ) { 
	*cur = cmap[ *cur ];
      }
      sort( startVtxs.begin(), startVtxs.end() );
      vector<int>::iterator newend = unique( startVtxs.begin(), startVtxs.end() );
      startVtxs.erase( newend, startVtxs.end() );
    }
    if ( endVtxs.size() > 0 ) { 
      for ( vector<int>::iterator cur = endVtxs.begin(), stop = endVtxs.end(); 
	    cur != stop; ++cur ) { 
	*cur = cmap[ *cur ];
      }
      sort( endVtxs.begin(), endVtxs.end() );
      vector<int>::iterator newend = unique( endVtxs.begin(), endVtxs.end() );
      endVtxs.erase( newend, endVtxs.end() );
    }
  }

  totalExecutionTimer.start();
  const int action = ( ( startVtxs.size() != 0 ) * 10 ) + ( endVtxs.size() != 0 );
  switch( action ) {
  case 0:
    // user specified neither start nor end vtxs.  simply order everything
    orderEverything();
    break;
  case 1:
    // user specified endVtxs, but not start vtxs
    findStartVtxs();
    orderComponent();
    break;
  case 10:
    // user specified startVtxs, but not endVtxs;
    findEndVtxs();
    orderComponent();
    break;
  case 11:
    // user specified both startVtxs and endVtxs;
    orderComponent();
    break;
  default:
    // error: shouldn't get here
    ERROR(SPINDLE_ERROR_UNEXP_CASE, "Invalid case %d, detected", action );
    totalExecutionTimer.stop();
    return false;
    break;
  }
  totalExecutionTimer.stop();
  if (nVtxsNumbered >= g->size()) { 
    algorithmicState = DONE;
    return OrderingAlgorithm::recomputePermutations(new2Old_1.begin());
  }
  return true;
}
#undef __FUNC__

bool
SloanEngine::orderEverything() {
  // user specified neither... use pseudodiameter on
  // each connected component

  //  int* new2Old = perm->getNew2Old().begin();
  //  int* new2Old2 = oldPerm->getNew2Old().begin();
  int * new2old_1 = new2Old_1.begin();
  int * new2old_2 = new2Old_2.begin();
  //
  // 1. use Sloan to number the rest.
  //
  {for (int i=0; i<g->size(); ++i) {            // for all vertices
    if (status[i] == NUMBERED) { continue; }   // ... not yet numbered
    if (( status[i] == INACTIVE ) && (g->deg(i) <  1) ) { // number small ones
      new2old_1[ nVtxsNumbered ] = i;
      new2old_2[ nVtxsNumbered ] = i;
      status[i] = NUMBERED;
      wavefront[nVtxsNumbered++] = 1;
      continue;
    }
    // now find the connected component
    bfs->setRoot(i);
    bfs->execute();                    
    
    // determine the size of the connected component
    // const int* vertexList =  bfs->getNew2Old().lend(); // unused
    // int nVisited = bfs->queryNVisited(); // unused
    
    // find a vertex in the connected component of minimum degree
    int cur_min_idx = bfs->queryMinDegreeVtx();
    
    // compute the diameter of that connected component
    if ( diam.isNull() ) {
      diam.take( new PseudoDiameter(g) );
    }
    diam->setRoot( cur_min_idx );
    diam->execute();
    pair<int, int> endpoint = diam->queryEndpoints();
    
    // set the start and end vertices appropriately
    resetStartVtxs();
    resetEndVtxs();
    setStartVtx( endpoint.first );
    setEndVtx( endpoint.second );
    orderComponent();  // then perform the ordering on the component.
    if (nVtxsNumbered >= g->size()) { 
      algorithmicState = DONE;
      return true;
    }
  }}
  return false; // shouldn't get this far
}

      
bool
SloanEngine::orderComponent() {
  // PRECONDITION:  startVtxs has at least one valid startVtxs
  //                endVtxs has at least one valid endVtx
  // POSTCONDITION: all vertices reachable from startVtx, endVtx are numbered
  
  //
  // 1. Verify that startVtx and endVtx are specified
  //
  if ( (startVtxs.size()==0) || (endVtxs.size()==0) ) { return false; } 
  
  //   if ( (status[startVtx] != INACTIVE) || (status[endVtx] != INACTIVE) ) { return false; }
  // later
  orderComponentTimer.start();

  //
  // 2. do breadth-first-search from endVtx.
  // 
  // ???  bfs->reset();
  bfs->setRoots( endVtxs.begin(), endVtxs.size() );
  bfs->execute();
  {for( int i=0, n=startVtxs.size(); i<n; ++i) {
    if( bfs->queryDistance( startVtxs[i] ) < 0 ) { // No negative distances !
      // Start vertex is not reachable from end vertex in this case.
      // Try disabling partition restrictions.
      orderComponentTimer.stop();
      return false;
    }
  }}
  const int *vtxList = bfs->getNew2Old().lend();
  int nVisited = bfs->queryNVisited();

  //
  // 3. generate the correct weights for the specified weighting scheme.
  //
  if ( weightingScheme == DETERMINED_BY_USER ) {
    numberVerticesKeptTimer.start();
    numberVertices( vtxList, nVisited );
    numberVerticesKeptTimer.stop();
  } else if (weightingScheme == DETERMINED_BY_CLASS) {
    int normFactor;
    if ( normalizedWeighting == ENABLED ) {
      // generate normalization values
      int max_deg = bfs->queryMaxDegree();
      normFactor = bfs->queryHeight() / max_deg;
      normFactor = (normFactor == 0) ? 1 : normFactor; // This happens often in LP problems.
    }
    // set Global and Local weights
    switch ( classOfGraph ) {
    case CLASS_1:
      LocalWeight = 8 * normFactor;
      GlobalWeight = 1;
      VertexWeight = ( vtxWeight.isNull() ) ? 0 : 1;
      numberVerticesKeptTimer.start();
      numberVertices( vtxList, nVisited );
      numberVerticesKeptTimer.stop();
      break;

    case CLASS_2:
      LocalWeight = normFactor;
      GlobalWeight = 2;
      VertexWeight = ( vtxWeight.isNull() ) ? 0 : 2;
      numberVerticesKeptTimer.start();
      numberVertices( vtxList, nVisited );
      numberVerticesKeptTimer.stop();
      break;

    case DONT_KNOW:
      // First Try CASE_1
      stopwatch case1Overhead;

      LocalWeight = 8 * normFactor;
      GlobalWeight = 1;
      VertexWeight = ( vtxWeight.isNull() ) ? 0 : 1;
      int begin = nVtxsNumbered;
      case1Overhead.start();
      numberVertices( vtxList, nVisited );
      case1Overhead.stop();
      int end = nVtxsNumbered;

      // Then save the result in oldPerm and oldWavefront
      wavefront.swap( oldWavefront );
      new2Old_1.swap( new2Old_2 );

      // Now try CASE_2
      stopwatch case2Overhead;

      LocalWeight = normFactor;
      GlobalWeight = 2;
      VertexWeight = (  vtxWeight.isNull() ) ? 0 : 2;
      setNVtxsNumbered( begin );
      case2Overhead.start();
      numberVertices( vtxList, nVisited );
      case2Overhead.stop();
      //if ( nVtxsNumbered != end ) { 
      //cerr << "Error: did not find the same number of vertices second time" << endl;
      //exit(-1);
      //}

      // Now compare the wavefronts of the two
      float wfsq_class1 = 0;
      float wfsq_class2 = 0;
      {for(int i=begin; i<end; i++) { 
	float wf = oldWavefront[i];
	wfsq_class1 += wf * wf;
      }}
      {for(int i=begin; i<end; i++) { 
	float wf = wavefront[i]; 
	wfsq_class2 += wf * wf;
      }}
      
      wavefront.swap( oldWavefront );
      new2Old_1.swap( new2Old_2 );
      if (wfsq_class2 <= wfsq_class1) {
	// if wavefront^2(CASE_2) <= wavefront^2(CASE_1) do nothing
	numberVerticesKeptTimer += case2Overhead;
	numberVerticesRejectedTimer += case1Overhead;
	setClass(2);
	// copy the regions back to the array
	memcpy( wavefront.begin() + begin, oldWavefront.begin() + begin, (end-begin)*sizeof(int) );
	memcpy( new2Old_1.begin() + begin, new2Old_2.begin() + begin, (end-begin)*sizeof(int) );
      } else {
	// else copy the corresponding entries from oldPerm and oldWavefront into the perm and wavefront
	// Then save the result in oldPerm and oldWavefront
	numberVerticesKeptTimer += case1Overhead;
	numberVerticesRejectedTimer += case2Overhead;
	setClass(1);
      } // end if
    } // end switch
  } // end else if (weightingScheme == DETERMINED_BY_CLASS)
  orderComponentTimer.stop();
  return true;
}



void 
SloanEngine::numberVertices( const int* vtxList, const int nVtxs) {
  //
  // 1. compute the initial priority for all visited vertices.
  //
  computeInitialPriority(vtxList, nVtxs);

  //
  // 2. Initialize Heap and status[]
  //
  {for (int i=0; i<nVtxs; i++ ) {
    status[ vtxList[i] ] = INACTIVE;
  }}
  for( vector<int>::const_iterator cur=startVtxs.begin(), stop=startVtxs.end(); cur != stop; ++cur ) {
    if ( status[ *cur ] == INACTIVE ) {
      heap->insert( initial_priority[ *cur ] , *cur );
      status[ *cur ] = PREACTIVE; 
    }
  }

  //
  // Special case, if we want to force the start nodes in the wavefront
  //
  if( makeStartVtxsInWavefront ) { 
    if ( VertexWeight == 0 ) { 
      for( vector<int>::const_iterator cur=startVtxs.begin(), stop=startVtxs.end(); cur != stop; ++cur ) {
	int i = *cur;
	if ( (status[ i ] != NUMBERED) && (status[ i ] != ACTIVE ) ) {
	  status[ i ] = ACTIVE; 
	  int weight_i = LocalWeight;
	  heap->incrementPriority( weight_i, i );
	  secondOrderNeighbors( weight_i, i );
	}
      }
    } else { // if VertexWeight != 0 
      for( vector<int>::const_iterator cur=startVtxs.begin(), stop=startVtxs.end(); cur != stop; ++cur ) {
	int i = *cur;
	if ( ( status[ i ] != NUMBERED ) && ( status[ i ] != ACTIVE ) ) {
	  status[ i ] = ACTIVE; 
	  int weight_i = LocalWeight * vtxWeight[i]; // here's the only difference
	  heap->incrementPriority( weight_i, i );
	  secondOrderNeighbors( weight_i, i );
	}
      }
    }
  }

  //
  // 3. Now do the ordering
  // 
  if (VertexWeight == 0) {
    unweightedOrdering(nVtxs);
  } else {
    weightedOrdering(nVtxs);
  }

}

void 
SloanEngine::computeInitialPriority( const int vtxList[], const int nVtxs ) {
  int vtx;
  const int * dist;
  if ( alternateGlobalPriority != 0 ) {
    dist = alternateGlobalPriority;
  } else { 
    dist = bfs->getDistance().lend();
  }

  if ((RefinementWeight == 0) && (VertexWeight == 0)) {
    //
    // This loop is the uncompressed Sloan ordering.
    //
    {for( int i = 0; i<nVtxs; i++) {
      vtx = vtxList[i];
      initial_priority[vtx] = ( (GlobalWeight * dist[vtx])
	                      - (LocalWeight  * g->deg(vtx)) );
    }}
  } else if ((RefinementWeight == 0) && (VertexWeight != 0)) { 
    //
    // This loop is the weighted Sloan ordering.
    // Duff, Reid, and Scott used this on their ``supernode'' (ie compressed) graphs
    //
    const int * vwgt = vtxWeight.lend();
    {for( int i = 0; i<nVtxs; i++) {
      vtx = vtxList[i];
      initial_priority[vtx] = ( (GlobalWeight * dist[vtx])
	                      - (LocalWeight  * g->deg(vtx))
	                      + (VertexWeight * vwgt[vtx]) );
    }}
  } else if ((RefinementWeight != 0) && (VertexWeight == 0)) { 
    if ( explicitOrder == 0 ) { 
      //
      // This loop is uncompressed Sloan refinement
      // it refines the implicit ordering of the input matrix
      //
      {for( int i = 0; i<nVtxs; i++) {
	vtx = vtxList[i];
	initial_priority[vtx] = ( (GlobalWeight     * dist[vtx])
	                        - (LocalWeight      * g->deg(vtx))
	                        - (RefinementWeight * vtx) );
      }} 
    } else { // if ( explicitOrder.nonNull() )
      //
      // This loop is uncompressed Sloan refinement
      // It refines the explicit permutation given by ``initial_ordering''
      //
      const int* inputOld2New = explicitOrder->getOld2New().lend();
      {for( int i = 0; i<nVtxs; i++) {
	vtx = vtxList[i];
	initial_priority[vtx] = ( (GlobalWeight     * dist[vtx])
	                        - (LocalWeight      * g->deg(vtx))
			        - (RefinementWeight * inputOld2New[vtx]) );
      }}
    }
  } else if ((RefinementWeight != 0) && (VertexWeight != 0)) { 
    if ( explicitOrder == 0 ) { // 
      //
      // This loop is compressed Sloan refinement
      // it refines the implicit ordering of the input matrix
      //
      const int * vwgt = vtxWeight.lend();
      {for( int i = 0; i<nVtxs; i++) {
	vtx = vtxList[i];
	initial_priority[vtx] = ( (GlobalWeight     * dist[vtx])
 	                        - (LocalWeight      * g->deg(vtx))
		                + (VertexWeight     * vwgt[vtx])
		                - (RefinementWeight * vtx) );
      }} 
    } else { // if ( explicitOrder.notNull() )
      //
      // This loop is compressed Sloan refinement
      // It refines the explicit permutation given by ``initial_ordering''
      //
      const int* inputOld2New = explicitOrder->getOld2New().lend();
      const int* vwgt = vtxWeight.lend();
      {for( int i = 0; i<nVtxs; i++) {
	vtx = vtxList[i];
	initial_priority[vtx] = ( (GlobalWeight     * dist[vtx])
	                        - (LocalWeight      * g->deg(vtx))
			        + (VertexWeight     * vwgt[vtx])
			        - (RefinementWeight * inputOld2New[vtx]) );
      }}
    }
  }
  if ( forceEndVtxsLast ) {
    // find minimum initial_priority of all end vtxs.
    int min = LARGE_INT;
    {for( vector<int>::const_iterator cur=endVtxs.begin(), stop=endVtxs.end();
	 cur != stop; ++cur ) {
      min = ( initial_priority[ *cur ] < min ) ? initial_priority[ *cur ] : min ;
    }}
    // delta = MIN_INT - min
    int delta = (min < 0 ) ? SMALL_INT - min : SMALL_INT;
    // reduce the initial_priority of all end vtxs by delta.
    {for( vector<int>::const_iterator cur=endVtxs.begin(), stop=endVtxs.end();
	  cur != stop; ++cur ) {
      if ( initial_priority[ *cur ] >= min ) { // keep from changing duplicates 
	initial_priority[ *cur ] += delta;
      }
    }}    
  }
}

void 
SloanEngine::unweightedOrdering(const int nVtxs) {
  int nActive= (makeStartVtxsInWavefront) ? startVtxs.size() : 0;
  int * new2old = new2Old_1.begin();
  nodeStatus * l_status = status.begin();
  int * l_initial_priority = initial_priority.begin();
  
  // 5. test for termination
  for( int k=0; (k<nVtxs) && (heap->size()>0) ; k++) {
    
    // 6. Find highest priority node in heap
    int i = heap->extractMax(); 
    ////cdbg << "Selected Vertex=" << i << ", iteration=" << k << ", nVtxsNumbered=" << nVtxsNumbered << endl;
    
    // 7. Label vertex
    new2old[nVtxsNumbered] = i;
    int status_i = l_status[i];
    l_status[i] = NUMBERED;

    // 8. build an adjacency list of vertex 
    // g->begin_adj(i), g->end_adj(i);

    // 9. for all j \in \adj(i) do 
    if (status_i == ACTIVE) {
      nActive--;
      for(Graph::const_iterator cur=g->begin_adj(i), stop=g->end_adj(i); cur != stop; ++cur) {
	int j = *cur;
	////cdbg << "\tExamining neighbor " << j << "...";
	switch( l_status[j] ) {
	case 0: // status_i==ACTIVE, l_status[j]==INACTIVE
	  // NOTE: This case doesn't occur in regular Sloan
	  int weight_j;
	  l_status[j] = ACTIVE; nActive++;
	  weight_j = l_initial_priority[j] + 2 * LocalWeight;
	  ////cdbg << "Case ACTIVE/INACTIVE:  node=" << j << " added with weight=" << weight_j << endl;
	  heap->insert(weight_j, j);
	  break;
	case 1: // status_i==ACTIVE, l_status[j]==PREACTIVE
	  l_status[j] = ACTIVE; nActive++;
	  weight_j = LocalWeight ;
	  ////cdbg << "Case ACTIVE/PREACTIVE:  node=" << j << " incremented by weight=" << weight_j << endl;
	  heap->incrementPriority( weight_j, j );
	  secondOrderNeighbors( weight_j, j );
	  break;
	case 2: // status_i==ACTIVE, l_status[j]==ACTIVE
	  ////cdbg << "Case ACTIVE/ACTIVE:" << endl;
	  break;
	case 3: // status_i==ACTIVE, l_status[j]==NUMBERED
	  ////cdbg << "Case ACTIVE/NUMBERED:" << endl;
	  break;
	default:
	  ////cdbg << "Unexpected Case " << l_status[j] << "fell through switch statement." << endl;
	  break;
	} // end switch
      } // end adj_i loop
    } else if (status_i == PREACTIVE) {
      for(Graph::const_iterator cur=g->begin_adj(i), stop=g->end_adj(i); cur != stop; ++cur) {
	int j = *cur;
	////cdbg << "\tExamining neighbor " << j << "...";
	switch( l_status[j] ) {
	case 0: // status_i==PREACTIVE, l_status[j]==INACTIVE
	  // NOTE: This case doesn't occur in regular Sloan
	  int weight_j;
	  l_status[j] = ACTIVE; nActive++;
	  weight_j = l_initial_priority[j] + 2 * LocalWeight ;
	  ////cdbg << "Case PREACTIVE/INACTIVE:  node=" << j << " added with weight=" << weight_j << endl;
	  heap->insert(weight_j, j);
	  weight_j = LocalWeight;
	  secondOrderNeighbors( weight_j, j );
	  break;
	case 1: // status_i==PREACTIVE, l_status[j]==PREACTIVE
	  l_status[j] = ACTIVE; nActive++;
	  weight_j = 2 * LocalWeight ;
	  ////cdbg << "Case PREACTIVE/PREACTIVE:  node=" << j << " incremented by weight=" << weight_j << endl;
	  heap->incrementPriority( weight_j, j );
	  weight_j = LocalWeight;
	  secondOrderNeighbors( weight_j, j );
	  break;
	case 2: // status_i==PREACTIVE, l_status[j]==ACTIVE
	  weight_j = LocalWeight ;
	  ////cdbg << "Case PREACTIVE/ACTIVE:  node=" << j << " incremented by weight=" << weight_j << endl;
	  heap->incrementPriority( weight_j, j );
	  break;
	case 3: // status_i==PREACTIVE, l_status[j]==NUMBERED
	  ////cdbg << "Case PREACTIVE/NUMBERED:" << endl;
	  break;
	default:
	  ////cdbg << "Unexpected Case " << l_status[j] << "fell through switch statement." << endl;
	  break;
	} // end switch
      } // end for adj_i
    } else { // status_i is neither PREACTIVE nor ACTIVE
      ////cdbg << "Unexpected case " << status_i << " for selected node" << endl;
    }
  wavefront[nVtxsNumbered++] = nActive+1;
  } // end k-loop
}



void SloanEngine::weightedOrdering(const int nVtxs) {
  int nActive= (makeStartVtxsInWavefront) ? startVtxs.size() : 0;
  int* new2old = new2Old_1.begin();
  nodeStatus * l_status = status.begin();
  int * l_initial_priority = initial_priority.begin();
  const int * vwgt = vtxWeight.lend();

  // 5. test for termination
  for( int k=0; (k<nVtxs) && (heap->size()>0) ; k++) {
    
    // 6. Find highest priority node in heap
    int i = heap->extractMax(); 
    ////cdbg << "Selected Vertex=" << i << ", iteration=" << k << ", nVtxsNumbered=" << nVtxsNumbered << endl;
    
    // 7. Label vertex
    new2old[nVtxsNumbered] = i;
    int status_i = l_status[i];
    l_status[i] = NUMBERED;

    // 8. build an adjacency list of vertex 
    // g->begin_adj(i), g->end_adj(i);

    // 9. for all j \in \adj(i) do 
    if (status_i == ACTIVE) {
      nActive--;
      for(Graph::const_iterator cur=g->begin_adj(i), stop=g->end_adj(i); cur != stop; ++cur) {
	int j = *cur;
	////cdbg << "\tExamining neighbor " << j << "...";
	switch( l_status[j] ) {
	case 0: // status_i==ACTIVE, l_status[j]==INACTIVE
	  // NOTE: This case doesn't occur in regular Sloan
	  int weight_j;
	  l_status[j] = ACTIVE; nActive++;
	  weight_j = l_initial_priority[j] + LocalWeight * ( vwgt[i] + vwgt[j] );
	  ////cdbg << "Case ACTIVE/INACTIVE:  node=" << j << " added with weight=" << weight_j << endl;
	  heap->insert(weight_j, j);
	  break;
	case 1: // status_i==ACTIVE, l_status[j]==PREACTIVE
	  l_status[j] = ACTIVE; nActive++;
	  weight_j = LocalWeight * vwgt[j];
	  ////cdbg << "Case ACTIVE/PREACTIVE:  node=" << j << " incremented by weight=" << weight_j << endl;
	  heap->incrementPriority( weight_j, j );
	  secondOrderNeighbors( weight_j, j );
	  break;
	case 2: // status_i==ACTIVE, l_status[j]==ACTIVE
	  ////cdbg << "Case ACTIVE/ACTIVE:" << endl;
	  break;
	case 3: // status_i==ACTIVE, l_status[j]==NUMBERED
	  ////cdbg << "Case ACTIVE/NUMBERED:" << endl;
	  break;
	default:
	  ////cdg << "Unexpected Case " << l_status[j] << "fell through switch statement." << endl;
	  break;
	} // end switch
      } // end adj_i loop
    } else if (status_i == PREACTIVE) {
      for(Graph::const_iterator cur=g->begin_adj(i), stop=g->end_adj(i); cur != stop; ++cur) {
	int j = *cur;
	////cdbg << "\tExamining neighbor " << j << "...";
	switch( l_status[j] ) {
	case 0: // status_i==PREACTIVE, l_status[j]==INACTIVE
	  int weight_j;
	  l_status[j] = ACTIVE; nActive++;
	  weight_j = l_initial_priority[j] + LocalWeight * ( vwgt[i] + vwgt[j] );
	  ////cdbg << "Case PREACTIVE/INACTIVE:  node=" << j << " added with weight=" << weight_j << endl;
	  heap->insert(weight_j, j);
	  weight_j = LocalWeight * vwgt[j];
	  secondOrderNeighbors( weight_j, j );
	  break;
	case 1: // status_i==PREACTIVE, l_status[j]==PREACTIVE
	  l_status[j] = ACTIVE; nActive++;
	  weight_j = LocalWeight * ( vwgt[i] + vwgt[j] );
	  ////cdbg << "Case PREACTIVE/PREACTIVE:  node=" << j << " incremented by weight=" << weight_j << endl;
	  heap->incrementPriority( weight_j, j );
	  weight_j = LocalWeight * vwgt[j];
	  secondOrderNeighbors( weight_j, j );
	  break;
	case 2: // status_i==PREACTIVE, l_status[j]==ACTIVE
	  weight_j = LocalWeight * vwgt[j];
	  ////cdbg << "Case PREACTIVE/ACTIVE:  node=" << j << " incremented by weight=" << weight_j << endl;
	  heap->incrementPriority( weight_j, j );
	  break;
	case 3: // status_i==PREACTIVE, l_status[j]==NUMBERED
	  ////cdbg << "Case PREACTIVE/NUMBERED:" << endl;
	  break;
	default:
	  ////cdbg << "Unexpected Case " << l_status[j] << "fell through switch statement." << endl;
	  break;
	} // end switch
      } // end for adj_i
    } else { // status_i is neither PREACTIVE nor ACTIVE
      ////cdbg << "Unexpected case " << status_i << " for selected node" << endl;
    }
  wavefront[nVtxsNumbered++] = nActive+1;
  } // end k-loop
}



void SloanEngine::secondOrderNeighbors( const int weight, const int vtx ) {
  nodeStatus * l_status = status.begin();
  int * l_initial_priority = initial_priority.begin();
  for(Graph::const_iterator cur=g->begin_adj(vtx), stop=g->end_adj(vtx); cur != stop; ++cur ) {
    int k = *cur;
    if (k == vtx) continue;
    ////cdbg << "\t\tExamining second order neighbor " << k << " ...";
    switch ( l_status[k] ) {
    case 0: // INACTIVE
      int weight_k;
      l_status[k] = PREACTIVE;
      weight_k = l_initial_priority[k] + weight;
      ////cdbg << " INACTIVE: adding to heap with weight=" << weight_k << ". " << endl;
      heap->insert( weight_k, k );
      break;
    case 1: // PREACTIVE
      weight_k = weight;
      ////cdbg << " PREACTIVE: incrementing weight by " << weight_k << ". " << endl;
      heap->incrementPriority( weight_k, k );
      break;
    case 2: // ACTIVE
      weight_k = weight;
      ////cdbg << " ACTIVE: incrementing weight by " << weight_k << ". " << endl;
      heap->incrementPriority( weight_k, k );
      break;
    case 3: // NUMBERED
      ////cdbg << " NUMBERED: do nothing. " << endl;
      break;
    default:
      cerr << "Unexpected Case fell through" << endl;
      break;
    } // end switch
  } // end for k = adj(vtx) loop
}

SPINDLE_IMPLEMENT_DYNAMIC( SloanEngine, SpindleAlgorithm )
