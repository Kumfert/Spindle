//
// QuotientGraph.cc -- a quotient graph with "elbow room"
//
// $Id: QuotientGraph.cc,v 1.2 2000/02/18 01:31:46 kumfert Exp $
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


#include "spindle/QuotientGraph.h"
#include "spindle/SharedArray.h"

#include <string.h>
#include <strings.h> // defines bzero

#ifdef REQUIRE_OLD_CXX_HEADER_SUFFIX
#include <iomanip.h>
#include "function.h"
#include "algo.h"
#else
#include <iomanip>
#include <functional>
#include <algorithm>
using namespace std;
#endif

#include "spindle/spindle_func.h"

// #define register /* */

//static struct pair_first_less : public binary_function< pair<int,int>, pair<int,int>, bool> { 
//  bool operator()( const pair<int,int>& x, const pair<int,int>& y ) const { return x.first < y.first; }
//};

template <class InputIterator, class T>
T myHash(InputIterator first, InputIterator last, T init) {
  while (first != last) 
    init += 1 + *first++;
  return init;
}
 
#ifndef DEBUG_REACHABLE_SET
#define DEBUG_REACHABLE_SET false
#endif

#ifndef HAVE_RESTRICT_KEYWORD
#define restrict /**/
#endif

#define register /**/

// NOTE:
//  
//    The following preproccessor directive SPINDLE_QGRAPH_REPACK_AGGRESSIVE, 
//    is still under development.  Currently, the code performs as if it were
//    set on 1.  Recently, I've been concerned that this might be too expensive
//    and intend on using this flag to disable it.
//
// This code has a flag for doing extra effort in defragmenting
// the adjList:   SPINDLE_QGRAPH_REPACK_AGGRESSIVE
//
// If SPINDLE_QGRAPH_REPACK_AGGRESSIVE == 0 :
//       then the quotient graph will try to place
//       a newly formed enode in the space provided
//       by the eliminated snode, or other adjacent enodes
//       ( whichever is largest).  Otherwise, it will 
//       leave it in the elbow room.  This will likely
//       cause the most amount of explicit defragmenting, 
//       but has the lowest overhead, since it need not
//       mark unused spaced in adjList as non-positive
//
// If SPINDLE_QGRAPH_REPACK_AGGRESSIVE == 1 :
//       Then unused space in the adjList is guaranteed to 
//       be non-positive integers.  Thus, after the
//       new enode is constructed in the "elbow room"
//       it chooses the space of the largest adjacent enode
//       or the snode itself as before.  If it still does
//       not fit, however, it can go an extra step by
//       scanning adjacent space above and below the chosen
//       spot for more room.  If this doesn't work, it will
//       try to shift it up as high as possible and reset the
//       elbow room.
//
// Default behavior is equivalent to SPINDLE_QGRAPH_REPACK_AGGRESSIVE being set
//       to 1.
//
#define SPINDLE_QGRAPH_REPACK_AGGRESSIVE 1

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

#ifdef __FUNC__
#undef __FUNC__
#endif

#define __FUNC__ "QuotientGraph::QuotientGraph( const Graph* graph )"
QuotientGraph::QuotientGraph( const Graph* graph ) {
  if ( graph != 0 && graph->isValid() ) {
    n = graph->queryNVtxs();
    if ( graph->getVtxWeight().size() == n ) {
      initialize( graph->queryNVtxs(), graph->getAdjHead().lend(), graph->getAdjList().lend(), 
		  graph->getVtxWeight().lend() );
    } else {
      initialize( graph->queryNVtxs(), graph->getAdjHead().lend(), graph->getAdjList().lend(), 0 );
    }
  }
}
#undef __FUNC__

#define __FUNC__ "void QuotientGraph::initialize( const int nVertices, const int *AdjHead, const int *AdjList, const int *weights )" 
void 
QuotientGraph::initialize( const int nVertices, const int *AdjHead, const int *AdjList, const int *vwgt ) {
  FENTER;
  initFinalizeTimer[0].start();
  lastENode = -1;
  if ( nVertices ) ; // keeps compiler from complaining
  // timestamp = 1;
  nEliminatedNodes = 0;
  nCompressedNodes = 0;
  nOutmatchedNodes = 0;
  totEliminatedNodes = 0;
  totCompressedNodes = 0;
  totOutmatchedNodes = 0;
  nDefrags = 0;
  eliminateSupernodeTimer[0].reset();
  eliminateSupernodeTimer[1].reset();
  eliminateSupernodeTimer[2].reset();
  singleUpdateTimer[0].reset();
  singleUpdateTimer[1].reset();
  singleUpdateTimer[2].reset();
  singleUpdateTimer[3].reset();
  singleUpdateTimer[4].reset();
  multipleUpdateTimer[0].reset();
  multipleUpdateTimer[1].reset();
  multipleUpdateTimer[2].reset();
  multipleUpdateTimer[3].reset();
  resetReachSetTimer[0].reset();
  resetReachSetTimer[1].reset();
  resetReachSetTimer[2].reset();
  // reasonable defaults
  isSingleElimination = false;
  incrementByOne = false;
  //  computeSetDiffs = false;
  //  computeExtDeg = true;
  lastEliminatedNode.resize(0);
  timestamp = 0;

  // userForceCompressMixAdjVtxs = -1;
  // userNAdjClqBound = -1;

  int nnz = AdjHead[n] + n;
  maxSpace = ( ( nnz * 1.3 ) > nnz + n ) ? (int)(nnz * 1.3) : nnz + n;
  if ( (adjList_ = (int *) malloc( maxSpace * sizeof(int) ) ) == 0 ) {
    // not enough memory, 
    if ( maxSpace == nnz + n ) { 
      cerr << "not enough memory!" << endl;
      exit( -1 );
    }
    // try again.
    maxSpace = ( ( nnz * 1.2 ) > nnz + n ) ? (int)(nnz * 1.2) : nnz + n;
    if ( (adjList_ = (int *) malloc( maxSpace * sizeof(int) ) ) == 0 ) {
      // not enough memory, 
      if ( maxSpace == nnz + n ) { 
	cerr << "not enough memory!" << endl;
	exit( -1 );
      }
      // try again.
      maxSpace = ( ( nnz * 1.1 ) > nnz + n ) ? (int)(nnz * 1.1) : nnz + n;
      if ( (adjList_ = (int *) malloc( maxSpace * sizeof(int) ) ) == 0 ) {
	// not enough memory, 
	if ( maxSpace == nnz + n ) { 
	  cerr << "not enough memory!" << endl;
	  exit( -1 );
	}
	// try again.
	maxSpace = nnz + n;
	if ( (adjList_ = (int *) malloc( maxSpace * sizeof(int) ) ) == 0 ) {
	  // not enough memory, 
	  cerr << "not enough memory!" << endl;
	  exit( -1 );
	} // end maxSpace = nnz + n;
      } // end maxSpace = max( 1.1 * nnz, nnz+n );
    } // end maxSpace = max ( 1.2 * nnz, nnz+n );
  } // end maxSpace = max( 1.3 * nnz, nnz+n );

  // need to copy AdjList to adjList_, but need to add a 
  // self edge at the beginning of each list.
  adjHead_ = new int [ n ]; 
  nSnodes_ = new int [ n ];
  { 
    register int curidx = 0;
    {for ( register int i=0; i<n; ++i ) {
      adjList_[curidx] = i;
      adjHead_[i] = curidx;
      ++curidx;
      nSnodes_[i] = -curidx; 
      for( register int j =AdjHead[i], j_max = AdjHead[i+1]; j<j_max ; ++j ) {
	int temp = adjList_[curidx] = AdjList[j];
	if ( temp != i ) { 
	  ++curidx;
	}
      }
      nSnodes_[i] += curidx;
    }}
    freeSpace = curidx;
    // no need to fill in the rest with -1;
  }
  
  {
    register int * restrict ip;
    nEnodes_    = ip = new int [n];   {for (register int i=0; i<n; ++i) *ip++ = 1;}
    parent_     = ip = new int [n];   {for (register int i=0; i<n; ++i) *ip++ = i;}
    next_       = ip = new int [n];   {for (register int i=0; i<n; ++i) *ip++ = i;}
    prev_       = ip = new int [n];   {for (register int i=0; i<n; ++i) *ip++ = i;}
    updateList_ = ip = new int [n+1]; {for (register int i=0; i<n; ++i) *ip++ = -1;}
    updateList_[n] = n;
    visited_    = ip = new int [n];   {bzero( (char *) ip, n*sizeof(int) );}
    setDiff_    = ip = new int [n];   {bzero( (char *) ip, n*sizeof(int) );}
    if ( vwgt == 0 ) {
      weight_ = ip = new int [n]; {for (int i=0; i<n; ++i) *ip++ = 1;}
      externDeg_ = ip = new int [n];   {for (int i=0; i<n; ++i) *ip++ = nSnodes_[i]; }
    } else {
      weight_ = new int [n];
      memcpy( weight_, vwgt, sizeof(int) * n );
      externDeg_ = ip = new int [n];   {for (int i=0; i<n; ++i) *ip++ = 0; }
      {for (register int i=0; i<n; ++i) {
	for(register int j=adjHead_[i]+1, j_max = adjHead_[i]+nSnodes_[i]+1; j<j_max; ++j ) {
	  externDeg_[i] += weight_[adjList_[j]];
	}
      }}
    }
  }
  sorter = new ArrayBucketSorter(n,n);  
  FEXIT;
}
#undef __FUNC__

QuotientGraph::~QuotientGraph() {
  delete[] adjHead_;
  delete[] nEnodes_;
  delete[] nSnodes_;
  delete[] weight_;
  delete[] parent_;
  delete[] next_;
  delete[] prev_;
  delete[] updateList_;
  delete[] externDeg_;
  delete[] visited_;
  if ( setDiff_ != 0 ) { 
    delete[] setDiff_;
  }
  delete sorter;
  free( adjList_ );
}

int 
QuotientGraph::deg( const int i ) const { 
  register const int * const restrict weight = weight_;
  register	 int * const restrict visited = visited_;
  int degree = 0;
  int stamp = nextStamp();
  visited[ i ] = stamp;

  for ( const int * e = enode_begin(i), * stop_e = enode_end(i); e != stop_e; ++e ) { 
    for ( const int * s = snode_begin(*e), *stop_s = snode_end(*e); s != stop_s; ++s ) { 
      if ( visited[ *s ] < stamp ) { 
	visited[ *s ] = stamp;
	degree += weight[*s];
      }
    }
  }
  return degree;
}

#define __FUNC__ "bool QuotientGraph::eliminateSupernode( const int i )"
bool QuotientGraph::eliminateSupernode( const int i ) {
  FENTER;

  ////////////////////////////////////////////////////
  //
  // I:  Verify that "i" is eligible for elimination
  //
  if ( (i<0) || (i>=n) || (nEnodes_[i]<0) || (parent_[i]!=i) || (updateList_[i]>=0) ) { 
    // (i<0) || (>=n) implies an invalid index
    // nEnodes[i]<0 implies "i" is already eliminated (eliminated nodes have no enodes)
    // parent[i]!=i implies that "i" is non-principal
    // updateList[i]>=0 implies that "i" is already on the updateList.
    //     The graph must be updated before "i" can be eliminated.
    TRACE( SPINDLE_TRACE_QGRAPH, 
	   "\n\tQuotientGraph::eliminateSupernode() { }" );
    FRETURN( false ); 
  }
  if ( isSingleElimination && ( lastEliminatedNode.size() != 0 ) ) { 
    // if we're restricted to single elimination and the last vertex
    // that was eliminated hasn't been updated yet, don't allow any more
    TRACE( SPINDLE_TRACE_QGRAPH, 
	   "\n\tQuotientGraph::eliminateSupernode() { }" );
    FRETURN( false );
  }
  // end PHASE I:
  TRACE( SPINDLE_TRACE_QGRAPH, 
	 "\n\tQuotientGraph::eliminateSupernode() { %d : ", i );


  lastENode = i;

  eliminateSupernodeTimer[0].start();
  int mySrc = adjHead_[i];     // where my information comes from 
  int myNEnodes = nEnodes_[i]; // number of enodes
  int myNSnodes = nSnodes_[i]; // number of supernodes
  if ( updateList_[n] == n ) { // if the update list is empty
    nEliminatedLastStage = 1;  //   starting a new round
  } else {                     // else
    ++nEliminatedLastStage;    //   increment here
  }

  //////////////////////////////////
  //
  // II: assemble enode from eliminated snode
  //
  {
    register       int * const restrict adjHead = adjHead_;
    register       int * const restrict adjList = adjList_;
    register       int * const restrict nEnodes = nEnodes_;
    register       int * const restrict nSnodes = nSnodes_;
    register       int * const restrict weight = weight_;
    register       int * const restrict setDiff = setDiff_;
    register       int * const restrict visited = visited_;
    register       int * const restrict parent = parent_;
    register const int * const restrict prev = prev_;
    register const int * const restrict next = next_;

    if ( myNEnodes == 1 ) {      // if "i" has no adjacent enodes...
      // ... build element in place

      // NOTE:
      //      This optimization allows us to skip many
      //      details.
      adjList[ mySrc ] = -1;    // flag location as unused.
      mySrc = ++adjHead[i];     // set pointer past inital enode
      int new_weight = 0;
      for(register int cur = mySrc, stop = mySrc+myNSnodes; cur<stop;++cur ) {
	new_weight += weight[adjList[cur]];
      }
      weight[i] = new_weight; // weight of a clique is the sum of its adjacent vtxs.
      setDiff[i] = new_weight;
    } else {                   // build new element in freeSpace
      int pack_begin = mySrc;
      int pack_end   = mySrc+myNSnodes+myNEnodes;
      int new_weight = 0;
      int dest = freeSpace;          // place where enode is assembled
      const int stamp = nextStamp(); // get the next stamp
      visited[ i ] = stamp;          // mark "i" as visited
      /////////////////////////////////////////////////////////////
      //
      // II a. for all adjacent enodes...
      //
      {
	for ( int j = mySrc, j_stop = mySrc+myNEnodes; j<j_stop; ++j) {
	  int enode = adjList[j];   
	  if ( enode < 0 ) {        // skip invalid numbers
	    continue;
	  } 
	  while( adjHead[ enode ] < 0 ) { // go to parents of non-principal enodes
	    enode = parent[ enode ];
	  }
	  adjList[j] = -1;                // make space available
	  
	  if ( nSnodes[enode] > ( pack_end - pack_begin ) ) { // remember largest enode adjlist
	    pack_begin = adjHead[enode];
	    pack_end   = adjHead[enode]+nSnodes[enode];
	  }
	  if ( enode != i ) { // if current enode is not the one we're just creating
	    // for all snodes adjacent to enode
	    // Mark all snodes adj to adj enodes as visited
	    for ( int k = adjHead[enode], k_stop = adjHead[enode]+nSnodes[enode]; 
		  k < k_stop; ++k ) {
	      // NOTE:
	      //      Since enodes have no adjacent enodes nEnodes[enode] 
	      // actually stores -(k+1) where k is the step that the enode
	      // was eliminated.  Therefore iterating from adjHead[enode]
	      // to adjHead[enode]+nSnodes[enode] is correct.
	      int snode = adjList[k];
	      if ( snode < 0 ) { 
		continue;
	      }
	      while( adjHead[ snode ] < 0 ) {   // if we have a non-principal node
		snode = parent[ snode ];        //    ... go to its parent
	      }
	      adjList[k] = snode;               // write the parent back
	      if ( visited[ snode ] >= stamp) { // if already visited...
		adjList[k] -= n;                //   ... mark as old data
		continue;                       //   ... go to next snode
	      }
	      visited[ snode ] = stamp;         // mark this snode as visited
	      new_weight += weight[ snode ];    // increase weight of this enode	
	      if ( dest == maxSpace ) { // if ran out of free space
		//
		// store some necessary info
		//
		adjHead[i] = j;             // write current enode as beginning
		adjList[ adjHead[i] ] = enode;
		nEnodes[i] = j_stop - j;    // store number of enodes not absorbed
		// store beginning of partially assembled new adjList
		int oldFreeSpace = freeSpace;  
		// store number of snodes done in current enode
		int nSnodesDone = k - adjHead[ enode ];
		// store number of snodes already in partially assembled enode
		int nItems = dest - freeSpace;

		//
		// defragment adjList array
		//
		defragAdjList();
		// NOTE:
		// This will repack all principal and outmatched snodes and enodes
		// and will reset freeSpace.
		// It does not change any values after the new freeSpace. so
		// our partially constructed element is preserved.
		++nDefrags;

		//
		// move the partially constructed element to beginning of new freeSpace
		//
		memcpy( adjList + freeSpace, adjList + oldFreeSpace, 
			nItems*sizeof(int) );
		
		//
		// now reset some important variables
		//
		mySrc = adjHead[i];                          // where my info is from
		myNEnodes = nEnodes[i];                      // number of enodes;
		myNSnodes = nSnodes[i];                      // number of snodes;
		j = mySrc;                                   // begin enodes adj to i
		j_stop = mySrc + myNEnodes;                  // end enodes adj to i
		enode = adjList[ j ];                        // get enode id#
		adjList[ j ] = -1;                           // free up space in adjList
		k = adjHead[ enode ] + nSnodesDone;          // begin snodes adj to enode
		k_stop = adjHead[ enode ] + nSnodes[ enode ];// end snodes adj to enode
		snode = adjList[ k ];                        // get snode id#
		dest = freeSpace + nItems;                   // set new dest
		pack_begin = mySrc;
		pack_end = mySrc+myNEnodes+myNSnodes;
	      }
	      // cleanup
	      adjList[dest] = snode;     // copy snode to current destination
	      ++dest;                    // increment destination
	      adjList[k] = -1;           // free up old location
	    } // end for all snodes
	    // the current enode is absorbed by "i"
	    parent[ enode ] = prev[i]; 
	    
	    // relabel parent pointer of all outmatched enodes(if any)
	    {
	      int outmatched_enode = next[ enode ];
	      while( outmatched_enode != enode ) {
		parent[ outmatched_enode ] = prev[ i ];
		outmatched_enode = next[ outmatched_enode ];
	      }
	    }
	    // NOTE:
	    // prev[i] will be the tail of the chain in the etree formed by the
	    // elimination of a large snode.

	    // Now the adjacent enode has no data, no weight, 
	    weight[ enode ] = 0;
	    nSnodes[ enode ] = 0;
	    adjHead[ enode ] = -1;
	  } // end if enode != i
	} // end for all adjacent enodes
      } // end II a. 

      ///////////////////////////
      //
      // II b. Now copy any unvisited adjacent snodes
      //
      {
	for ( int k = mySrc + myNEnodes, stop_k = mySrc + myNEnodes + myNSnodes ; 
	      k < stop_k; ++k ) { 
	  int snode = adjList[k];
	  if ( snode < 0 ) {                // if we happened upon unused space
	    continue;                       //    ... skip it.
	  }
	  int ancestor = snode;
	  while( adjHead[ ancestor ] < 0 ) {// if we have a non-principal node
	    ancestor = parent[ ancestor ];  //    ... go to its parent
	  }                                 //     
	  if ( parent[ snode ] >= 0 ) {     // as long as snode is not outmatched
	    parent[ snode ] = ancestor;     //    set parent to ancestor
	  }
	  adjList[k] = snode;               // write the parent back
	  if ( visited[ snode ] >= stamp) { // if already visited...
	    adjList[k] -= n;                //   ... mark as old data
	    continue;                       //   ... go to next snode
	  }
	  visited[ snode ] = stamp;         // mark this snode as visited
	  new_weight += weight[ snode ];    // increase weight of this enode	
	  if ( dest == maxSpace ) { // if ran out of free space
	    //
	    // store some necessary info
	    //
	    adjHead[i] = k;             // write current snode as beginning
	    adjList[ adjHead[i] ] = snode;
	    nEnodes[i] = 0;             // don't have any more enodes
	    nSnodes[i] = stop_k - k;    // we know exactly how many snodes
	    // store beginning of partially assembled new adjList
	    int oldFreeSpace = freeSpace;  
	    // store number of snodes already in partially assembled enode
	    int nItems = dest - freeSpace;

	    //
	    // defragment adjList array
	    //
	    defragAdjList();
	    // NOTE:
	    // This will repack all principal and outmatched snodes and enodes
	    // and will reset freeSpace.
	    // It does not change any values after the new freeSpace. so
	    // our partially constructed element is preserved.
	    ++nDefrags;
	    
	    //
	    // move the partially constructed element to beginning of new freeSpace
	    //
	    memcpy( adjList + freeSpace, adjList + oldFreeSpace, 
		    nItems*sizeof(int) );
	    
	    //
	    // now reset some important variables
	    //
	    mySrc = adjHead[i];                          // where my info is from
	    myNEnodes = nEnodes[i];                      // number of enodes ==0
	    myNSnodes = nSnodes[i];                      // number of snodes;
	    k = adjHead[ i ];                            // begin snodes adj to enode
	    stop_k = adjHead[ i ] + nSnodes[ i ];        // end snodes adj to enode
	    dest = freeSpace + nItems;                   // set new dest
	    pack_begin = mySrc;
	    pack_end = mySrc+myNEnodes+myNSnodes;
	  }
	  // cleanup
	  adjList[dest] = snode;     // copy snode to current destination
	  ++dest;                    // increment destination
	  adjList[k] -= n;           // free up old location
	} // end for all snodes
      }// end  II c.

      weight[i] = new_weight;
      setDiff[i] = new_weight;
      myNSnodes = dest - freeSpace;
      
      ///////////////////////////////////////////////////
      //
      // II.b. Now place the newly formed element, if possible
      //
      {
	int cleanup_begin = freeSpace + myNSnodes;  //space that needs reclaiming later
	// int cleanup_end = freeSpace + myNSnodes;     
#if ( SPINDLE_QGRAPH_REPACK_AGGRESIVE > 0 )
	if ( (pack_end - pack_begin) < myNSnodes ) { 
	  // if current enode doesn't fit in space from largest contributing node
	  // scan for more space above
	  while ( (--pack_begin >=0 ) && ( adjList[ pack_begin ] < 0 ) ) 
	    ;
	  ++pack_begin;
	}
	if ( (pack_end - pack_begin) < myNSnodes ) { 
	  // if current enode _still_ doesn't fit in space from largest contributing node
	  // scan for more space below
	  while ( (pack_end < freeSpace ) && ( adjList[ pack_end ] < 0 ) ) {
	    if ( pack_end - pack_begin >= myNSnodes ) {
	      break;
	    }
	    ++pack_end;
	  }
	}
#endif
	if ( pack_end == freeSpace ) { 
	  // pack space is immediately above freeSpace
	  // just shift it up and go.
	  memcpy( adjList + pack_begin, adjList + freeSpace, myNSnodes * sizeof(int) );
	  adjHead[i] = pack_begin;
	  nSnodes[i] = myNSnodes;
	  freeSpace = pack_begin + myNSnodes;
	  cleanup_begin = freeSpace;
	} else if ( ( pack_end - pack_begin ) >= myNSnodes ) { 
	  // if new enode can fit into pack_space... pack it there
	  memcpy( adjList + pack_begin, adjList + freeSpace, myNSnodes * sizeof(int) );
	  adjHead[i] = pack_begin;
	  nSnodes[i] = myNSnodes;
#if ( SPINDLE_QGRAPH_REPACK_AGGRESSIVE > 0 ) 
	  // now try to move freeSpace up if possible
	  while ( (adjList[ --freeSpace ] < 0) && (freeSpace > 0) ) // while the one above is negative.
	    ;
	  ++freeSpace; // whoops! either freeSpace is negative, or adjList[ freeSpace] is already used.
#endif
	  cleanup_begin = freeSpace;
	} else {
	  // commit new enode to the freeSpace where it is now.
	  // ... but first see if we can shift up some.
	  pack_begin = freeSpace;
	  do { 
	    --pack_begin;
	  } while ( (pack_begin >=0 ) && ( adjList[ pack_begin ] < 0 ) ) ;
	  ++pack_begin;
	  // ... now see how far we can shift
	  if ( pack_begin != freeSpace ) { // if we've made some progress
	    // shift it up as much as possible
	    memcpy( adjList + pack_begin, adjList + freeSpace, myNSnodes * sizeof(int) );
	    adjHead[i] = pack_begin;
	    nSnodes[i] = myNSnodes;
	    freeSpace = pack_begin + myNSnodes;
	    cleanup_begin = freeSpace;
	  } else { // just commit it where it is
	    adjHead[i] = freeSpace;
	    nSnodes[i] = myNSnodes;
	    freeSpace += myNSnodes;
	  } // end else 
	} // end commit it somewhere in freeSpace
	
	//#if ( SPINDLE_QGRAPH_REPACK_AGGRESSIVE > 0 )
	// Reclaim space in elbow room
	//	while ( cleanup_begin < cleanup_end ) { 
	// adjList[cleanup_begin] = -1;
	// ++cleanup_begin;
	//}
	//#endif
      } // end IIb.
    } // end else if ( nEnodes[i] > 1 )
  } // end II:
  eliminateSupernodeTimer[0].stop();

  // reset some information
  mySrc = adjHead_[i];     // where my information comes from 
  myNEnodes = nEnodes_[i]; // number of enodes
  myNSnodes = nSnodes_[i]; // number of supernodes

  eliminateSupernodeTimer[1].start();
  ///////////////////////////////////////
  //
  // III:  number enode and update elimination forest
  //
  {
    //cout << "Eliminate Supernode: ";

    // NOTE:
    //      If "i" represented several snodes, then the snodes form a path
    //      in the elimination forest.
    //
    register int * nEnodes = nEnodes_;
    register int * parent = parent_;
    register int * next = next_;
    register int * prev = prev_;
    register int tail = prev[ i ];         // prev[i] = bottom of the path
    while ( tail != i ) {                  // while cur snode in path is not the top...
      //ASSERT( nEnodes[ tail ] >= 0, "Trying to number a node that is already numbered");
      TRACE( SPINDLE_TRACE_QGRAPH, "%d ", tail);
      nEnodes[ tail ] = -(++totEliminatedNodes); //    mark cur as eliminated here.
      parent[ tail ] = prev[ tail ];       //    set its parent to next snode in path.
      int oldtail  = tail;                 //    save the current snode
      tail = prev[ tail ];                 //    advance to next snode in path
      prev[ oldtail ] = oldtail;           //    delete list info for previous snode
      next[ oldtail ] = oldtail;           //    delete list info for previous snode
      --nCompressedNodes;                  //    one less uneliminated compressed node
    }
    nEnodes[ i ] = - (++totEliminatedNodes); // number top of chain
    ++nEliminatedNodes;                    // add one eliminated supernode
    prev[ i ] = i;
    next[ i ] = i;
    lastEliminatedNode.push_back(i);
  } // end number snode
  TRACE( SPINDLE_TRACE_QGRAPH, "}");
  eliminateSupernodeTimer[1].stop();

  
  /////////////////////////////////////
  //
  // IV: Add all snodes adj to i to updateList.
  //        (The adjLists of these nodes is not consistant until ::update() )
  //
  eliminateSupernodeTimer[2].start();
  {
    register const int * const restrict nEnodes = nEnodes_;
    register const int * const restrict adjList = adjList_;
    register       int * const restrict parent = parent_;
    register       int * const restrict updateList = updateList_;
    int head = updateList[n];   // head == n implies list is empty, 
                                // updateList[j] == n implies end of list
    for ( register int j = mySrc, j_stop = mySrc + myNSnodes; j<j_stop; ++j ) {
      int s = adjList[ j ];
      if ( parent[ s ] < 0 ) { // if outmatched
	int outmatchedBy = n + parent[ s ];
	if ( nEnodes[ outmatchedBy ] < 0 ) { 
	  parent[ s ] = s;
	  --nOutmatchedNodes;
	}
	if ( updateList[ s ] < 0 ) {  // if s is not yet in updateList[]
	  updateList[ s ] = head;
	  head = s ;
	}
      } else if ( parent[ s ] == s ) { 
	if ( updateList[ s ] < 0 ) {  // if s is not yet in updateList[]
	  updateList[ s ] = head;
	  head = s ;
	}
      }
    }
    updateList[n] = head;       // save the head of the list
    // NOTE:
    //  if we insert just one item into an empty list. That one item 
    //  will be the only one with a positive entry "n" within [0,n)
  } // end III
  eliminateSupernodeTimer[2].stop();
  
  FRETURN( true );
}
#undef __FUNC__

#define __FUNC__ "bool QuotientGraph::update( VertexList& updateNodes, VertexList& removeNodes )"
bool
QuotientGraph::update( VertexList& updateNodes, VertexList& removeNodes ) {
  FENTER;

  //
  // QuotientGraph update, will accomplish the following
  //
  // 1. update the graph (if any snodes have been eliminated)
  // 2. reset the two vertex lists
  // 3. add all snodes that have been merged or outmatched in
  //    remove nodes.
  // 4. add all snodes that require a priority update in updateNodes
  // 5. reset the updateList so that all nodes are elligible for
  //    elimination.
  // 6. if all nodes are eliminated, it will also create the 
  //    old2new ordering out of the information stored in
  //    the nEnodes[] array
  if ( lastEliminatedNode.size() == 0 ) {
    FRETURN( false );
  }
  updateTimer.start();
  updateNodes.resize(0);
  removeNodes.resize(0);

  if ( isSingleElimination ) {
    // DO single elimination update

    const int stamp = nextStamp(); 
    // NOTE:  The same stamp is shared in the next two subroutines.
    singleUpdateTimer[0].start();
    computeSetDiffs( stamp );
    singleUpdateTimer[0].stop();
    
    singleUpdateTimer[1].start();
    stripOldEntriesAndPackIntoBucketSorter( stamp );
    singleUpdateTimer[1].stop();

    singleUpdateTimer[2].start();
    scanBucketSorterForSupernodes( removeNodes );
    singleUpdateTimer[2].stop();

    singleUpdateTimer[3].start();
    pruneNonPrincipalSupernodes( updateNodes );
    singleUpdateTimer[3].stop();

    singleUpdateTimer[4].start();
    resetUpdateList( );
    singleUpdateTimer[4].stop();
  } else {
    // Do multiple elimination update
    int adj1Head = n;
    int adj2Head = n;
    int adjNHead = n;
    
    // NOTE: updateList is split between snodes that are adjacent only
    //       to two enodes, and all others.
    multipleUpdateTimer[0].start();
    //cout << "Before update()" << endl;
    //printShortGraph();
    xferNewEnodesToEnodeListsOfReachSet( );
    //cout << "After transfer()" << endl;
    //printShortGraph();
    cleanEnodes();
    //cout << "After clean enodes()" << endl;
    //printShortGraph();
    cleanSnodes();    
    //cout << "After clean snodes()" << endl;
    //printShortGraph();
    multipleUpdateTimer[0].stop();

    multipleUpdateTimer[1].start();
    sortReachSet( adj1Head, adj2Head, adjNHead );
    // stripOldEntriesFromAdjListsOfReachSet( adj2Head, adjNHead );
    multipleUpdateTimer[1].stop();

    multipleUpdateTimer[2].start();
    compressAndOutmatchReachSet( adj2Head, removeNodes );
    multipleUpdateTimer[2].stop();
    //cout << "compress and outmatch reach set" << endl;
    //printShortGraph();

    multipleUpdateTimer[3].start();
    resetReachSet( adj1Head, adj2Head, adjNHead, updateNodes);
    multipleUpdateTimer[3].stop();
  }
  updateTimer.stop();

  //printShortGraph();

  if ( totEliminatedNodes == n ) { 
    initFinalizeTimer[1].start();
    // convert nEnodes to old2new perm
    //  old2new = -(nEnodes[i]+1)
    register int * nEnodes = nEnodes_;
    for( register int i =0; i<n; ++i ) { 
      nEnodes[i] = -(nEnodes[i] + 1);
    }
    initFinalizeTimer[1].stop();
  } 
  //ASSERT( totEliminatedNodes <= n , "totEliminatedNodes = %d > n = %d", totEliminatedNodes, n );
  if ( totEliminatedNodes > n ) { 
    initFinalizeTimer[1].start();
    register int * nEnodes = nEnodes_;
    int  minimum = 2*n;
    int  maximum = -1;
    int * temp_array = new int [n];
    for( int i = 0; i<n ; ++i ) { 
      nEnodes[i] = -(nEnodes[i] + 1);
      if ( ( nEnodes[i] < n ) && ( nEnodes[i]>=0 ) ) { 
	if (nEnodes[i]>maximum ) {
	  maximum = nEnodes[i];
	}
	if ( nEnodes[i]<minimum ) { 
	  minimum = nEnodes[i];
	}
	temp_array[ nEnodes[i] ] = 1;
      } else { 
	ERROR( 0, " nEnodes[%d] = %d, where n=%d\n", i, nEnodes[i], n );
      }
    }
    //PRINTF("Maximum = %d, minimum = %d\n", maximum, minimum );
    for ( int i = 0; i<n; ++i ) { 
      if ( temp_array[ i ] == 0 ) { 
	ERROR(0, " no index number %d found for ordering\n", i);
      }
    }
    delete[] temp_array;
    initFinalizeTimer[1].stop();
  }
  FRETURN( true );
}
#undef __FUNC__



#define __FUNC__ "void QuotientGraph::computeSetDiffs();"
void 
QuotientGraph::computeSetDiffs( const int stamp ) {
  FENTER;

  register const int * const restrict adjHead = adjHead_;
  register       int * const restrict adjList = adjList_;
  register const int * const restrict nEnodes = nEnodes_;
  register const int * const restrict nSnodes = nSnodes_;
  register const int * const restrict weight = weight_;
  register       int * const restrict setDiff = setDiff_;
  register       int * const restrict visited = visited_;
  register       int * const restrict parent = parent_;

  /////////////////////////////////
  //
  // First compute setDiffs of  | adj(e2) \ adj(e) | where
  // "e" is the most recently eliminated node.
  //

  int me = lastEliminatedNode.back();
  int head = adjHead[ me ];
  int nItems = 0;

  // for all snodes adjacent to last eliminated node
  for ( int i = head, stop_i = head + nSnodes[ me ]; i<stop_i; ++i ) {
    int s = adjList[ i ];
    int ancestor = s;
    while ( adjHead[ ancestor ] < 0 ) {  // while snode is compressed
      ancestor = parent [ ancestor ];    // go to parent
    }
    if ( parent[ s ] >= 0 ) {            // as long as s is not outmatched
      parent[ s ] = ancestor;            // ensure parent is self
    }

    if ( visited[ s ] < stamp ) {  // if not yet visited...
      visited[ s ] = stamp;        //    mark the adjacent snode as "visited" for later
      adjList[ head + nItems ] = s;//    copy unique principal snode back to adjList
      ++nItems;                    //    increase number of items
      if ( nEnodes[ s ] > 1 ) { // if the supernode has another adj enode...
	// Note:
	//      at this time, the snode hasn't been updated to the newest enode.
	//      so nEnodes[s] only has enodes other than newest enode.
	
	// for all enodes, e, adjacent to supernode,s
	for ( int j = adjHead[ s ] , stop_j = adjHead[s] + nEnodes[s] ; j <stop_j; ++j ) {
	  int e = adjList[ j ];          
	  if ( ( parent[e] != e ) && ( parent[e]>0 ) ) {  
	                        // if element is absorbed
	    e = me;             // the absorbing element must be the most recent one.
	    adjList[j] = me;    // replace it in adjlist 

	  }
	  if( ( parent[e] == e ) || (parent[e] < 0 ) ) {   
	                                 //   if enodes has not been absorbed.
	    if ( visited[e] < stamp ) {  //     if enode has not been visited
	      visited[e] = stamp;        //        it is now & initialize setDiff
	      setDiff[e] = weight[e] - weight[s];   
	    } else {                     //     else if it has been visited
	      setDiff[e] -= weight[s];   //     just subtract the common vertex weight
	    }
	  } // end if principal or outmatched
	} // end for all enodes
      } // end if supernode has another adj enode
    } // end if not yet visited
  } // end for all snodes adjacent to last eliminated node

#if ( SPINDLE_QGRAPH_REPACK_AGGRESSIVE > 0 ) 
  // flag unused spaces as available for reclaimation.
  int oldNItems = nSnodes[ me ];
  for ( int k = head+nItems, stop_k = head+oldNItems; k < stop_k ; ++k ) { 
    adjList[ k ] = -1;
  }
#endif

  setDiff_[ me ] = weight[ me ];
  nSnodes_[ me ] = nItems;

  FEXIT;
}
#undef __FUNC__



#define __FUNC__ " void stripOldEntriesAndPackIntoBucketSorter( const int stamp );"
void 
QuotientGraph::stripOldEntriesAndPackIntoBucketSorter( const int stamp ) {
  FENTER;
  register const int * const restrict adjHead = adjHead_;
  register       int * const restrict adjList = adjList_;
  register       int * const restrict nEnodes = nEnodes_;
  register       int * const restrict nSnodes = nSnodes_;
  register       int * const restrict weight = weight_;
  register const int * const restrict setDiff = setDiff_;
  register       int * const restrict parent = parent_;
  register       int * const restrict next = next_;
  register       int * const restrict prev = prev_;
  register const int * const restrict visited = visited_;

  int me = lastEliminatedNode.back();
  int head = adjHead[ me ];
  
  ////////////////////////////////
  //
  // element absorption
  //
  // int nleft = n - totEliminatedNodes;
  // for all snodes adjacent to newest formed enode...
  for ( int i = head, stop_i = head + nSnodes[ me ]; i<stop_i; ++i ) {
    int s = adjList[i];                      // name of the snode
    while ( adjHead[ s ] < 0 ) {             // while it has been merged
      s = parent[ s ];                       // find its parent
    }
    adjList[i] = s;                          // write the parent back
    register int src  = adjHead[ s ];        // beginning of s space
    register int dest = adjHead[ s ];
    int degree = weight[ me ] - weight[ s ]; // degree of snode s
    // int dfill  = degree;
    int hashval = 0; // -(s+1);                     // include snode in hashval
      
    // forall  enodes, e, adjacent to snode, s
    for ( const int  stop = adjHead[ s ] + nEnodes[ s ];  src<stop; ++src ) {
      int e = adjList[ src ];           // 
      if ( e<0 || e>=n || e == me || weight[e]==0) {   // if e is bad value, or e is me
	;                               //    ... skip to next iteration
      } else 	if ( e == s ) {         // else if e is really self-edge to snode
	adjList[ dest ] = e;            //    copy it to new list
	++dest;
      } else if ( setDiff[ e ] == 0 ) { // if enode is outmatched by newest one
	// TRACE( SPINDLE_TRACE_QGRAPH_UPD, "\n\tOutmatch vtx %d by %d", e, me );
	parent[ e ] = me - n;           //    outmatch it with newest one
	weight[ e ] = 0;                //    set weight = 0;
	// link me and e in indistinguishable lists
	int prev_me = prev[ me ];
	next[ prev[ me ] ] = e;
	prev[ me ] = prev[ e ];
	next[ prev[ e ] ] = me;
	prev[ e ] = prev_me;
      } else if ( ( setDiff[ e ] > 0 ) 
		  && ( parent[e]==e) ) {// else if element is not outmatched
	degree += setDiff[ e ];         //    add set diff
	adjList[ dest ]  = e;           //    copy to new list
	++dest;
	hashval += (e+1);               
      }
    } // end forall enodes, e, adj to snode, s.
    
    // remember number of enodes in s, (not including me, but we'll add that later)
    int newNEnodes = dest - adjHead[ s ];
    ASSERT( adjHead[ s ] >= 0 ,"adjHead[s] does not have meaningful data");
    ASSERT( newNEnodes >= 0, "cannot have a negative number of enodes");

    // forall snodes, z, adjacent to snode, s. 
    for( const int stop = adjHead[s] + nEnodes[s] + nSnodes[s]; src < stop; ++src ) {
      int z = adjList[ src ];
      if ( nEnodes[z] > 0) {            // if z is an snode
	if ( visited[z] < stamp ) {     // if z is not in adj(me) 
	  // (remember we flagged these in computeSetDiffs() too )
	  // add to degree and add to new list
	  degree += weight[z];
	  adjList[ dest ] = z;
	  ++dest;
	  hashval += (z+1);
	}
      }
    } // end forall snodes, z, adjacent to snode, s.
    
    // save external degree
    externDeg_[s] = max( 0, degree );

    // now pack me into enode list
    adjList[ dest ] = adjList[ adjHead[s] + newNEnodes ]; // copy 1st snode to end
    ++dest;
    adjList[ adjHead[s] + newNEnodes ] = adjList[ adjHead[s] ]; // cpy 1st enode to end 
    adjList[ adjHead[s] ] = me;                     // put me at front of enode list
    
    // finalize the number of enodes and snodes adj to 's'
    //ASSERT( nEnodes[s] >= 0, "clobbering permutation");
    nEnodes[s] = newNEnodes + 1;                   // update number of enodes
    ASSERT( nEnodes[s] >= 0,"Cannot have a negative number of enodes");
    nSnodes[s] = dest - adjHead[s] - nEnodes[s];   // update number of snodes
    ASSERT( nSnodes[s] >= 0,"Cannot have a negative number of snodes");

    // insert snode, s, into bucket sorter
    sorter->insert( hashval, s );
  } // end for all snodes adjacent to me
  FEXIT;
}
#undef __FUNC__



#define __FUNC__ "void QuotientGraph::scanBucketSorterForSupernodes( )"
void
QuotientGraph::QuotientGraph::scanBucketSorterForSupernodes( VertexList& removeNodes ) {
  FENTER;

  register int * const restrict adjHead = adjHead_;
  register int * const restrict adjList = adjList_;
  register int * const restrict nEnodes = nEnodes_;
  register int * const restrict nSnodes = nSnodes_;
  register int * const restrict weight = weight_;
  register int * const restrict externDeg = externDeg_;
  register int * const restrict visited = visited_;
  register int * const restrict parent = parent_;
  register int * const restrict next = next_;
  register int * const restrict prev = prev_;
  
  ////////////////////////////////////////
  //
  // supervariable detection
  // 
  // examine all hash buckets with two or more snodes.
  // Do this by examining all unique hash keys for all
  // snodes adj to me
  int me = lastEliminatedNode.back();
  int head = adjHead[ me ];
  
  // for all snodes adjacent to newest formed enode...
  for ( int i = head, stop_i = head + nSnodes[ me ]; i<stop_i; ++i ) {
    int s = adjList[i]; 
    int bucket = sorter->queryBucket( s );       // get current bucket of snode s
    if ( bucket == -1 ) {                        // if bucket == -1, 
      // item is already removed from sorter
      // probably already merged with something else
      continue;                                  //    go to next snode
    }
    
    int lastTargetScattered = -1;                // 
    int stamp = nextStamp();
    int targetNode;
    // while there exists a node in the current bucket
    while (  ( targetNode = sorter->queryFirstItem( bucket ) ) != -1 ) {
      sorter->remove( targetNode );  // remove that item
      if ( parent[ targetNode ] != targetNode ) { // if item is non-principal
	continue;                                 // skip it and continue;
      }
      // for all other nodes in the same bucket
      int candidateNode = sorter->queryFirstItem( bucket );
      while ( candidateNode != -1 ) { 
	bool possibleMatch = true;   // run through series of tests eliminate possibilities
	// first see if adjList sizes match
	if ( ( nEnodes[ candidateNode ] != nEnodes[ targetNode ] ) ||
	     ( nSnodes[ candidateNode ] != nSnodes[ targetNode ] ) ) {
	  possibleMatch = false;
	} 
	if ( parent[ candidateNode]  != candidateNode ) { 
	  possibleMatch = false;
	}
	if ( possibleMatch == true ) { 
	  // Check the adjacency lists.
	  // Unf. since the lists are not sorted, so we mark
	  // all the nodes adj to targetNode.  Then check if the
	  // candidate hits all the same marks.
	  if ( lastTargetScattered != targetNode ) {  
	    // if we haven't marked this one yet
	    stamp = nextStamp();                      //    get new timestamp
	    for( register int j = adjHead[ targetNode ], 
		   stop_j = adjHead[ targetNode ] + nSnodes[ targetNode ] 
		                                  + nEnodes[ targetNode ];
		 j < stop_j; ++j ) {
	      visited[ adjList[ j ] ] = stamp;
	    }
	    lastTargetScattered = targetNode;
	  }
	  // Now we are ready to check the candidate node
	  {
	    for( register int j = adjHead[ candidateNode ], 
		   stop_j = adjHead[ candidateNode ] + nSnodes[ candidateNode ] 
		   + nEnodes[ candidateNode ];
		 j < stop_j; ++j ) {
	      if ( adjList[j] != candidateNode ) { 
		if ( visited[ adjList[ j ] ] != stamp ) {
		  // test failed.  
		  possibleMatch = false;                 
		  break;
		}
	      }
	    }
	  }
	}
	if ( possibleMatch == false ) { 
	  candidateNode = sorter->queryNextItem( candidateNode );
	} else {
	  // success!  candidateNode can be merged into targetNode
	  TRACE( SPINDLE_TRACE_QGRAPH_UPD, "\n\tCompress snode %d into %d", 
		 candidateNode, targetNode );
	  ++nCompressedNodes;
	  ++totCompressedNodes;
	  removeNodes.push_back( candidateNode );
	  externDeg[ targetNode ] = 
	    max( 0, externDeg[ targetNode ] - weight[ candidateNode ] );
	  weight[ targetNode ] += weight[ candidateNode ];
	  weight[ candidateNode ] = 0;
	  parent[ candidateNode ] = targetNode;
	  // now link up the circular doubly linked list
	  int prev_target = prev[ targetNode ];
	  next[ prev[ targetNode ] ] = candidateNode;
	  prev[ targetNode ] = prev[ candidateNode ];
	  next[ prev[ candidateNode ] ] = targetNode;
	  prev[ candidateNode ] = prev_target;
	  // clean up the space
	  int z = candidateNode;
#if ( SPINDLE_QGRAPH_REPACK_AGGRESSIVE > 0 )
	  for( int k = adjHead[ z ], stop_k = adjHead[ z ]+nEnodes[ z ]; 
	       k < stop_k; ++k ) { 
	    adjList[ k ] = -1;
	  }
#endif
	  nEnodes[ z ] = 0;
	  nSnodes[ z ] = 0;
	  adjHead[ z ] = -1;
	  // now handle some details with the bucketSorter
	  int nodeToRemove = candidateNode;                      // remember matched node
	  candidateNode = sorter->queryNextItem( nodeToRemove ); // find next candidate
	  sorter->remove( nodeToRemove );                        // remove prev candidate
	} // end if possibleMatch == true
      } // end for all other nodes in same bucket
    } // end while there exists a node in the current bucket
  } // end for all nodes adjacent to me

  FEXIT;
}
#undef __FUNC__



#define __FUNC__ "bool QuotientGraph::pruneNonPrincipalSupernodes();"
void
QuotientGraph::pruneNonPrincipalSupernodes( VertexList& updateNodes ) {
  FENTER;
 
  register const int * const restrict adjHead = adjHead_;
  register       int * const restrict adjList = adjList_;
  register       int * const restrict nSnodes = nSnodes_;
  register const int * const restrict parent = parent_;
  
  ////////////////////////////////
  //
  // Prune non-principal supernodes from me
  //
  int me = lastEliminatedNode.back();
  int head = adjHead[ me ];

  int dest = head;
  int mySNodes = nSnodes[ me ];
  for ( int i = head, stop_i = head + mySNodes; i<stop_i; ++i ) {
    int s = adjList[i]; 
    if ( parent[s] == s ) {       // if s is principal snode
      adjList[ dest ] = s;        //  ... copy it
      ++dest;
      updateNodes.push_back( s ); //  ... and add to updateNodes
    } else if ( parent[s]<0  ) {  // if s is outmatched
      adjList[ dest ] = s;        //  ... copy it 
      ++dest;                     
    }
  }
  nSnodes[ me ] = dest - head;
  
#if ( SPINDLE_QGRAPH_REPACK_AGGRESSIVE > 0 ) 
  // mark remaining space, if any, as usable
  for ( int i = dest, stop_i = head+mySNodes; i<stop_i; ++i ) {
    adjList[ i ] = -1;
  }
#endif

  if ( head+mySNodes == freeSpace ) { // if my snode list was at the end of the freespace
    freeSpace = dest;                 //    freeSpace starts at end of my pruned list
  }
  FEXIT;
}
#undef __FUNC__



#define __FUNC__ "bool QuotientGraph::resetUPdateList()"
void
QuotientGraph::resetUpdateList() {
  FENTER;
  int * const updateList = updateList_;

  ////////////////////////
  //
  // Reset updateList
  //
  register int s = updateList[n];
  while( s < n ) { 
    register int temp = updateList[ s ];
    updateList[ s ] = -1;
    s = temp;
  }
  updateList[ n ] = n;
  lastEliminatedNode.resize(0);

  FEXIT;
}
#undef __FUNC__



#define __FUNC__ "void QuotientGraph::xferNewEnodesToEnodeListsOfReachSet();"
void
QuotientGraph::xferNewEnodesToEnodeListsOfReachSet() {
  FENTER;

  register const int * const restrict adjHead = adjHead_;
  register 	 int * const restrict adjList = adjList_;
  register 	 int * const restrict nEnodes = nEnodes_;
  register 	 int * const restrict nSnodes = nSnodes_;
  register const int * const restrict updateList = updateList_;

  ////////////////////////////////
  //
  // Transfer new enodes from snode lists into enode lists
  // 

  // forall reachable nodes, r, requiring degree update
  for ( int r = updateList[ n ]; r < n ; r = updateList[ r ] ) {
    // forall snodes, s, in adj(r)
    int head = adjHead[ r ];
    int snode_begin = head + nEnodes[ r ];
    int snode_end = snode_begin + nSnodes[ r ];
    for ( int i = snode_begin, stop_i = snode_end; i < stop_i; ++i ) { 
      int s = adjList[ i ]; 
      if ( nEnodes[ s ] < 0 ) { // implies s was eliminated at (n+nEnodes[s])
	std::swap( adjList[i], adjList[ snode_begin ] ); // swap current with first snode
	++snode_begin;                              // advance snode to enode_list
      }
    } // end forall snodes, s, in adj(r)
    //ASSERT(nEnodes[s]>=0,"clobbering permutation");
    nEnodes[ r ] = snode_begin - head;      // write back number of enodes
    ASSERT( nEnodes[r] >= 0,"Cannot have a negative number of enodes");
    nSnodes[ r ] = snode_end - snode_begin; // write back number of snodes
    ASSERT( nSnodes[r] >= 0,"Cannot have a negative number of enodes");
  } // end for all snodes, s, requiring degree update

  FEXIT;
}
#undef __FUNC__



void
QuotientGraph::cleanEnodes() {
  register const int * const restrict adjHead = adjHead_;
  register 	 int * const restrict adjList = adjList_;
  register const int * const restrict updateList = updateList_;
  register 	 int * const restrict nEnodes = nEnodes_;
  register const int * const restrict nSnodes = nSnodes_;
  register const int * const restrict parent = parent_;
  register 	 int * const restrict visited = visited_;

  for ( int r = updateList[n];  r < n ; r = updateList[ r ] ) {
    // for all reachable nodes in the update list
    const int head = adjHead[ r ];
    int snode_begin = head + nEnodes[ r ];
    int snode_end = snode_begin + nSnodes[ r ];
    int cur = head;
    const int stamp = nextStamp();
    while ( cur < snode_begin ) {      // while there are enodes
      int e = adjList[ cur ];          //   get next adj enode
      if ( e != r ) {                  // if enode is not self loop
	while( parent[ e ] != e ) {    //   advance to principal enode 
	  ASSERT( parent[ e ] >= 0, "parent of enode should not be negative!\n");
	  e = parent[ e ];             // 
	}
	adjList[ cur ] = e;            //   write back
      }
      if ( visited[ e ] < stamp ) {    // if not visited
	visited[ e ] = stamp;          //   stamp it
	++cur;                         //   save it
      } else { //   else if not principal
	// get rid of it
	// shift enode from end of enode list to current.
	adjList[ cur ] = adjList[ --snode_begin ];
	// shift snode list from end of snodes to end of enodes
	adjList[ snode_begin ] = adjList[ --snode_end ];
	// flag old space as available
	adjList[ snode_end ] = -1;
      } 
    }
    nEnodes[ r ] = snode_begin - head;
  }
}

void QuotientGraph::cleanSnodes() {
  register const int * const restrict adjHead = adjHead_;
  register 	 int * const restrict adjList = adjList_;
  register const int * const restrict updateList = updateList_;
  register const int * const restrict nEnodes = nEnodes_;
  register 	 int * const restrict nSnodes = nSnodes_;
  register const int * const restrict parent = parent_;
  register 	 int * const restrict visited = visited_;

  for ( int r = updateList[n];  r < n ; r = updateList[ r ] ) {
    // for all reachable nodes in the update list
    const int head = adjHead[ r ];
    const int snode_begin = head + nEnodes[ r ];
    int snode_end = snode_begin + nSnodes[ r ];
    int cur = head;
    const int stamp = nextStamp();

    // first flag all the enodes of r
    for ( ;cur < snode_begin;++cur ) {                // while there are enodes
      int e = adjList[ cur ];
      visited[ e ] = stamp;
    }
    
    // now check if we should keep the snode
    while ( cur < snode_end ) {                  // while there are snodes
      int s = adjList[ cur ];                    //   get next adj snode
      bool keepit = true;                        //     
      if ( parent[s] != s && parent[s]>0) {      //   if it is not principal and not outmatched
	keepit = false;
      } else {                                   //   else if it is principal
	int iter = adjHead[ s ];
	const int stop = iter + nEnodes[ s ];
	for (;iter < stop; ++iter ) {                //     for all enodes of s
	  if ( visited[ adjList[ iter ] ] == stamp) {//       if its flagged (ie common enode with r)
	    keepit = false;
	    break;
	  }
	}
      }
      if ( keepit ) { 
	++cur;
      } else { 
	adjList[ cur ] = adjList[ --snode_end ]; //     shift in new one from end
	adjList[ snode_end ] = -1;               //     mark old space as available
      }
    }
    nSnodes[ r ] = snode_end - snode_begin;
  }
}

void
QuotientGraph::sortReachSet( int &adj1Head, int &adj2Head, int &adjNHead ) { 
  register const int * const restrict nEnodes = nEnodes_;
  register const int * const restrict nSnodes = nSnodes_;
  register 	 int * const restrict updateList = updateList_;

  adj1Head = n; // "n" is used as a flag for the end of the list
  adj2Head = n;
  adjNHead = n;
  
  int r = updateList[ n ]; 
  while ( r < n ) { 
    //
    // Second: add item to corresponding list
    //
    int next = updateList[ r ];     // store next item in list
    if ( nEnodes[ r ] == 2 ) { 
      // if nEnodes == 2 ( myself and one other )
      // put "r" on adj1Head list
      updateList[ r ] = adj1Head;
      adj1Head = r;
    } else if ( nEnodes[ r ] == 3  && nSnodes[ r ] == 0 ) { 
      // if nEnodes == 3 (myself + two others)
      // put "r" on adj2Head list
      updateList[ r ] = adj2Head;
      adj2Head = r;
    } else { 
      // put "r" on adjNHead list
      updateList[ r ] = adjNHead;
      adjNHead = r;
    }
    r = next; 
  } // end for all reachable snoder, s, that require a degree update
  FEXIT;
}

#define __FUNC__ "void QuotientGraph::stripOldEntriesFromAdjListsOfReachSet();" 
void 
QuotientGraph::stripOldEntriesFromAdjListsOfReachSet(int &adj2Head, int &adjNHead ) {
  FENTER;
  register const int * const restrict adjHead = adjHead_;
  register 	 int * const restrict adjList = adjList_;
  register 	 int * const restrict nEnodes = nEnodes_;
  register 	 int * const restrict nSnodes = nSnodes_;
  register const int * const restrict weight = weight_;
  register 	 int * const restrict externDeg = externDeg_;
  register 	 int * const restrict visited = visited_;
  register 	 int * const restrict updateList = updateList_;
  
  ////////////////////////////////
  //
  // 1. Strip off outdated entries in Enode and Snode lists.
  // 2. Separate updateList into two stacks:
  //    One of all snodes with two one headed by adj2Head and one by adjNHead
  //    Note: updateList is used for both stacks
  //
  adj2Head = n; // "n" is used as a flag for the end of the list
  adjNHead = n;
 
  // for all reachable nodes, r, that require a degree update
  int r = updateList[ n ]; 
  while ( r < n ) { 
    
    //
    // First strip off all the unneeded entries
    //
    const int stamp = nextStamp();          // get new timestamp;
    const int head = adjHead[ r ];          // start of enode list
    int snode_begin = head + nEnodes[ r ];  // start of snode list
    int snode_end = snode_begin + nSnodes[ r ]; // end of snode list
    const int end = snode_end;
    register int cur = head;                      
    register int degree = 0;
    visited[ r ] = stamp;             // flag myself
    while( cur < snode_begin ) {      // while there are enodes
      register int e = adjList[ cur ];//    get the next one
      if ( e == r ) {                 //    if this is a self-enode
 	 ++cur;                        //      advance to next item
      } else {                        //    else if not self-enode
	//	while( parent[ e ] != e ) {   //      while not principal
	//	  e = parent[ e ];            //        get its parent
	//	}
	//	adjList[ cur ] = e;           //        copy it
	if ( visited[ e ] < stamp ) { //      if not yet visited
	  visited[ e ] = stamp;       //        mark it 
	  ++cur;                      //        advance to next item
	  // for all snodes, z, adj to enode e....
	  for ( register int j = adjHead[ e ] ,
		  stop_j = adjHead[ e ] + nSnodes[ e ];
		j < stop_j; ++j ) {
	    const int z = adjList[ j ];
	    degree += ( visited[ z ] < stamp ) ? weight[ z ] : 0 ;
 	     visited[ z ] = stamp;
	  }
	} else { // if visited
	  // shift enode from end of enode list to current.
	  adjList[ cur ] = adjList[ --snode_begin ];
	  // shift snode list from end of snodes to end of enodes.
	  adjList[ snode_begin ] = adjList[ --snode_end ];
	  // flag old space as available
	  adjList[ snode_end ] = -1;
	}// end else if visited
      } // end else if e != s
    } // end while src < snode_begin
    nEnodes[ r ] = snode_begin - head;
    ASSERT( nEnodes[ r ] >= 0, "Cannot have a negative number of enodes");
    
    if ( (nEnodes[r]) > 1 && incrementByOne ) ++degree;
    
    while( cur < snode_end ) {      // while there are snodes
      int s = adjList[ cur ];
      if ( visited[ s ] < stamp ) { // if not yet stamped
	visited[ s ] = stamp;       //   it is now
	//	if ( ( parent[ s ] == s ) || ( parent[ s ] < 0 ) ) { 
	// if its a principal snode or outmatched
	degree += weight[ s ];        // add it to degree
	++cur;                        // go to next one
	//	} else { // if snode is compressed.
	//	  adjList[ cur ] = adjList[ --snode_end ];    // shift in new one from end
	//	  adjList[ snode_end ] = -1;                  // mark old space as available
	// }
       } else { // if already stamped
 	 adjList[ cur ] = adjList[ --snode_end ]; // shift in new one from end
 	 adjList[ snode_end ] = -1;               // mark old space as available
       }
       // NOTE:
       // if snode is not principal, it must be adjacent to one that is, 
       // therefore no need to chase up parent[] pointers to find it.
     } // end while src < snode_end
     nSnodes[ r ]  = snode_end - snode_begin;
     externDeg[ r ] = degree;
 
 #if ( SPINDLE_QGRAPH_REPACK_AGGRESSIVE > 0 ) 
     while ( cur < end ) { // mark remaining space as unused.
       adjList[ cur ] = -1;
       ++cur;
     }
 #endif
 
     //
     // Second: add item to corresponding list
     //
     int next = updateList[ r ];     // store next item in list
     if ( nEnodes[ r ] == 3  && nSnodes[ r ] == 0 ) { 
       // if nEnodes == 3 (myself + two others)
       // put "s" on adj2Head list
       updateList[ r ] = adj2Head;
       adj2Head = r;
     } else { 
       // put "s" on adjNHead list
       updateList[ r ] = adjNHead;
       adjNHead = r;
     }
     r = next; 
   } // end for all snodes, s, that require a degree update
 
   FEXIT;
 }
 #undef __FUNC__





#define __FUNC__ "void QuotientGraph::compressAndOutmatchReachSet( "\
                 "int adj2Head, VertexList &removeNodes ) ;"
void 
QuotientGraph::compressAndOutmatchReachSet( int adj2Head, VertexList &removeNodes ) {
  FENTER;
  
  register 	 int * const restrict adjHead = adjHead_;
  register 	 int * const restrict adjList = adjList_;
  register 	 int * const restrict nEnodes = nEnodes_;
  register 	 int * const restrict nSnodes = nSnodes_;
  register 	 int * const restrict weight = weight_;
  register 	 int * const restrict externDeg = externDeg_;
  register 	 int * const restrict visited = visited_;
  register 	 int * const restrict parent = parent_;
  register 	 int * const restrict next = next_;
  register 	 int * const restrict prev = prev_;
  register const int * const restrict updateList = updateList_;

  ////////////////////////////////////////////
  //
  // Compression and Outmatching
  //
  
  // for each snode in adj2Head
  for ( int s = adj2Head; s < n; s = updateList[ s ] ) {
    if ( parent[ s ] != s ) {               // if this snode has already been compressed
      continue;                             // go to next snode
    }
    int head = adjHead[ s ];                // start of enode list
    int snode_begin = head + nEnodes[ s ];  // start of snode list
    bool marked_one_enode = false;
    
    // for all enodes, e, adjacent to snode s
    for ( int i = head, stop_i = snode_begin; i < stop_i; ++i ) { 
      int stamp; // used later
      int e = adjList[ i ];
      if ( e == s ) {          // if skip self edges
	;
      } else if ( marked_one_enode == false ) {  // else if we found our first enode
	// stamp all snodes adj( e ) 
	stamp = nextStamp();
	
	// for all snodes, z,  adjacent to enode, e
	for ( int j = adjHead[ e ] , stop_j = adjHead[ e ] + nSnodes[ e ];
	      j < stop_j; ++j ) { 
	  int z = adjList[ j ];
	  visited[ z ] = stamp;
	} // end forall snodes, z, adj ( e )
	marked_one_enode = true;                 // now we have marked our first enode
      } else if ( marked_one_enode == true ) {   // if we are on our second enode...
	// for all snodes, z,  adjacent to enode, e
	for ( int j = adjHead[ e ] , stop_j = adjHead[ e ] + nSnodes[ e ];
	      j < stop_j; ++j ) { 
	  int z = adjList[ j ];
	  if ( z != s && visited[z] == stamp ) {  
	    // if this snode was visited via other enode
	    if ( nEnodes[ z ] == 3  && nSnodes[ z ] == 0 ) {  
	      // if this snode also has only two adj enodes
	      // merge z into s
	      if ( parent[ z ] < 0 ) { // was outmatched
		--nOutmatchedNodes;
	      }
	      TRACE( SPINDLE_TRACE_QGRAPH_UPD, "\n\tCompress snode %d into %d", 
		     z, s);
	      ++nCompressedNodes;
	      ++totCompressedNodes;
	      removeNodes.push_back( z );
	      externDeg[ s ] -= weight[ z ];
	      externDeg[ s ] = (externDeg[s]<0)? 1 : externDeg[s];
	      weight[ s ] += weight[ z ];
	      weight[ z ] = 0;
	      parent[ z ] = s;
	      // now link up the circular doubly linked list
	      int prev_s = prev[ s ];
	      next[ prev[ s ] ] = z;
	      prev[ s ] = prev[ z ];
	      next[ prev[ z ] ] = s;
	      prev[ z ] = prev_s;
#if ( SPINDLE_QGRAPH_REPACK_AGGRESSIVE >  0 )
	      // finally clean up the space
	      for( int k = adjHead[ z ], stop_k = adjHead[ z ]+nEnodes[ z ]; 
		   k < stop_k; ++k ) { 
		adjList[ k ] = -1;
	      }
#endif
	      nEnodes[ z ] = 0;
	      nSnodes[ z ] = 0;
	      adjHead[ z ] = -1;
	    } else if ( parent[ z ] == z ) { 
	      // z is outmatched by s
	      parent[ z ] = s - n;
	      ++nOutmatchedNodes;
	      ++totOutmatchedNodes;
	    } // end else if z is outmatched by s
	  } // end if z != s && visited[z] == stamp
	} // end forall snodes z adj( e );
      } // end if second adjacent enode;
    } // end forall enodes, e, adj to snode s
  } // end for each item in adj2Head

  FEXIT;
}
#undef __FUNC__



#define __FUNC__ "bool QuotientGraph::resetReachSet(int adj2Head, int adjNHead,"\
                 "VertexList& updateNodes );"
void 
QuotientGraph::resetReachSet( int adj1Head, int adj2Head, int adjNHead, VertexList &updateNodes ) {
  FENTER;

  register const int * const restrict parent = parent_;
  register 	 int * const restrict updateList = updateList_;
  register const int * const restrict adjHead = adjHead_;
  register const int * const restrict adjList = adjList_;
  register const int * const restrict weight = weight_;
  register const int * const restrict nEnodes = nEnodes_;
  register const int * const restrict nSnodes = nSnodes_;
  register 	 int * const restrict visited = visited_;
  register 	 int * const restrict externDeg = externDeg_;

  /////////////////////////
  // 
  // first compute degree and reset adj1Head list
  //
  int r = adj1Head;
  int degree = 0;
  while( r < n ) { 
    resetReachSetTimer[0].start();
    int temp = updateList[ r ];
    updateList[ r ] = -1;
    if ( parent[ r ] == r ) { 
      degree = weight[ adjList[ adjHead[ r ] + 1 ] ];
      degree -= weight[ r ];
      // NOTE: iterate over snodes
      int cur = adjHead[ r ] + nEnodes[ r ];
      const int stop = cur + nSnodes[ r ];
      for( ; cur != stop; ++cur ) { 
	degree += weight[ adjList[ cur ] ];
      }
      externDeg[ r ] = degree;
      updateNodes.push_back( r );
    }
    r = temp;
    resetReachSetTimer[0].stop();
  }
  
  ////////////////////////
  //
  // Reset updateList
  //
  r = adj2Head;
  while( r < n ) { 
    resetReachSetTimer[1].start();
    int temp = updateList[ r ];
    updateList[ r ] = -1;
    if ( parent[ r ] == r ) {
      const int enode1 = adjList[ adjHead[ r ] + 1 ];
      const int enode2 = adjList[ adjHead[ r ] + 2 ];
      const int stamp = nextStamp();
      degree = weight[ enode1 ];
      degree += weight[ enode2 ];
      // now flag all snodes adj to enode1
      {
	// NOTE: iterating over enodes (nEnodes[] is not useful here!)
	int cur = adjHead[ enode1 ];
	const int stop = cur + nSnodes[ enode1 ];
	for( ; cur != stop; ++cur ) { 
	  visited[ adjList[ cur ] ] = stamp;
	}
      }
      // now if any snode of enode2 is flagged, subtract its weight 
      { 
	// NOTE: iterating over enodes (nEnodes is not useful here!)
	int cur = adjHead[ enode2 ];
	const int stop = cur + nSnodes[ enode2 ];
	for( ; cur != stop; ++cur ) { 
	  if ( visited[ adjList[ cur ] ] == stamp ) { 
	    degree -= weight[ adjList[ cur ] ];
	  }
	}
      }
      externDeg[ r ] = degree - weight[ r ];
      updateNodes.push_back( r );
    }
    r = temp;
    resetReachSetTimer[1].stop();
  }

  // FINALLY DO ALL THE OTHERS
  r = adjNHead;
  while( r < n ) { 
    resetReachSetTimer[2].start();
    int temp = updateList[ r ];
    updateList[ r ] = -1;
    if ( parent[ r ] == r ) {
      updateNodes.push_back(r);
      externDeg[ r ] = deg(r); // - weight[ r ]; //deg() is really external degree
    }
    r = temp;
    resetReachSetTimer[2].stop();
  }    
  updateList[ n ] = n;
  lastEliminatedNode.resize(0);
  FEXIT;
}
#undef __FUNC__



int 
QuotientGraph::nextStamp() 
#ifndef mutable 
  const
#endif 
{ 
  if ( (++timestamp) >= LARGEINT ) {
    bzero( (char *) visited_, n*sizeof(int) );
    timestamp = 1;
  }
  return timestamp;
}

// prints out both adjacency lists of the graph
void 
QuotientGraph::printShortGraph( ostream& outstr ) const {

  register const int * const restrict nEnodes = nEnodes_;
  register const int * const restrict setDiff = setDiff_;
  register const int * const restrict externDeg = externDeg_;
  register const int * const restrict parent = parent_;
  register const int * const restrict weight = weight_;

  outstr << "QuotientGraph(short) {" << endl;
  for (int i=0; i<n; ++i) {
    if ( parent[i] == i ) {  // if is principal
      outstr << setw(6) << i << " (" << setw(6) << weight[i] << ") : |<-clq" ;
      if ( totEliminatedNodes < n ) { 
	{for(const int * start=enode_begin(i), *stop=enode_end(i); start<stop;++start) {
	  outstr << setw(6) << " ";
	}}
      }
      outstr << "|<-vtx";
      {for(const int * start=snode_begin(i), *stop=snode_end(i); start<stop;++start) {
	outstr << setw(6) << " ";
      }}
      outstr << "|<-end " << endl;
      
      if ( totEliminatedNodes < n ) { 
	if ( nEnodes[i] < 0 ) { 
	  outstr << setw(5) << "" << setw(5) << "" << setw(6) << setDiff[i] << "  | " ;
	} else { 
	  outstr << setw(5) << "" << setw(6) << externDeg[i] << setw(5) << ""  << "  | " ;
	}
	{for(const int * start=enode_begin(i), *stop=enode_end(i); start<stop;++start) {
	  outstr << setw(6) << *start;
	}}
      } else { 
	outstr << setw(18) << setDiff[i] << "  | ";
      }
      outstr << "    | ";
      {for(const int * start=snode_begin(i), *stop=snode_end(i); start<stop;++start) {
	outstr << setw(6) << *start;
      }}
      outstr << "    | " << endl;
    } else if ( parent[i] < 0 ) {
      outstr << setw(6) << i << setw(12) << "" 
	     << ": ( outmatched by " << parent[i]+n << " )" << endl;
      outstr << setw(6) << "" << setw(12) << "" << "  ||| " << endl;
    } else { 
      outstr << setw(6) << i << setw(12) << "" 
	     << ": ( absorbed by " << parent[i] << " )" << endl;
      outstr << setw(6) << "" << setw(12) << "" << "  ||| " << endl;
    }      
  }
  cout << "}" << endl;
}

// prints out both adjacency lists of the graph
void 
QuotientGraph::printTinyGraph( ostream& outstr ) const {

  register const int * const restrict nEnodes = nEnodes_;
  register const int * const restrict setDiff = setDiff_;
  register const int * const restrict externDeg = externDeg_;
  register const int * const restrict parent = parent_;
  register const int * const restrict weight = weight_;

  outstr << "QuotientGraph(tiny)  {" << endl;
  for (int i=0; i<n; ++i) {
    if ( parent[i] == i ) {  // if is principal
      outstr << setw(5) << i << " (" << setw(6) << weight[i] << ") : |<-clq" ;
      if ( totEliminatedNodes < n ) { 
	{for(const int * start=enode_begin(i), *stop=enode_end(i); start<stop;++start) {
	  outstr << setw(5) << " ";
	}}
      }
      outstr << "|<-vtx";
      {for(const int * start=snode_begin(i), *stop=snode_end(i); start<stop;++start) {
	outstr << setw(5) << " ";
      }}
      outstr << "|<-end " << endl;
      
      if ( totEliminatedNodes < n ) { 
	if ( nEnodes[i] < 0 ) { 
	  outstr << setw(5) << "" << setw(5) << "" << setw(5) << setDiff[i] << "  | " ;
	} else { 
	  outstr << setw(5) << "" << setw(5) << externDeg[i] << setw(5) << ""  << "  | " ;
	}
	{for(const int * start=enode_begin(i), *stop=enode_end(i); start<stop;++start) {
	  outstr << setw(5) << *start;
	}}
      } else { 
	outstr << setw(15) << setDiff[i] << "  | ";
      }
      outstr << "    | ";
      {for(const int * start=snode_begin(i), *stop=snode_end(i); start<stop;++start) {
	outstr << setw(5) << *start;
      }}
      outstr << "    | " << endl;
    }      
  }
  cout << "}" << endl;
}

int 
QuotientGraph::defragAdjList() { 
  FENTER;

  register       int * const restrict adjHead = adjHead_;
  register       int * const restrict adjList = adjList_;
  register const int * const restrict parent = parent_;
  register const int * const restrict nSnodes = nSnodes_;
  register const int * const restrict nEnodes = nEnodes_;

  // defragments memory in adjList
  // returns pointer to first unused location in adjList

  // we must repack the data, but the heads of the lists
  // are not stored monotonically, so we must repack the data
  // in its current order to avoid overwriting data
  typedef vector< pair< int, int> > SorterType;
  static SorterType vecSorter; // static data in a function... its constructed once 
  vecSorter.reserve(n); // reserve memory for at least n
  vecSorter.resize(0);  // reset to zero

  {for ( int i=0; i<n; ++i ) { // for all vertices and cliques...
    if ( parent[i]==i || parent[i]<0 ) { // if its principal snode, enode, or outmatched
      vecSorter.push_back( pair< int , int >( adjHead[i], i ) );
      // insert the list_head and index into vecSorter
    }
  }}
  // now sort the pairs.
  sort( vecSorter.begin(), vecSorter.end(), pair_first_less<int,int>() );

  int src = 0;  // where to copy _from_ in adjList array 
  int dest = 0; // where to copy _to_ in adjList array
  for (SorterType::iterator cur=vecSorter.begin(), stop = vecSorter.end(); 
       cur != stop; ++ cur ) {
    // for all items in the vecSorter  ( which are all principal items )
    int i = (*cur).second; 
    src = adjHead[i];                      //     get the old location
    int nItems = (nEnodes[i]<0) ? 
      nSnodes[i] : nEnodes[i] + nSnodes[i];//     get the number of ints to move
    if ( (src!=dest) && (nItems>0) ) {     //     if there is data to move
      memcpy( adjList + dest, adjList + src, sizeof(int) * nItems );  // move it
      // NOTE: memmove is guaranteed to work correctly when regions overlap,
      //       we are always copying to lower memory, so this should always work too
      adjHead[i] = dest;       // adjust head of adjList
    }
    dest += nItems; // increment to next available dest
  } 
  freeSpace = dest;
  return dest;
}


