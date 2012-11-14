//
// PseudoDiameter.cc
//
// $Id: PseudoDiameter.cc,v 1.2 2000/02/18 01:32:03 kumfert Exp $
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

#include "spindle/PseudoDiameter.h"

#ifdef REQUIRE_OLD_CXX_HEADER_SUFFIX
#include <algo.h>
#else
#include <algorithm>
using namespace std;
#endif

#ifndef SPINDLE_FUNC_H_
#include "spindle/spindle_func.h"
#endif

#ifndef SPINDLE_GRAPH_H_
#include "spindle/Graph.h"
#endif

#ifndef SPINDLE_BFS_H_
#include "spindle/BreadthFirstSearch.h"
#endif

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

static char runError[] = "Execution error detected.";
static char invalidGraph[] = "Invalid graph detected.";
static char invalidState[] = "Invalid state detected.";

SPINDLE_IMPLEMENT_DYNAMIC( PseudoDiameter, SpindleAlgorithm )

PseudoDiameter::PseudoDiameter() {
  g = 0;
  shrinkingStrategy = 0;
  forwardBFS = 0;
  reverseBFS = 0;
  reset();
}

PseudoDiameter::PseudoDiameter( const Graph* graph ) {
  g = 0;
  shrinkingStrategy = 0;
  forwardBFS = 0;
  reverseBFS = 0;
  reset();
  setGraph( graph );
}

PseudoDiameter::~PseudoDiameter() {
  if ( forwardBFS != 0 ) {
    delete forwardBFS;
  }
  if ( reverseBFS != 0 ) {
    delete reverseBFS;
  }
  if ( shrinkingStrategy != 0 ) {
    delete shrinkingStrategy;
  }
}

bool  
PseudoDiameter::reset(){
  errMsg = 0;
  algorithmicState = EMPTY;
  g = 0;
  nVtxs = 0;
  root = -1;
  startVtx = -1;
  endVtx = -1;
  diameter = 0;
  nVisited = 0;
  nShortCircuits = 0;
  nBFS = 0;
  minMaxWidth = 0;
  revMaxWidth = 0;
  nFlips = 0;

  partition = 0;
  curPartition = -1;
  partitionMembership = strict;

  if ( shrinkingStrategy == 0 ) {
    ReidScottShrinkingStrategy *reidScott = new ReidScottShrinkingStrategy();
    reidScott->setNVtxsMax( 5 );
    shrinkingStrategy =  reidScott;
  }
  if ( forwardBFS == 0 ) {
    forwardBFS = new BreadthFirstSearch();
  } else {
    forwardBFS->reset();
  }
  if ( reverseBFS == 0 ) {
    reverseBFS = new BreadthFirstSearch();
  } else {
    reverseBFS->reset();
  }
  timer.reset();
  return true;
}

bool
PseudoDiameter::setGraph( const Graph* graph ) {
  if ( algorithmicState == INVALID ) {
    return false;
  } else if ( graph->isValid() ) {
    g = graph;
    nVtxs = g->size();
    forwardBFS->setGraph( g );
    reverseBFS->setGraph( g );
    if ( algorithmicState == DONE ) {
      // if setting the graph after a run, 
      // the previous root becomes invalid
      root = -1;
    }
    algorithmicState = READY;
    return true;
  } else { 
    algorithmicState = INVALID;
    errMsg = invalidGraph;
    return false; 
  }
}

bool
PseudoDiameter::setRoot( const int new_root ) {
  switch ( algorithmicState ) {
  case INVALID:
    return false;
  case EMPTY:
    return false;
  case READY:
  case DONE:
    algorithmicState = READY;
    if (( new_root < -1 ) || ( new_root >= nVtxs )) {
      return false;
    } else {
      root = new_root;
      return true;
    }
  default:
    algorithmicState = INVALID;
    errMsg = invalidState;
    return false;
  }
}
    

bool
PseudoDiameter::setShrinkingStrategy( PseudoDiamShrinkingStrategy * shrink ) {
  if ( shrinkingStrategy == shrink ) {
    return false;
  }
  switch ( algorithmicState ) {
  case INVALID:
    return false;
  case EMPTY:
    break;
  case READY:
    break;
  case DONE:
    algorithmicState = READY;
    break;
  default:
    algorithmicState = INVALID;
    errMsg = invalidState;
    return false;
  }
  if ( shrinkingStrategy != 0 ) {
    delete shrinkingStrategy;
  }
  shrinkingStrategy = shrink;
  return true; 
} 
  

bool 
PseudoDiameter::enablePartitionRestrictions( const int partitionMap[], PartitionMembership membership ) {
  if ( algorithmicState == INVALID ) { return false; }
  if ((partition != 0) || (partitionMap == 0))  { return false; }
  partition = partitionMap;
  partitionMembership = membership; 
  if ( partitionMembership == strict ) { 
    forwardBFS->enablePartitionRestrictions( partitionMap, BreadthFirstSearch::strict );
    reverseBFS->enablePartitionRestrictions( partitionMap, BreadthFirstSearch::strict );
  } else { 
    forwardBFS->enablePartitionRestrictions( partitionMap, BreadthFirstSearch::loose );
    reverseBFS->enablePartitionRestrictions( partitionMap, BreadthFirstSearch::loose );
  }
  return true; 
}

bool 
PseudoDiameter::disablePartitionRestrictions( ) {
  if ( algorithmicState == INVALID ) { return false; }
  if (partition == 0) { return false;  } // partition restrictions already disabled
  partition = 0; // NOTE:  PseudoDiameter does not own partition vector
  forwardBFS->disablePartitionRestrictions();
  reverseBFS->disablePartitionRestrictions();
  return true;
}

bool 
PseudoDiameter::setCurrentPartition( const int i ) { 
  if ( partition == 0 ) { 
    return false; 
  } else { 
    curPartition = i;
    return true;
  }
}

bool PseudoDiameter::execute() {
  if ( algorithmicState != READY ) { return false; }
  timer.start();
  startVtx = -1;
  endVtx = -1;
  diameter = -1;
  nVisited = 0;
  nShortCircuits = 0;
  nBFS = 0;

  // if no root is specified, find a vertex of minimum degree
  if ( root == -1 ) {
    if ( partition == 0 ) {  // if no partition restrictions, try each node
      int min_idx = 0;
      int min_deg = g->deg(min_idx);
      for (int i=1; i<nVtxs; i++) {
	if ( g->deg(i) < min_deg ) {
	  min_idx = i;
	  min_deg = g->deg(min_idx);
	}
      }
      root = min_idx;
    } else { // even if partitionMembership is loose, the root must be in the partition
      int min_idx = -1;
      int min_deg = INT_MAX;
      for (int i=1; i<nVtxs; i++) {
	if ( (partition[i] == curPartition)  && (g->deg(i) < min_deg ) ){
	  min_idx = i;
	  min_deg = g->deg(min_idx);
	}
      }
      if ( min_idx == -1 ) { 
	errMsg = "cannot find root in current partition" ;
	algorithmicState = INVALID;
	return false;
      }
      root = min_idx;
    }
  } else { // if root is specified
    if ( partition != 0 ) { 
      curPartition = partition[ root ];
    }
  }
  bool result = run();

  if ( result == true ) {
    algorithmicState = DONE;
  } else if ( errMsg == 0 ) {
    errMsg = runError;
    algorithmicState = INVALID;
  }
  timer.stop();
  return result;
}

// to compute a pseudo diameter
bool PseudoDiameter::run() {
  bool chk;
  bool justswapped = false;
  int minWidth;
  vector<int> workvec;
  
  //
  // 1. initialization
  //
  startVtx = root;
  candidates.reserve( nVtxs );
  endVtx = -1;  // NOTE:  endVtx = -1 indicates that an adequate endVtx is not found
  if(g->deg(root) == 0) { // if root vertex is not connected to anything.
    endVtx = root; 
    nVisited = 1;
    return true;
  }

  //
  // 2. Do the main loop while
  //
  do { 

    //
    // 3. do the forward breadth first search from start node
    //
    if ( justswapped == false ) {
      chk = forwardBFS->setRoot( startVtx ); if ( chk == false ) { return false; }
      chk = forwardBFS->execute();           if ( chk == false ) { return false; }
      nBFS++;
    } else { // just set old backwardBFS to forwardBFS... can skip this recomputation
      justswapped = false;
    }
    diameter = forwardBFS->queryHeight();

    //
    // 3.a do some inter-loop initialization... 
    //
    minWidth = nVtxs;
    int maxHeight = forwardBFS->queryHeight();
    candidates.resize(0);

    //
    // 4. extract the vertices farthest away from startVtx
    //    NOTE: for more explanation about steps 4, 5, & 6 check out
    //          $SPINDLE_HOME/src/util/drivers/examples/ex2.cc  
    //
    const int *new2old = forwardBFS->getNew2Old().lend();
    const int *vtx_end = new2old + forwardBFS->queryNVisited();
    const int *vtx_begin = vtx_end - forwardBFS->queryNFarthest();

    //
    // 5. If using loose partition restrictions, 
    //    we allow the BFS's to include vertices that are
    //    adjacent to the ones strictly in the partition, we do not
    //    want to allow them in our list of candidates
    if ( ( partition != 0 ) && ( partitionMembership == loose ) ) { 
      workvec.resize(0); // resize workvec.
      for ( const int *cur = vtx_begin; cur != vtx_end; ++cur ) { // for all vtxs in last level
	if ( partition[ *cur ] == curPartition ) {  // if the vertex is strictly in the partition.
	  workvec.push_back( *cur );                // add it to the work vector
	}
      } // end for

      if ( workvec.size() == 0 ) {       // if workvec is empty, 
	// all vtxs on last level were not strict members of the partition, 
	// This means we have to go one level back.
	// we are guaranteed that there will be some vertices that are strict members
	// in this level since loose partition restrictions require adjacency to a
	// strict member
	vtx_end = vtx_begin;  // set vtx_end to the end of vtx_begin;
	const int * level = ((const BreadthFirstSearch *) forwardBFS)->getDistance().lend();
	vtx_begin = vtx_end - 1;
	int nextToLastLevel = level[ *vtx_begin ];
	while ( level [ *vtx_begin ] == nextToLastLevel ) { 
	  --vtx_begin; 
	}
	++vtx_begin;
	// now search for nodes
	for ( const int *cur = vtx_begin; cur != vtx_end; ++cur ) { // for all vtxs in last level
	  if ( partition[ *cur ] == curPartition ) {  // if the vertex is strictly in the partition.
	    workvec.push_back( *cur );                // add it to the work vector
	  }
	}
      } // end if workvec.size() == 0 on first try
      vtx_begin = workvec.begin();
      vtx_end   = workvec.end();
    } // end if ( partition != 0 && partitionMembership == loose
	
    // now shrink the list of strict members to the partition to a smaller list of candidates
    shrinkingStrategy->shrink( vtx_begin, vtx_end, g, candidates );  // loads candidates

    //
    // 6. For each candidate vertex in ``candidates'' do a backward BFS
    //
    for(vector<int>::const_iterator start=candidates.begin(), stop=candidates.end();
	start != stop; ++start) {
      int candidate = *start;
      if (endVtx == -1) { 
	reverseBFS->disableShortCircuiting();
      } else {
	reverseBFS->enableShortCircuiting(minWidth); 
      }
      chk = reverseBFS->setRoot(candidate); if (chk==false) { return false; }
      chk = reverseBFS->execute();          if (chk==false) { return false; }
      nBFS++;
      
      // 6.a.  if backward bfs short circuited, continue
      if (reverseBFS->hasShortCircuited()) {
	nShortCircuits++;
	continue;
      }
      // 6.b.  else if bfs is higher and narrower, make candidate the new start
      if ((reverseBFS->queryHeight() > maxHeight)  && (reverseBFS->queryWidth() < minWidth) ) {
	startVtx = candidate;
	endVtx = -1;
	// now swap the BFS objects
	BreadthFirstSearch * temp = reverseBFS;
	reverseBFS = forwardBFS;
	forwardBFS = temp;
	forwardBFS->disableShortCircuiting();
	// and set the flag to skip recomputing the forwardBFS;
	justswapped = true;
	nFlips++;
	break;
      }
      // 6.c   else if bfs is narrower than any previous bfs, make candidate the end
      if (reverseBFS->queryWidth() < minWidth) { 
	endVtx = candidate;
	minWidth = reverseBFS->queryWidth();
      }
    } // end forall candidates in shrunk_list
  } while ( endVtx == -1 );
  nVisited = forwardBFS->queryNVisited();

  // We now have two pseudo-peripheral nodes.  Is one naturally a startVtx
  // and the other naturally an endVtx???
  // Reid and Scott say do both and take startVtx as the one with the 
  // narrowest band
  if ( minWidth < forwardBFS->queryWidth() ) {
    // swap one more time.
    int tempi = startVtx;
    startVtx = endVtx;
    endVtx = tempi;
    minMaxWidth = minWidth;
    revMaxWidth = forwardBFS->queryWidth();
    ++nFlips;
    BreadthFirstSearch * tempbfs = forwardBFS;
    forwardBFS = reverseBFS;
    reverseBFS = tempbfs;
  } else {
    minMaxWidth = forwardBFS->queryWidth();
    revMaxWidth = minWidth;
  }
  return true;
}

void
PseudoDiamShrinkingStrategy::pack_large_set( const int * src_begin, const int * src_end, 
					     const Graph *g ) {
  // make sure there is enough space
  large_set.reserve( src_end - src_begin );
  small_set.reserve( src_end - src_begin );

  // make sets empty
  large_set.resize(0);
  small_set.resize(0);

  // pack the large set
  for( const int * cur = src_begin;  cur <  src_end; ++cur ) {
    int cur_vtx = *cur;
    large_set.push_back( make_pair( g->deg(cur_vtx), cur_vtx) ); 
  }
  
  // sort the set by vertex degree
  sort(large_set.begin(), large_set.end(), pair_first_less<int,int>() );
}

bool
SloanShrinkingStrategy::shrink( const int * src_begin, const int * src_end, 
				const Graph * g, vector< int >& dest ) {
  pack_large_set( src_begin, src_end, g );
  int sz = large_set.size();
  dest.resize(0);
  for(vector<pair<int,int> >::const_iterator start=large_set.begin(), end = (large_set.begin()+((sz+1)/2));
      start != end; ++start ) {
    dest.push_back( (*start).second );
  }
  if ( dest.size() == 0 ) {
    // warning!!!
    if ( large_set.size() == 0 ) {
      dest.push_back( *src_begin );
    } else {
      dest.push_back( (*(large_set.begin())).second );
    }
  }
  return true;
}

bool
DuffReidScottShrinkingStrategy::shrink( const int * src_begin, const int * src_end, 
					const Graph * g, vector< int >& dest ) {
  pack_large_set( src_begin, src_end, g );
  back_insert_iterator< vector< pair< int, int > > >  insert_small_set(small_set);
  unique_copy( large_set.begin(), large_set.end(), insert_small_set , pair_first_equal<int,int>() );
  dest.resize(0);
  for(vector<pair<int,int> >::const_iterator start=small_set.begin(), end = small_set.end();
      start != end; ++start ) {
    dest.push_back( (*start).second );
  }
  if ( dest.size() == 0 ) {
    // warning!!!
    if ( large_set.size() == 0 ) {
      dest.push_back( *src_begin );
    } else {
      dest.push_back( (*(large_set.begin())).second );
    }
  }
  return true;
}

ReidScottShrinkingStrategy::ReidScottShrinkingStrategy() {
  nVtxsMax = 5;
}

ReidScottShrinkingStrategy::ReidScottShrinkingStrategy( const int NVtxsMax ) {
  if ( NVtxsMax <= 0 ) {
    nVtxsMax = 5;
  } else {
    nVtxsMax = NVtxsMax;
  }
}

bool
ReidScottShrinkingStrategy::setNVtxsMax( const int NVtxsMax ) {
  if ( NVtxsMax <= 0 ) {
    return false;
  } else {
    nVtxsMax = NVtxsMax;
    return true;
  }
}

int
ReidScottShrinkingStrategy::getNVtxsMax() const {
  return nVtxsMax;
}

bool
ReidScottShrinkingStrategy::shrink( const int * src_begin, const int * src_end, 
				    const Graph * g, vector< int >& dest ) {
  dest.resize(0);
  pack_large_set( src_begin, src_end, g );
  temp.resize(0);
  for( vector<pair<int,int> >::const_iterator cur=large_set.begin(), stop=large_set.end();
       cur != stop; ++cur ) {
    if ( find( temp.begin(), temp.end(), (*cur).second ) == temp.end() ) { 
      // if cur is not in any vertices yet visited
      dest.push_back( (*cur).second );
      if ((int)dest.size() >= nVtxsMax ) { return true; }
      temp.insert( temp.end(), g->begin_adj( (*cur).second ), g->end_adj( (*cur).second ) );
    }
  }
  if ( dest.size() == 0 ) {
    // warning!!!
    if ( large_set.size() == 0 ) {
      dest.push_back( *src_begin );
    } else {
      dest.push_back( (*(large_set.begin())).second );
    }
  }
  return true;
}
