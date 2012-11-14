//
// BreadthFirstSearch.cc
//
// $Id: BreadthFirstSearch.cc,v 1.2 2000/02/18 01:32:02 kumfert Exp $
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

//#include <limits.h>
using namespace std;

#include "spindle/BreadthFirstSearch.h"

static const int LARGE_INT = INT_MAX - 1;


#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

static char setGraphBeforeRoot[] = "Cannot set root(s) before setting valid graph.";
static char invalidGraph[] = "Invalid graph detected.";
static char invalidRoot[] = "Invalid  roots detected: out of range";
static char invalidState[] = "Invalid algorithmic state detected.";
static char rootsNotOnSamePartition[] = "Invalid roots detected: stradles subdomains.";
static char executionError[] = "Execution error detected.";
static SharedArray<int> null_array;

#ifdef __FUNC__
#undef __FUNC_
#endif

#define __FUNC__  "BreadhFirstSearch()::BreadthFirstSearch()"
BreadthFirstSearch::BreadthFirstSearch() {
  FENTER;
  stamp = LARGE_INT;
  FCALL reset();
  FEXIT;
}
#undef __FUNC__

#define __FUNC__ "BreadthFirstSearch::BreadthFirstSearch( const Graph* graph )"
BreadthFirstSearch::BreadthFirstSearch( const Graph* graph ) {
  FENTER;
  stamp = LARGE_INT;
  FCALL reset();
  FCALL setGraph( graph );
  FEXIT;
}
#undef __FUNC__

#define __FUNC__ "BreadthFirstSearch::~BreadthFirstSearch()"
BreadthFirstSearch::~BreadthFirstSearch() {
  FENTER;
  FEXIT;
}
#undef __FUNC__

#define __FUNC__ "bool BreadthFirstSearch::reset()"
bool 
BreadthFirstSearch::reset() {
  FENTER;
  g = 0;
  nVtxs = 0;
  errMsg = 0;

  partition = 0;
  curPartition = -1;
  partitionMembership = strict;

  nVisited = 0;
  nFarthest = 0;
  height = 0;
  width = 0;
  maxDegree = -1;
  minDegree = -1;
  maxDegreeVtx = -1;
  minDegreeVtx = -1;
  shortCircuited = false;
  timer.reset();

  algorithmicState = EMPTY;
  FCALL resetRoots();

  FRETURN( true );
}
#undef __FUNC__


#define __FUNC__ "bool BreadthFirstSearch::setGraph( const Graph* graph )"
bool
BreadthFirstSearch::setGraph( const Graph* graph ) {
  FENTER;
  if ( algorithmicState==INVALID ) {
    FRETURN( false );
  } else if ( graph->isValid() ) {
    g = graph;
    nVtxs = g->size();
    maxWidth = nVtxs;
    FCALL resetRoots();

    algorithmicState = EMPTY;
    FRETURN( true );
  } else {
    algorithmicState = INVALID;
    errMsg = invalidGraph;
    FRETURN( false );
  }
}
#undef __FUNC__

#define __FUNC__ "bool BreadthFirstSearch::resetRoots()"
bool
BreadthFirstSearch::resetRoots() {
  FENTER;
  switch( algorithmicState ) {
  case INVALID:
    FRETURN( false );
  case EMPTY:
    roots.resize(0);
    FRETURN( true );
  case READY:
    FCALL roots.resize(0);
    algorithmicState = EMPTY;
    FRETURN( true );
  case DONE:
    FCALL roots.resize(0);
    algorithmicState = EMPTY;
    FRETURN( true );
  default:
    algorithmicState = INVALID;
    errMsg = invalidState;
    FRETURN( false ); 
  }
}
#undef __FUNC__

#define __FUNC__ "bool BreadthFirstSearch::setRoot( const int root )"
bool
BreadthFirstSearch::setRoot( const int root ) {
  FENTER;
  if ( g == 0 ) { 
    algorithmicState = INVALID;
    errMsg = setGraphBeforeRoot;
    FRETURN(false);
  } else if ( (root < 0) || (root >= nVtxs) ) { 
    algorithmicState = INVALID;
    errMsg = invalidRoot;
    FRETURN( false );
  }
  switch( algorithmicState ) {
  case INVALID:
    // wrong.  must reset() once error has occured
    FRETURN( false );
    break; // not neccessary, but...
  case DONE:
    // fine.  It just ran successfully and we are
    //        running again from a different root.
    FCALL resetRoots();  // sets algorithmicState to EMPTY with old graph
    break;
  case READY:
    // fine.  It is ready to go, but we are adding an
    //        additional root.
  case EMPTY:
    // fine.  We have a graph and are specifying a root.
    break;
  default:
    // error
    algorithmicState = INVALID;
    errMsg = invalidState;
    FRETURN( false );
  }
  FCALL roots.push_back( root );
  algorithmicState = READY;
  FRETURN( true );
}
#undef __FUNC__


#define __FUNC__ "bool BreadthFirstSearch::setRoots( const int* root, const int nRoots )"
bool
BreadthFirstSearch::setRoots( const int* root, const int nRoots ) {
  FENTER;
  if ( g == 0 ) { 
    algorithmicState = INVALID;
    errMsg = setGraphBeforeRoot;
    FRETURN( false );
  } else if ( (root == 0) || (nRoots >= nVtxs) || (nRoots <= 0 ) ) { 
    algorithmicState = INVALID;
    errMsg = invalidRoot;
    FRETURN( false );
  }
  switch( algorithmicState ) {
  case INVALID:
    // wrong.  must reset() once error has occured
    FRETURN( false );
    break; // not neccessary, but...
  case DONE:
    // fine.  It just ran successfully and we are
    //        running again from a different root.
    resetRoots();  // sets algorithmicState to EMPTY with old graph
    break;
  case READY:
    // fine.  It is ready to go, but we are adding an
    //        additional root.
  case EMPTY:
    // fine.  We have a graph and are specifying a root.
    break;
  default:
    // error
    algorithmicState = INVALID;
    errMsg = invalidState;
    FRETURN( false );
  }
  for( int i=0; i<nRoots; ++i ) {
    if ( ( root[i] < 0 ) || ( root[i] >= nVtxs ) ) {
      algorithmicState = INVALID;
      errMsg = invalidRoot;
      FRETURN( false );
    }
    FCALL roots.push_back( root[i] );
  }
  algorithmicState = READY;
  FRETURN( true );
}
#undef __FUNC__

#define __FUNC__ "bool BreadthFirstSearch::execute()"
bool 
BreadthFirstSearch::execute() {
  FENTER;
  // 1. validate state
  if ( algorithmicState==INVALID || algorithmicState==DONE ) { 
    FRETURN( false ); 
  } 
  timer.start();

  // 2 Initialize for this run
  current_distance = 0;
  nVisited = 0;
  shortCircuited = false;
  cur_width = 0; 
  widest_yet = 0;
  // first handle the ``SharedArray<int> distance''
  {
    // if doesn't own current array, or size is wrong, 
    //    create and own array of correct size
    FCALL distance.resize( nVtxs );  
    // initialize everything to ``-1''
    FCALL distance.init( -1 );
  }

  // similarly for ``SharedArray<int> new2old''
  {
    // if doesn't own current array, or size is wrong, 
    //    create and own array of correct size
    FCALL new2old.resize( nVtxs );  
    // initialize everything to ``-1''
    //// Instead, just say everything after nVisited is undefined.
    ////    FCALL new2old.init( -1 );
  }
  
  // now do the same with the ``SharedArray<int> visited''
  {
    // if doesn't own current array, or size is wrong, 
    //    create and own array of correct size
    FCALL visited.resize( nVtxs );
    // now  increment the stamp
    if (stamp < LARGE_INT ) {
      stamp++;
    } else {
      stamp = 1;
      // initialize everything to false
      visited.init( 0 );
    }
  } // end initialize ``SharedArray<int> visited''
  

  // 3 load the root vtxs
  if ( partition == 0 ) {
    for( vector<int>::const_iterator it = roots.begin(), stop = roots.end(); it != stop; ++it ) {
      int new_root = *it;
      if ( visited[ new_root ] < stamp ) {  // keep from double visited duplicates
	distance[ new_root ] = current_distance;
	new2old[ nVisited++ ] = new_root;
	visited[ new_root ] = stamp;
	++cur_width;
	++widest_yet;
      }
    }
  } else {
    curPartition = partition[*(roots.begin())];
    for( vector<int>::const_iterator it = roots.begin(), stop = roots.end(); it != stop; ++it ) {
      if (partition[*it] != curPartition ) {
	algorithmicState = INVALID;
	errMsg = rootsNotOnSamePartition;
	FRETURN( false );
      }
      int new_root = *it;
      if ( visited[ new_root ] < stamp ) {
	distance[ new_root ] = current_distance;
	new2old[ nVisited++ ] = new_root;
	visited[ new_root ] = stamp;
	++cur_width;
	++widest_yet;
      }
    }
  }
  FCALL roots.resize(0); // don't need these anymore
  
  // 4. run
  bool result;
  if (partition==0) {
    result = run();
  } else if ( partitionMembership == strict ) {
    result = partitionedRun();
  } else { 
    result = loosePartitionedRun();
  }
  if (result) { 
    algorithmicState = DONE;
  } else if (errMsg == 0) {
    algorithmicState = INVALID;
    errMsg = executionError;
  }
  timer.stop();
  FRETURN( result );
}
#undef __FUNC__ 


#define __FUNC__ "bool BreadthFirstSearch::run()"
bool 
BreadthFirstSearch::run() {
  FENTER;
  int k=0;
  int * l_visited = visited.begin();
  int * l_distance = distance.begin();
  int * l_new2old = new2old.begin();
  int l_maxDegree = -1;
  int l_maxDegreeVtx = -1;
  int l_minDegree = g->queryNVtxs()+1;
  int l_minDegreeVtx = -1;
  const int * adjHead = g->getAdjHead().lend();
  const int * vtxWeight = 0;
  
  if (g->getVtxWeight().size() == g->size() ) {
    vtxWeight = g->getVtxWeight().lend();
    while( k < nVisited ) {
      current_distance++; // current distance is one more
      int prev_width = cur_width;
      int prev_nVisited = nVisited;
      for( int j=0; j<prev_width; j++) { // foreach vertex at distance(current_distance-1)
	int i = l_new2old[k++];            // increment k here.
	int curDegree = 0;
	for(const int * cur=g->begin_adj(i), *stop=g->end_adj(i);
	    cur != stop; ++cur ) { // for all vertices adjacent to i
	  int adj_i = *cur;
	  curDegree += vtxWeight[ adj_i ];
	  if (visited[adj_i]<stamp) {
	    // if not visited  and either no partition restrictions or adj_i is in cur partition
	    l_visited[adj_i] = stamp;
	    l_distance[adj_i] = current_distance;
	    l_new2old[nVisited++] = adj_i;
	  } // end if
	} // end for adj(i)
	if ( curDegree < l_minDegree ) {
	  l_minDegree = curDegree;
	  l_minDegreeVtx = i;
	}
	if ( curDegree > l_maxDegree ) {
	  l_maxDegree = curDegree;
	  l_maxDegreeVtx = i;
	}
      } // end foreach vertex at dist(current_distance-1)
      cur_width = nVisited - prev_nVisited;
      widest_yet = (cur_width > widest_yet) ? cur_width : widest_yet;
      
      if (cur_width > maxWidth) { 
	// short-circuit triggered
	shortCircuited = true;
	width = widest_yet;
	height = current_distance;
	nFarthest = cur_width;
	minDegree = l_minDegree;
	maxDegree = l_maxDegree;
	minDegreeVtx = l_minDegreeVtx;
	maxDegreeVtx = l_maxDegreeVtx;
	FRETURN( true );
      } 
      if (prev_nVisited == nVisited) { // no more to add
	// normal termination
	width = widest_yet;
	height = current_distance-1;
	nFarthest = prev_width;
	minDegree = l_minDegree;
	maxDegree = l_maxDegree;
	minDegreeVtx = l_minDegreeVtx;
	maxDegreeVtx = l_maxDegreeVtx;
	FRETURN( true );
      }
    } // end while k < nVisited
  } else { // no vtx weights
    while( k < nVisited ) {
      current_distance++; // current distance is one more
      int prev_width = cur_width;
      int prev_nVisited = nVisited;
      for( int j=0; j<prev_width; j++) { // foreach vertex at distance(current_distance-1)
	int i = l_new2old[k++];            // increment k here.
	int curDegree = adjHead[i+1] - adjHead[i];
	for(const int * cur=g->begin_adj(i), *stop=g->end_adj(i);
	    cur != stop; ++cur ) { // for all vertices adjacent to i
	  int adj_i = *cur;
	  if (visited[adj_i]<stamp) {
	    // if not visited  and either no partition restrictions or adj_i is in cur partition
	    l_visited[adj_i] = stamp;
	    l_distance[adj_i] = current_distance;
	    l_new2old[nVisited++] = adj_i;
	  } // end if
	} // end for adj(i)
	if ( curDegree < l_minDegree ) {
	  l_minDegree = curDegree;
	  l_minDegreeVtx = i;
	}
	if ( curDegree > l_maxDegree ) {
	  l_maxDegree = curDegree;
	  l_maxDegreeVtx = i;
	}
      } // end foreach vertex at dist(current_distance-1)
      cur_width = nVisited - prev_nVisited;
      widest_yet = (cur_width > widest_yet) ? cur_width : widest_yet;
      
      if (cur_width > maxWidth) { 
	// short-circuit triggered
	shortCircuited = true;
	width = widest_yet;
	height = current_distance;
	nFarthest = cur_width;
	minDegree = l_minDegree;
	maxDegree = l_maxDegree;
	minDegreeVtx = l_minDegreeVtx;
	maxDegreeVtx = l_maxDegreeVtx;
	FRETURN( true );
      } 
      if (prev_nVisited == nVisited) { // no more to add
	// normal termination
	width = widest_yet;
	height = current_distance-1;
	nFarthest = prev_width;
	minDegree = l_minDegree;
	maxDegree = l_maxDegree;
	minDegreeVtx = l_minDegreeVtx;
	maxDegreeVtx = l_maxDegreeVtx;
	FRETURN( true );
      }
    } // end while k < nVisited
  } // end else ( no vtxWeights )
  FRETURN( false );
}
#undef __FUNC__


#define __FUNC__ "bool BreadthFirstSearch::partitionedRun()"
bool 
BreadthFirstSearch::partitionedRun() {
  FENTER;
  int k=0;
  int * l_visited = visited.begin();
  int * l_distance = distance.begin();
  int * l_new2old = new2old.begin();
  int l_maxDegree = -1;
  int l_maxDegreeVtx = -1;
  int l_minDegree = g->queryNVtxs()+1;
  int l_minDegreeVtx = -1;
  const int * adjHead = g->getAdjHead().lend();
  const int * vtxWeight = 0;
  
  if ( g->getVtxWeight().size() == g->size() ) {
    while( k < nVisited ) {
      current_distance++; // current distance is one more
      int prev_width = cur_width;
      int prev_nVisited = nVisited;
      
      for( int j=0; j<prev_width; j++) { // foreach vertex at distance(current_distance-1)
	int i = l_new2old[k++];            // increment k here.
	int curDegree = 0;
	for(Graph::const_iterator cur=g->begin_adj(i), stop=g->end_adj(i);
	  cur != stop; ++cur ) { // for all vertices adjacent to i
	  int adj_i = *cur;
	  curDegree += vtxWeight[ adj_i ];
	  if ((l_visited[adj_i]<stamp) &&  (partition[adj_i] == curPartition) ) {
	    // if not visited  and either no partition restrictions or adj_i is in cur partition
	    l_visited[adj_i] = stamp;
	    l_distance[adj_i] = current_distance;
	    l_new2old[nVisited++] = adj_i;
	  } // end if
	} // end for adj(i)
	if ( curDegree < l_minDegree ) {
	  l_minDegree = curDegree;
	  l_minDegreeVtx = i;
	}
	if ( curDegree > l_maxDegree ) {
	  l_maxDegree = curDegree;
	  l_maxDegreeVtx = i;
	}
      } // end foreach vertex at dist(current_distance-1)
      cur_width = nVisited - prev_nVisited;
      widest_yet = (cur_width > widest_yet) ? cur_width : widest_yet;
      
      if (cur_width > maxWidth) { 
	// short-circuit triggered
	shortCircuited = true;
	width = widest_yet;
	height = current_distance;
	nFarthest = cur_width;
	minDegree = l_minDegree;
	maxDegree = l_maxDegree;
	minDegreeVtx = l_minDegreeVtx;
	maxDegreeVtx = l_maxDegreeVtx;
	FRETURN( true );
      } 
      if (prev_nVisited == nVisited) { // no more to add
	// normal termination
	width = widest_yet;
	height = current_distance-1;
	nFarthest = prev_width;
	minDegree = l_minDegree;
	maxDegree = l_maxDegree;
	minDegreeVtx = l_minDegreeVtx;
	maxDegreeVtx = l_maxDegreeVtx;
	FRETURN( true );
      }
    } // end while k < nVisited
  } else { 
    while( k < nVisited ) {
      current_distance++; // current distance is one more
      int prev_width = cur_width;
      int prev_nVisited = nVisited;
      
      for( int j=0; j<prev_width; j++) { // foreach vertex at distance(current_distance-1)
	int i = l_new2old[k++];            // increment k here.
	int curDegree = adjHead[i+1] - adjHead[i];
	for(Graph::const_iterator cur=g->begin_adj(i), stop=g->end_adj(i);
	    cur != stop; ++cur ) { // for all vertices adjacent to i
	  int adj_i = *cur;
	  if ((l_visited[adj_i]<stamp) &&  (partition[adj_i] == curPartition) ) {
	    // if not visited  and either no partition restrictions or adj_i is in cur partition
	    l_visited[adj_i] = stamp;
	    l_distance[adj_i] = current_distance;
	    l_new2old[nVisited++] = adj_i;
	  } // end if
	} // end for adj(i)
	if ( curDegree < l_minDegree ) {
	  l_minDegree = curDegree;
	  l_minDegreeVtx = i;
	}
	if ( curDegree > l_maxDegree ) {
	  l_maxDegree = curDegree;
	  l_maxDegreeVtx = i;
	}	
      } // end foreach vertex at dist(current_distance-1)
      cur_width = nVisited - prev_nVisited;
      widest_yet = (cur_width > widest_yet) ? cur_width : widest_yet;
      
      if (cur_width > maxWidth) { 
	// short-circuit triggered
	shortCircuited = true;
	width = widest_yet;
	height = current_distance;
	nFarthest = cur_width;
	minDegree = l_minDegree;
	maxDegree = l_maxDegree;
	minDegreeVtx = l_minDegreeVtx;
	maxDegreeVtx = l_maxDegreeVtx;
	FRETURN( true );
      } 
      if (prev_nVisited == nVisited) { // no more to add
	// normal termination
	width = widest_yet;
	height = current_distance-1;
	nFarthest = prev_width;
	minDegree = l_minDegree;
	maxDegree = l_maxDegree;
	minDegreeVtx = l_minDegreeVtx;
	maxDegreeVtx = l_maxDegreeVtx;
	FRETURN( true );
      }
    } // end while k < nVisited
  }
  FRETURN( false );
}
#undef __FUNC__

#define __FUNC__ "bool BreadthFirstSearch::loosePartitionedRun()"
bool 
BreadthFirstSearch::loosePartitionedRun() {
  FENTER;
  int k=0;
  int * l_visited = visited.begin();
  int * l_distance = distance.begin();
  int * l_new2old = new2old.begin();
  int l_maxDegree = -1;
  int l_maxDegreeVtx = -1;
  int l_minDegree = g->queryNVtxs()+1;
  int l_minDegreeVtx = -1;
  const int * adjHead = g->getAdjHead().lend();
  const int * vtxWeight = 0;
  
  if ( g->getVtxWeight().size() == g->size() ) {
    while( k < nVisited ) {
      current_distance++; // current distance is one more
      int prev_width = cur_width;
      int prev_nVisited = nVisited;
      
      for( int j=0; j<prev_width; j++) { // foreach vertex at distance(current_distance-1)
	int i = l_new2old[k++];            // increment k here.
	int curDegree = 0;
	bool i_in_partition = ( partition[ i ] == curPartition );
	for(Graph::const_iterator cur=g->begin_adj(i), stop=g->end_adj(i);
	  cur != stop; ++cur ) { // for all vertices adjacent to i
	  int adj_i = *cur;
	  curDegree += vtxWeight[ adj_i ];
	  if ((l_visited[adj_i]<stamp) && ( i_in_partition || (partition[adj_i] == curPartition) ) ) {
	    // if not visited  and either i or adj_i is in cur partition (or both)
	    l_visited[adj_i] = stamp;
	    l_distance[adj_i] = current_distance;
	    l_new2old[nVisited++] = adj_i;
	  } // end if
	} // end for adj(i)
	if ( curDegree < l_minDegree ) {
	  l_minDegree = curDegree;
	  l_minDegreeVtx = i;
	}
	if ( curDegree > l_maxDegree ) {
	  l_maxDegree = curDegree;
	  l_maxDegreeVtx = i;
	}
      } // end foreach vertex at dist(current_distance-1)
      cur_width = nVisited - prev_nVisited;
      widest_yet = (cur_width > widest_yet) ? cur_width : widest_yet;
      
      if (cur_width > maxWidth) { 
	// short-circuit triggered
	shortCircuited = true;
	width = widest_yet;
	height = current_distance;
	nFarthest = cur_width;
	minDegree = l_minDegree;
	maxDegree = l_maxDegree;
	minDegreeVtx = l_minDegreeVtx;
	maxDegreeVtx = l_maxDegreeVtx;
	FRETURN( true );
      } 
      if (prev_nVisited == nVisited) { // no more to add
	// normal termination
	width = widest_yet;
	height = current_distance-1;
	nFarthest = prev_width;
	minDegree = l_minDegree;
	maxDegree = l_maxDegree;
	minDegreeVtx = l_minDegreeVtx;
	maxDegreeVtx = l_maxDegreeVtx;
	FRETURN( true );
      }
    } // end while k < nVisited
  } else { 
    while( k < nVisited ) {
      current_distance++; // current distance is one more
      int prev_width = cur_width;
      int prev_nVisited = nVisited;
      
      for( int j=0; j<prev_width; j++) { // foreach vertex at distance(current_distance-1)
	int i = l_new2old[k++];            // increment k here.
	int curDegree = adjHead[i+1] - adjHead[i];
	bool i_in_partition = ( partition[ i ] == curPartition );
	for(Graph::const_iterator cur=g->begin_adj(i), stop=g->end_adj(i);
	    cur != stop; ++cur ) { // for all vertices adjacent to i
	  int adj_i = *cur;
	  if ((l_visited[adj_i]<stamp) && ( i_in_partition ||  (partition[adj_i] == curPartition) ) ) {
	    // if not visited  and either i or adj_i is in cur partition (or both)
	    l_visited[adj_i] = stamp;
	    l_distance[adj_i] = current_distance;
	    l_new2old[nVisited++] = adj_i;
	  } // end if
	} // end for adj(i)
	if ( curDegree < l_minDegree ) {
	  l_minDegree = curDegree;
	  l_minDegreeVtx = i;
	}
	if ( curDegree > l_maxDegree ) {
	  l_maxDegree = curDegree;
	  l_maxDegreeVtx = i;
	}	
      } // end foreach vertex at dist(current_distance-1)
      cur_width = nVisited - prev_nVisited;
      widest_yet = (cur_width > widest_yet) ? cur_width : widest_yet;
      
      if (cur_width > maxWidth) { 
	// short-circuit triggered
	shortCircuited = true;
	width = widest_yet;
	height = current_distance;
	nFarthest = cur_width;
	minDegree = l_minDegree;
	maxDegree = l_maxDegree;
	minDegreeVtx = l_minDegreeVtx;
	maxDegreeVtx = l_maxDegreeVtx;
	FRETURN( true );
      } 
      if (prev_nVisited == nVisited) { // no more to add
	// normal termination
	width = widest_yet;
	height = current_distance-1;
	nFarthest = prev_width;
	minDegree = l_minDegree;
	maxDegree = l_maxDegree;
	minDegreeVtx = l_minDegreeVtx;
	maxDegreeVtx = l_maxDegreeVtx;
	FRETURN( true );
      }
    } // end while k < nVisited
  }
  FRETURN( false );
}
#undef __FUNC__


#define __FUNC__ "bool BreadthFirstSearch::enableShortCircuiting( const int max_width )"
bool 
BreadthFirstSearch::enableShortCircuiting( const int max_width ) { 
  FENTER;
  if ( algorithmicState == INVALID ) { FRETURN( false ); }
  if ((max_width <= 0) || (max_width >= nVtxs)) { FRETURN( false ); }
  maxWidth = max_width;
  FRETURN( true );
}
#undef __FUNC__

#define __FUNC__ "bool BreadthFirstSearch::disableShortCircuiting()"
bool 
BreadthFirstSearch::disableShortCircuiting() {
  FENTER;
  if ( algorithmicState == INVALID ) { FRETURN( false ); }
  if (maxWidth == nVtxs) { FRETURN( false ); } 
  maxWidth = nVtxs;
  FRETURN( true );
}
#undef __FUNC__

#define __FUNC__ "bool BreadthFirstSearch::enablePartitionRestrictions( const int partitionMap[],  PartitionMembership membership = strict )"
bool 
BreadthFirstSearch::enablePartitionRestrictions( const int partitionMap[], PartitionMembership membership ) {
  FENTER;
  if ( algorithmicState == INVALID ) { FRETURN( false ); }
  if ((partition != 0) || (partitionMap == 0))  { FRETURN( false ); }
  partition = partitionMap;
  partitionMembership = membership; 
  FRETURN( true );
}
#undef __FUNC__

#define __FUNC__ "bool BreadthFirstSearch::disablePartitionRestrictions( )"
bool 
BreadthFirstSearch::disablePartitionRestrictions( ) {
  FENTER;
  if ( algorithmicState == INVALID ) { FRETURN( false ); }
  if (partition == 0) { FRETURN( false );  } // partition restrictions already disabled
  partition = 0; // NOTE:  BreadthFirstSearch does not own partition vector
  FRETURN( true );
}
#undef __FUNC__

const SharedArray<int>&
BreadthFirstSearch::getNew2Old() const { 
  if ( algorithmicState == DONE ) {
    return new2old;
  } else {
    return null_array;
  }
}

SharedArray<int>&
BreadthFirstSearch::getNew2Old() { 
  if ( algorithmicState == DONE ) {
    return new2old;
  } else {
    return null_array;
  }
}
  
const SharedArray<int>&
BreadthFirstSearch::getDistance() const { 
  if ( algorithmicState == DONE ) {
    return distance;
  } else { 
    return null_array;
  }
}

SharedArray<int>&
BreadthFirstSearch::getDistance() {
  if ( algorithmicState == DONE ) {
    return distance;
  } else { 
    return null_array;
  }
}

int 
BreadthFirstSearch::queryDistance( const int i ) const { 
  if ( algorithmicState == DONE ) { 
    if ((i >= 0) && (i < nVtxs)) { 
      return distance[i];
    }
  } 
  return -1;
}

SPINDLE_IMPLEMENT_DYNAMIC( BreadthFirstSearch, SpindleAlgorithm )

