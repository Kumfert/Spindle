//
// QuotientGraph.h -- a quotient graph with "elbow room"
//
// $Id: QuotientGraph.h,v 1.2 2000/02/18 01:31:46 kumfert Exp $
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

#ifndef SPINDLE_QUOTIENT_GRAPH_H_
#define SPINDLE_QUOTIENT_GRAPH_H_

#ifndef SPINDLE_SYSTEM_H_
#include "spindle/SpindleSystem.h"
#endif

#ifdef REQUIRE_OLD_CXX_HEADER_SUFFIX
#include <iostream.h>
#include "pair.h"
#include "vector.h"
#include "set.h"
#include "list.h"
#include "iterator.h"
#else
#include <iostream>
#include <utility>  // pair
#include <vector>
#include <set>
#include <list>
#include <iterator>
using namespace std;
#endif

#ifndef SPINDLE_ARRAY_BUCKET_SORTER_H_
#include "spindle/ArrayBucketSorter.h"
#endif

#ifndef SPINDLE_GRAPH_H_
#include "spindle/Graph.h"
#endif

#ifndef LARGEINT
#  ifdef MAXINT
#    define LARGEINT MAXINT
#  else
#    define LARGEINT 2000000000
#  endif
#endif

SPINDLE_BEGIN_NAMESPACE

class QuotientGraph { 

public:
  typedef ArrayBucketSorter BucketSorter;
  typedef vector<int> VertexList;

private:
  int lastENode;
  bool incrementByOne;// strange behavior???
  int n;              // the size of the symmetric matrix
  int maxint;         // the largest positive integer
  int * adjList_;     // the adjacency list of enodes and snodes (should be at least nnz + n )
  int freeSpace;      // the first index into adjList that is unused.
  int maxSpace;       // the total size of adjList;
  int * adjHead_;     // index into adjList.
  // NOTES:
  // adjHead[] < 0 implies that the node was compressed.
  // parent[] array should be used to advance to the principal node
  int * parent_;      // points to node or element that absorbs this
  // NOTES:
  // + parent[i] == i implies node i is principal node
  // + parent[i] < 0  implies that i is outmatched by n+parent[i]
  // + parent[i] >=0 && != i implies that i is non-principal
  int * nEnodes_;      // number of eliminated nodes
  // NOTES:
  // + Initialized to all ones since every snode has at least a enode of itself.
  // + nEnodes[i]<0 implies that i is an enode and was eliminated at step (-nEnodes[i]+1).
  int * nSnodes_;      // number of supernodes
  int * weight_;       // weight of each snode and enode
  int * setDiff_;      // array for set differences ( for single elimination )
                       // list of eliminated snodes ( for multiple elimination )
  int * externDeg_;    // array for external degree of vertices
  int * next_;         // next item in circular doubly linked list
  int * prev_;         // prev item in circular doubly linked list
  // NOTE:
  // only one item in each list has the property parent[i] == i.
  // all other items in each circle has the property that parent[j] != j
  // and that going up through parents, you will eventually get to the one that
  // is also the head of the list.

  int * updateList_;   // single list of snodes that requires updating.
  // NOTE:
  // + length = n+1.  
  // + updateList[n] == n implies list is empty
  // + updateList[n] == j (0<=j<n) implies j is the head of the list
  // + updateList[i] < 0 implies "i" not in list
  // + updateList[i] == n implies "i" is last in list
  // + updateList[i] == j (0<=j<n) implies "i" is in list
  // + A snode that requires updating cannot be eliminated.  

  int totEliminatedNodes;   // total number of negative entries in nEnodes;
  int nEliminatedNodes;     // number of principal enodes eliminated
  int totCompressedNodes;   // total number of snodes compressed
  int nCompressedNodes;     // current number of non-eliminated snodes 
  int totOutmatchedNodes;   // total number of times an snode was outmatched by another
  int nOutmatchedNodes;     // current number of outmatched snodes
  int nEliminatedLastStage; // number of snodes eliminated in last stage
  // NOTE:
  // totEliminated >= nEliminated
  // totCompressed >= nCompressed
  // totOutmatched >= nOutmatched
  // nEligible = n - totCompressedNodes - nEliminatedNodes
  int nDefrags;             // number of times adjList array is defragmented

  bool isSingleElimination;
  //  bool computeSetDiffs;
  //  bool computeExtDeg;
  vector<int> lastEliminatedNode;

  BucketSorter * sorter;

  void initialize( const int nVertices, const int *AdjHead, const int *AdjList, 
		   const int *vwgt );

  // This is a possible subroutine of eliminateSupernode()
  int defragAdjList();

  // The following are subroutines of update for single elimination
  void computeSetDiffs( const int stamp );
  void stripOldEntriesAndPackIntoBucketSorter( const int stamp );
  void scanBucketSorterForSupernodes( VertexList& removeNodes );
  void pruneNonPrincipalSupernodes( VertexList& updateNodes );
  void resetUpdateList( );

  // The following are subroutines of update for multiple elimination 
  void xferNewEnodesToEnodeListsOfReachSet() ;
  // void qgraph_stop_here();
  void cleanEnodes();
  void cleanSnodes();
  void sortReachSet( int &adj1Head, int &adj2Head, int &adjNHead );
  void stripOldEntriesFromAdjListsOfReachSet( int &adj2Head, int &adjNHead ) ;
  void compressAndOutmatchReachSet( int adj2Head, VertexList &removeNodes ) ;
  void resetReachSet(int adj1Head, int adj2Head, int adjNHead, VertexList &updateNodes );

public:
  // constructor: uncompressed
  QuotientGraph( const Graph* graph );
  //  QuotientGraph( const int nVertices, const int *AdjHead, const int *AdjList );

  ~QuotientGraph();
  /*  
  int queryNEliminatedNbrs(int i) { return (i==0)? 1 : i ; } // HACK
  void reset() { }
  */
  int queryNEliminated() const { return nEliminatedNodes; }
  int queryNCompressed() const { return nCompressedNodes; }
  int queryNOutmatched() const { return nOutmatchedNodes; }
  int queryTotEliminated() const { return totEliminatedNodes; }
  int queryTotCompressed() const { return totCompressedNodes; }
  int queryTotOutmatched() const { return totOutmatchedNodes; }
  int queryLastEliminatedNode() const { return lastENode; }

  int queryNEliminatedLastStage() const { return nEliminatedLastStage; }

  const int * getOld2New() const;
  // returns 0 if totEliminatedNodes != n
  // else returns -(nEnodes+1);
  
  bool eliminateSupernode( const int i );
  // void purgeVtxs( const vector< pair< int, int> >& vtxs2purge );
  // bool eliminatePurgedVtxs();
  
  // returns false if no vertices have been eliminated
  // resets updateNodes and removeNodes to empty.
  // updates QuotientGraph
  // puts all current nodes that need a priority update in updateNodes
  // puts all nodes that have been outmatched or merged in removeNodes
  bool update( VertexList& updateNodes, VertexList& removeNodes );

  // set some details for compression options.
  // bool setMixAdjCompression( const int option ); 
  // -1=class decide, 0=override to no, 1=override yes
  //bool setNAdjClqCompressionBound(const int bound ); 
  // -1=class decide, 0=no compression, 2 = 2adj only,

  // Returns which step the vertex was eliminated at.  NOTE:
  // This number may not be what was expected since the quotient
  // graph handles indistinguishable vertices.
  int eliminatedAt( const int vtx_ ) const;

  // Returns true if the vertex (or element) is a principal vertex.
  // Returns false if argument is out of range, or vertex/element has
  // been merged to something else.
  bool isPrincipal( const int vtx_ ) const; 

  // Returns the parent of a merged(compressed) element(vertex)
  int queryParent( const int vtx_ ) const; 
  const int* getParentArray() const;

  // Returns the weight of a principal vertex/element 
  // for a principal vertex, weight is the size of the inode list
  // for a principal element, weight is the sum of the element
  //      weights it has absorbed, 
  //     each element starts with weight == its weight as vertex
  int queryWeight( const int vtx_ ) const ;
  const int* getWeightArray() const;
  const int* getExternDegreeArray() const;
  const int* getSetDiffsArray() const;
  bool enableSingleElimination();
  bool enableMultipleElimination();

  void enableIncrementByOne() { incrementByOne = true; }
  void disableIncrementByOne() { incrementByOne = false; }
  
  // Returns true if the vertex has been eliminated, false otherwise
  bool isEliminated( const int vtx_ ) const ;

  // Returns true iff the vertex cannot be eliminated until the graph is updated.
  bool notEligible( const int vtx_ ) const ;

  // prints out both adjacency lists of the graph
  void printShortGraph( ostream& outstr = cout ) const;
  void printTinyGraph( ostream& outstr = cout ) const;
  
  // prints out supernode information
  // void printLongGraph( ostream& outstr = cout );
  
protected:
  // used for timestamping mechanism
#ifndef mutable
  // mutable is a recognized keyword
  int nextStamp() const;
#else
  // mutable is not recognized so it is defined as an empty macro
  int nextStamp();
#endif
  mutable int timestamp;
  int * visited_;

private:

  // compute degrees of a vertex using various approximations.
  // vtx is assumed to be in a valid range.  
  VertexList delayedVtxs;
  VertexList reachableNeighbors;
  vector< pair<int,int> >purgedVtxs;  // vertices that are removed until the end

  bool singleEliminationUpdate( VertexList& updateNodes, VertexList& removeNodes );
  bool multipleEliminationUpdate( VertexList& updateNodes, VertexList& removeNodes );

  // three phases.
  //  0. construct new enode from snode and adjacent enodes
  //  1. number enode and update elimination forest
  //  2. add all snodes adj to new enode to updateList
  stopwatch eliminateSupernodeTimer[3];

  stopwatch updateTimer;
  // five stages for single elimination
  stopwatch singleUpdateTimer[5];
  // four sub-stages for multiple elimination
  stopwatch multipleUpdateTimer[4];
  // three sub-stages for reseting the reachable set in multiple elimination
  stopwatch resetReachSetTimer[3];

  // times one-time costs
  // 0. initialization
  // 1. finalization
  stopwatch initFinalizeTimer[2];
public:
  const stopwatch* lendInitFinalizeTimer() const { return initFinalizeTimer; }
  const stopwatch* lendEliminateSupernodeTimer() const { return eliminateSupernodeTimer; }
  const stopwatch& lendUpdateTimer() const { return updateTimer; }
  const stopwatch* lendSingleUpdateTimer() const { return singleUpdateTimer; }
  const stopwatch* lendMultipleUpdateTimer() const { return multipleUpdateTimer; }
  const stopwatch* lendResetReachSetTimer() const { return resetReachSetTimer;}
  
  const int* enode_begin( const int i ) const ;
  const int* enode_end( const int i ) const ;
  const int* snode_begin( const int i ) const ;
  const int* snode_end( const int i ) const ;
  int deg( const int i ) const ;

  const int queryNCompressedNodes() const { return nCompressedNodes; }
  const int queryNOutmatchedNodes() const { return nOutmatchedNodes; }
  const int queryNDefrags() const { return nDefrags; }

};

//
// some documentation is helpful
// update()
//    if isSingleElimination
//	 computeSetDiffs()
//	 stripOldEntriesAndPackIntoBucketSorter()
//	 scanBucketSorterForSupernodes()
//	 pruneNonPrincipalSupernodes()
//	 resetUpdateList()
//    else
//	 xferNewEnodesToEnodeListsOfReachSet()
//	 stripOldEntriesFromAdjListsOfReachSet()
//	 compressAndOutmatchReachSet()
//	 resetReachSet()



inline const int* 
QuotientGraph::enode_begin( const int i ) const { 
  if ( i<0 || i>=n || adjHead_[i]<0 ) { 
    return 0;
  } else {
    return adjList_ + adjHead_[i];
  }
}

inline const int*  
QuotientGraph::enode_end( const int i ) const { 
  if ( i<0 || i>=n || adjHead_[i]<0 ) { 
    return 0;
  } else if ( nEnodes_[i]<0 ) {
    return adjList_ + adjHead_[i];
  } else {
    return adjList_ + adjHead_[i] + nEnodes_[i];
  }
}

inline const int* 
QuotientGraph::snode_begin( const int i ) const {
  if ( i<0 || i>=n || adjHead_[i]<0 ) { 
    return 0;
  } else if ( nEnodes_[i]<0 ) {
    return adjList_ + adjHead_[i];
  }else {
    return adjList_ + adjHead_[i] + nEnodes_[i];
  }
}

inline const int*   
QuotientGraph::snode_end( const int i ) const {
  if ( i<0 || i>=n || adjHead_[i]<0 ) { 
    return 0;
  } else if ( nEnodes_[i]<0 ) {
    return adjList_ + adjHead_[i] + nSnodes_[i];
  } else {
    return adjList_ + adjHead_[i] + nEnodes_[i] + nSnodes_[i];
  }
}


inline int 
QuotientGraph::eliminatedAt( const int vtx ) const {
  if ( (vtx < 0) || (vtx >= n) || (nEnodes_[vtx] >= 0 ) ) {
    return -1;
  } else {
    return -(nEnodes_[vtx]+1);
  }
}

inline bool 
QuotientGraph::isPrincipal( const int vtx ) const {
  if ( (vtx < 0) || (vtx >= n) || (adjHead_[vtx] <0) ) {
    return false;
  } else {
    return true;
  }
}

inline bool 
QuotientGraph::notEligible( const int vtx ) const {
  if ( (vtx < 0) || (vtx >= n) ||  (updateList_[vtx]>=0) ) {  
    return true;
  } else {
    return false;
  }
}

inline int 
QuotientGraph::queryParent( const int vtx ) const {
  if ( (vtx < 0) || (vtx >= n) ) { //|| (parent_[vtx]==vtx) ) 
    return -1;
  } else {
    return parent_[ vtx ];
  }
}

inline int 
QuotientGraph::queryWeight( const int vtx ) const {
  if ( (vtx < 0) || (vtx >= n) || (adjHead_[vtx] <0) ) {
    return -1;
  } else {
    return weight_[vtx];
  }
}

inline const int*
QuotientGraph::getWeightArray() const {
  return weight_;
}

inline const int*
QuotientGraph::getExternDegreeArray() const {
  return externDeg_;
}

inline const int*
QuotientGraph::getSetDiffsArray() const { 
  return setDiff_;
}

inline bool
QuotientGraph::enableSingleElimination() {
  if ( !isSingleElimination ) {
    isSingleElimination = true;
    return true;
  }
  return false;
}

inline bool
QuotientGraph::enableMultipleElimination() {
  if ( isSingleElimination ) {
    isSingleElimination = false;
    return true;
  }
  return false;
}

inline const int*
QuotientGraph::getParentArray() const {
  return parent_;
}

inline bool 
QuotientGraph::isEliminated( const int vtx ) const {
  if ( (vtx < 0) || (vtx >= n) || nEnodes_[vtx] >=0 ) {
    return false;
  } else { 
    return true;
  }
}

inline const int * QuotientGraph::getOld2New() const 
{ return (totEliminatedNodes==n) ? nEnodes_ : 0; }

SPINDLE_END_NAMESPACE

#endif
