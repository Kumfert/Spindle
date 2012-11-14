//
// PseudoDiameter.h
// 
// $Id: PseudoDiameter.h,v 1.2 2000/02/18 01:32:03 kumfert Exp $
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
//============================================================================
//
//

#ifndef SPINDLE_PSEUDO_DIAM_H_
#define SPINDLE_PSEUDO_DIAM_H_

#ifndef SPINDLE_SYSTEM_H_
#include "spindle/SpindleSystem.h"
#endif

#ifndef SPINDLE_ALGORITHM_H_
#include "spindle/SpindleAlgorithm.h"
#endif

//#ifndef SPINDLE_BFS_H_
//#include "spindle/BreadthFirstSearch.h"
//#endif

//#ifndef SPINDLE_GRAPH_H_
//#include "spindle/Graph.h"
//#endif

#ifdef REQUIRE_OLD_CXX_HEADER_SUFFIX
#include <vector.h>
#include <pair.h>
#else
#include <vector>
#include <utility>
using std::vector;
using std::pair;
#endif

SPINDLE_BEGIN_NAMESPACE

class Graph; // forward declaration

class BreadthFirstSearch; // forward declaration

class PseudoDiamShrinkingStrategy; // defined later


/**
 * @memo computes pseudo-peripheral nodes on a connected component of a graph
 * @type class 
 *
 *
 * @see PseudoDiamShrinkingStrategy
 * @author Gary Kumfert
 * @version #$Id: PseudoDiameter.h,v 1.2 2000/02/18 01:32:03 kumfert Exp $#
 */
class PseudoDiameter : public SpindleAlgorithm {
public:
  /**
   * Defines partition restrictions to involve strict membership, 
   * or to include nodes adjacent to a partition.
   */
  typedef enum { strict, loose } PartitionMembership; 

protected:
  //This first set of data types are requred to compute the pseudo diameter

  /**
   * Const pointer to graph guarantees that this utility does not modify the 
   * graph its acting on.  However, the graph it points to must exist for
   * the lifetime of this object.
   */
  const Graph * g;
  /** Forward Breadth First Search object */
  BreadthFirstSearch * forwardBFS;

  /** Reverse Breadth First Search object */
  BreadthFirstSearch * reverseBFS;
  
  /** Shrinking strategy */
  PseudoDiamShrinkingStrategy * shrinkingStrategy;

  /** number of vertices in the current graph */
  int nVtxs;

  /** starting point to search for the nodes, -1 lets the class choose itself */
  int root;
  
  /** one end of the pseudo-peripheral pair */
  int startVtx;
  
  /** the other end of the pseudo-peripheral pair */
  int endVtx; 

  /** number of hops from startVtx to endVtx */
  int diameter;         

  /** number of vertices reachable to startVtx and endVtx */
  int nVisited; 

  /** number of times a BFS short-circuited in finding the startVtx, endVtx pair */
  int nShortCircuits;   

  /** number of times a BFS was performed (either forward or reverse) */
  int nBFS;

  /** minimum of all maximum level set widths */
  int minMaxWidth;      

  /** max width of the reverse BFS */
  int revMaxWidth;      

  /** number of times that forward and reverse BFS's (and ergo startVtx) is switched */
  int nFlips;           
  
  /** const pointer to a partition array (optional) */
  const int *partition;        

  /** identity of the current working partition (optional) */
  int curPartition;

  /** determine how vertices are considered members of a partition */
  PartitionMembership partitionMembership;

  /** timer for keeping track of execution time */
  stopwatch timer;

  /** vector to maintain a list of candidate endVtxs */
  vector<int> candidates;
  
  /** does the actual work after #execute()# handles initialization */
  bool run();  

public:
  /** construct an empty class */
  PseudoDiameter();
  
  /**
   * convenience constructor
   * @param valid graph 
   * 
   * class will be in #algorithmicState==READY# if succeeds, #INVALID# otherwise
   */
  PseudoDiameter( const Graph* graph ); // constructs ready class

  /** destructor */
  ~PseudoDiameter();

  /**
   * sets the class to #algorithmicState==EMPTY# as if it were just
   * created by the default constructor.  #INVALID# state is erased
   * and timers are reset
   */
  virtual bool reset();

  /**
   * set an input graph
   * @return true only iff no error detected
   * This sets the class to #algorithmicState==READY# and gives
   * it a graph to execute on.
   * @param is a pointer to a valid graph
   */
  bool setGraph( const Graph* graph );

  /**
   * Set shrinking strategy
   * @return true iff no error detected
   *
   * PseudoDiameter computations rely on forward and reverse BFS's.
   * After the forward BFS is computed, a list of nodes farthest
   * from the forward BFS (aka the startVtx ) is generated as
   * candidates for the endVtx.  reverseBFS's are rooted at
   * the candidates to see which one should be chosen. 
   *
   * One technique for reducing the work is shrinking the set of
   * candidates using some heuristic.  There are a few to 
   * choose from.  Each descendant of PseudoDiamShrinkingStrategy
   * implements a different one.
   *
   * @param shrink must be a descendant of the abstract base class.
   * @see PseudoDiamShrinkingStrategy
   */
  bool setShrinkingStrategy( PseudoDiamShrinkingStrategy * shrink );

  /**
   * @param root must be in range [0..nVtxs )
   *        -1 lets the object choose the starting point itself (default)
   * @return true iff no error detected
   */
  bool setRoot( const int root );               


  /**
   * enable parition restrictions
   * @param partitionMap an array of length nVtxs with each entry corresponding to 
   *        a vertex in the graph.  The partition labels need not be in any particular
   *        order, partitionMap[i] != partitionMap[j] implies that vertices i and j are
   *        not in the same partition.
   * @return true iff BFS not INVALID, no error detected, and parition not already set
   */
  bool enablePartitionRestrictions( const int partitionMap[], PartitionMembership membership = strict );

  /**
   * disable partition restrictions
   * @return true iff BFS not INVALID, partition restrictions are set, and no error detected
   */
  bool disablePartitionRestrictions();

  /**
   * set the partition ID that the Pseudodiam should restrict itself to
   * @return true iff partition restrictions are enabled
   * @param i should match some id in the partitionMap
   */
  bool setCurrentPartition( const int i ) ;

  /**
   * Run the pseudo-diameter heuristic
   * @return true iff no error detected
   */
  virtual bool execute();

  /**
   * Query psuedo peripheral nodes.
   * @return the pseudo-peripheral nodes.
   * The result is undefined if #algorithmicState!=DONE#
   */
  pair<int, int> queryEndpoints() const { return pair<int,int>(startVtx,endVtx); }

  /**
   * @return the diameter of the forward BFS
   * The result is undefined if #algorithmicState!=DONE#
   */
  int queryDiameter() const { return diameter; }

  /**
   * @return the number of vertices visited
   * The result is undefined if #algorithmicState!=DONE#
   */
  int queryNVisited() const { return nVisited; }   // returns number vertices visited on last run

  /** @return minimum of the maximum widths of a level set in the forwardBFS */
  int queryMinMaxWidth() const { return minMaxWidth; }

  /** @return maximum width of a level set in the reverseBFS */
  int queryRevMaxWidth() const { return revMaxWidth; }

  /** @return the total number of times a BFS short-circuited */
  int queryNShortCircuits() const { return nShortCircuits; }

  /** @return the total number of BFS's computed (forward and reverse) */
  int queryNBFS() const { return nBFS; } // returns number of breadth-first searches

  /** @return total number of times a endVtx was made the new startVtx 
      (hence flipping a BFS) */
  int queryNFlips() const { return nFlips; } // returns number of times candidate was made new start

  /** grant const access to the forward BFS */
  const BreadthFirstSearch* getForwardBFS() const { return forwardBFS; }

  /** grant const access to the reverse BFS */ 
  const BreadthFirstSearch* getReverseBFS() const { return reverseBFS; }

  /** grant const access to the execution timer */
  const stopwatch& getTimer() const { return timer; }

private:
  SPINDLE_DECLARE_DYNAMIC( PseudoDiameter )
};

/**
 * @memo abstract base class for various shrinking strategies for PseudoDiameter
 * @type class
 *
 * All these strategies take a set of vertices (src_begin, src_end)
 * and construct a second vector that is a proper subset via some
 * graph based heuristic.  A good shrinking strategy drastically
 * reduces the second set without producing significantly inferior
 * pseudo-peripheral nodes in practice.
 *
 * @author Gary Kumfert
 * @version #$Id: PseudoDiameter.h,v 1.2 2000/02/18 01:32:03 kumfert Exp $#
 */
class PseudoDiamShrinkingStrategy { 
protected:
  /** The large set of vertices stored in (ID,deg) pairs */
  vector< pair < int, int > > large_set;

  /** The small set of vertices stored in (ID,deg) pairs */
  vector< pair < int, int > > small_set;

  /**
   * given a list of vertices in an array, pack (ID,deg) pairs in large_set
   * @param src_begin points to the begining of an array of ints
   *        in the range [0..g->queryNVtxs() )
   * @param src_end points to one-past-end of the array.
   * @param g points to a valid graph
   */
  void pack_large_set( const int * src_begin, const int * src_end, const Graph *g );
  
public:
  /**
   * pure virtual function
   * @return true iff no error found
   * @param src_begin points to the begining of an array of ints
   *        in the range [0..g->queryNVtxs() )
   * @param src_end points to one-past-end of the array.
   * @param g points to a valid graph
   * @param dest is a vector of ints that returns the reduced list of candidates
   */
  virtual bool shrink( const int * src_begin,
		       const int * src_end, 
		       const Graph   * g, 
		             vector< int >& dest ) = 0;
  /** destructor */
  virtual ~PseudoDiamShrinkingStrategy() {}
};

/**
 * @memo Implements the shrinking strategy by Sloan
 * @type class
 *
 * Reduces the list of candidates by sorting them by degree and choosing
 * the lowest half of them.  This is the least aggressive of the 
 * collection.
 */
class SloanShrinkingStrategy : public PseudoDiamShrinkingStrategy { 
public:
  /**
   * pure virtual function
   * @return true iff no error found
   * @param src_begin points to the begining of an array of ints
   *        in the range [0..g->queryNVtxs() )
   * @param src_end points to one-past-end of the array.
   * @param g points to a valid graph
   * @param dest is a vector of ints that returns the reduced list of candidates
   */
  virtual bool shrink( const int * src_begin, const int * src_end, 
		       const Graph * g, vector< int >& dest );
  virtual ~SloanShrinkingStrategy() {}
};

/**
 * @memo Implements the shrinking strategy by Duff, Reid, and Scott
 * @type class 
 *
 * It selects a single candidate of each degree.
 * Significantly faster than Sloan's
 */
class DuffReidScottShrinkingStrategy : public PseudoDiamShrinkingStrategy { 
public:
  /**
   * pure virtual function
   * @return true iff no error found
   * @param src_begin points to the begining of an array of ints
   *        in the range [0..g->queryNVtxs() )
   * @param src_end points to one-past-end of the array.
   * @param g points to a valid graph
   * @param dest is a vector of ints that returns the reduced list of candidates
   */
  virtual bool shrink( const int * src_begin, const int * src_end, 
		       const Graph * g, vector< int >& dest );
  virtual ~DuffReidScottShrinkingStrategy() {}
};

/** 
 * @memo Implements the Reid-Scott shrinking strategy.
 * @type class 
 *
 * Select 'nVtxsMax' vertices of minimum degree subject to the constraint
 * that no two selected vertices are adjacent.
 * In practice, this does not have an appreciable improvement of 
 * distance or speed over Duff-Reid-Scott, but it does tend to reduce the
 * maximum width.
 *
 * nVtxsMax == 5 by default
 */
class ReidScottShrinkingStrategy : public PseudoDiamShrinkingStrategy { 
private:

  /** an additional temporary work vector */
  vector<int> temp;

  /** maximum number of vertices to examine */
  int nVtxsMax;

public:
  /** default constructor */
  ReidScottShrinkingStrategy();

  /** convenience constructor that also sets the maximum number of vertices
      in the small_set */
  ReidScottShrinkingStrategy( const int NVtxsMax);

  /** 
   * set the maximum number of vertices to allow in the small set.
   * @return true iff no error detected
   * @param must be nonnegative
   */
  bool setNVtxsMax( const int NVtxsMax );

  /**
   * @return the number of vertices currently allowed in the small set.
   */
  int getNVtxsMax() const;

  /**
   * pure virtual function
   * @return true iff no error found
   * @param src_begin points to the begining of an array of ints
   *        in the range [0..g->queryNVtxs() )
   * @param src_end points to one-past-end of the array.
   * @param g points to a valid graph
   * @param dest is a vector of ints that returns the reduced list of candidates
   */
  virtual bool shrink( const int * src_begin, const int * src_end, 
		       const Graph* g, vector< int >& dest );
  virtual ~ReidScottShrinkingStrategy() {}
};

SPINDLE_END_NAMESPACE 

#endif

