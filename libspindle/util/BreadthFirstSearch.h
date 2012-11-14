//
// BreadthFirstSearch.h
//
// $Id: BreadthFirstSearch.h,v 1.2 2000/02/18 01:32:03 kumfert Exp $
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
//=========================================================================
//

#ifndef SPINDLE_BFS_H_
#define SPINDLE_BFS_H_

#ifndef SPINDLE_SYSTEM_H_
#include "spindle/SpindleSystem.h"
#endif

#ifndef SPINDLE_ALGORITHM_H_
#include "spindle/SpindleAlgorithm.h"
#endif

#ifndef SPINDLE_AUTO_ARRAY_H_
#include "spindle/SharedArray.h"
#endif

#ifndef SPINDLE_GRAPH_H_
#include "spindle/Graph.h"
#endif

#ifdef REQUIRE_OLD_CXX_HEADER_SUFFIX
#include "vector.h"
#else
#include <vector>
using std::vector;
#endif

SPINDLE_BEGIN_NAMESPACE

/**
 * @memo Perform a Breadth-First search on a graph.
 * @type class
 *
 * This utility class can perform breadth-first searches on
 * a graph starting at a specific point or set of points, called root(s).
 * The breadth-first search is limited to vertices that have
 * a path from any one of the roots.
 *
 * It is used by the PseudoDiameter class which requires the
 * added functionality to "short-circuit" a breadth-first search when
 * one of the "levels" becomes too large. 
 *
 * Futhermore, this class can be made sensitive to partition restrictions.
 * By enabling this feature, the breadth first search restricts itself
 * to only vertices on the graph with the same partition number as
 * the root(s).  Note that all roots must exist on the
 * same partition ID for this to work.  
 *
 * @see PseudoDiameter
 * @author Gary Kumfert
 * @version #$Id: BreadthFirstSearch.h,v 1.2 2000/02/18 01:32:03 kumfert Exp $#
 */
class BreadthFirstSearch : public SpindleAlgorithm {
public:
  /**
   * Defines partition restrictions to involve strict membership, 
   * or to include nodes adjacent to a partition.
   */
  typedef enum{ strict, loose } PartitionMembership;

protected:
  /**
   * const pointer to a graph.  This guarantees that the BFS cannot alter the
   * graph that its pointing to.
   */
  const Graph* g;              

  /** Number of vertices in the graph */
  int nVtxs;

  /** distance from each vertex to the root */
  SharedArray<int> distance;   

  /** marker array that prevents double visiting, reset before each run */
  SharedArray<int> visited;    

  /** the stamp used in the visited array */
  int stamp;

  /** list of roots to start from */
  vector<int> roots;           

  /** the order in which vertices are visited */
  SharedArray<int> new2old;    

  /** const pointer to a partition array (optional) */
  const int *partition;        

  /** identity of the current working partition (optional) */
  int curPartition;

  /** determine how vertices are considered members of a partition */
  PartitionMembership partitionMembership;

  // This next set of data types hold specific details about the last time
  // a breadth first search was executed from a particular root.
  
  /** number of vertices visited on the most recent run */
  int nVisited;

  /** number of vertices farthest away from the root(s) on the most recent run */
  int nFarthest;
  
  /** height of the search tree */
  int height;

  /** widest level of the search tree */
  int width;
  
  /** 
   * maximum number of vertices having same distance from root
   * (used in short-circuiting strategy )
   */
  int maxWidth;  
  
  /** true iff the last run was short-circuited */
  bool shortCircuited;
  
  /** maximum degree of all vertices encountered */
  int maxDegree;

  /** the ID of a vertex that achieves this maximum degree */
  int maxDegreeVtx;  

  /** minimum degree of all vertices encountered */
  int minDegree;
  
  /** the ID of a vertex that achieves this minimum degree */
  int minDegreeVtx;

  /** A basic timer */
  stopwatch timer;

private:

  /** does the actual BFS  */
  bool run();            

  /** does the BFS when partition restrictions are activated */
  bool partitionedRun(); 

  /** does the BFS when partition restrictions are activated, but allow neighbors */
  bool loosePartitionedRun(); 

protected:

  /** */
  int current_distance;

  /** */
  int cur_width;

  /** */
  int widest_yet;

public:
  // constructors / destructors

  /** default constructor: upon success #SpindleAlgorithm::algorithmicState == EMPTY# */
  BreadthFirstSearch();

  /** convenience constructor: 
   *  upon success, #SpindleAlgorithm::algorithmicState == READY# 
   *  @param graph points to a validated graph
   */
  BreadthFirstSearch( const Graph* graph );

  /** 
   * destructor:
   */
  ~BreadthFirstSearch();

  /**
   * resets internal state to equivalent of default constructor
   * upon success #SpindleAlgorithm::algorithmicState == EMPTY#
   */
  virtual bool reset();  

  /**
   * enable short circuiting mechanism.
   * @return true iff BFS not INVALID and no error detected
   *
   * This is used primarily by the PseudoDiameter algorithm.
   *
   * @param max_width must be in range [0..nVtxs), if a level is detected in
   *        the run that is greater than max_width, the BFS aborts.
   * @see PseudoDiameter
   */
  bool enableShortCircuiting( const int max_width );

  /**
   * disable short circuiting mechanism
   * @return true ifff BFS not INVALID and no error detected
   */
  bool disableShortCircuiting();

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
   * set the graph to operate on.
   * @return true iff BFS not INVALID, graph is valid, and no error detected.
   * @param graph must point to a valid graph object for the lifetime of the BFS.
   */
  bool setGraph( const Graph* graph );               

  /**
   * set the root for the BFS
   * @return true iff BFS not INVALID and no error detected
   *
   * if #algorithmicState==EMPTY# then the internal list of roots is empty.
   *                              The new root is set as root for next #execute()# call. 
   *                              and the new state is #READY#.
   * if #algorithmicState==READY# then the new root is appended to an internal list of roots.
   * if #algorithmicState==DONE#, then the internal list of roots is deleted, and the new root
   *                              is added previous roots are removed first.
   * @param root is in range [0 .. nVtxs)
   */
  bool setRoot( const int root );

  /**
   * adds a set of roots to the BFS
   * @return true iff BFS not INVALID and no error detected
   *
   * if #algorithmicState==EMPTY# then the internal list of roots is empty.
   *                              The new roots are set as roots for next #execute()# call. 
   *                              and the new state is #READY#.
   * if #algorithmicState==READY# then the new roots are appended to an internal list of roots.
   * if #algorithmicState==DONE#, then the internal list of roots is deleted, and the new roots
   *                              are added previous roots are removed first.
   * @param root is an array of roots, each in the range [0 .. nVtxs)
   * @param nRoots is the number of entries in the root[] array.
   */
  bool setRoots( const int* root, const int nRoots);

  /**
   * removes all roots from BFS,
   * @return true iff BFS not INVALID
   *
   * Since when roots are removed, there is not enough information to execute, 
   * the algorithmicState is set to EMPTY.
   */
  bool resetRoots();                                 

  /**
   * run the BFS
   */
  virtual bool execute();

  /**
   * @return number vertices visited on last run
   */
  int queryNVisited() const { return nVisited; }   

  /**
   * @return number of vertices max distance from root on last run.
   */
  int queryNFarthest() const { return nFarthest; } 

  /**
   * @return max Distance of all vertices visited on last run.
   */
  int queryHeight() const { return height; }       

  /**
   * @return max number of vertices of same distance on last run.
   */
  int queryWidth() const { return width; }        

  /**
   * @return true iff the BFS was short-circuited
   */
  bool hasShortCircuited() const { return shortCircuited; }

  /**
   * @return the maximum degree encountered in in the last run.
   */
  int queryMaxDegree() const { return maxDegree; }

  /**
   * @return the ID of a vertex that had the maximum degree encountered in in the last run.
   */
  int queryMaxDegreeVtx() const { return maxDegreeVtx; }

  /**
   * @return the minimum degree encountered in in the last run.
   */
  int queryMinDegree() const { return minDegree; }

  /**
   * @return the ID of a vertex that had the minimum degree encountered in in the last run.
   */
  int queryMinDegreeVtx() const { return minDegreeVtx; }

  /**
   * @return timer for execution time.
   */
  const stopwatch& getTimer() const { return timer; }

  /**
   * @return const reference to the New2Old array
   */
  const SharedArray<int>& getNew2Old() const;
  
  /**
   * @return non-const reference to the Hew2Old array
   */
  SharedArray<int>& getNew2Old();
  
  /**
   * @return const reference to the distance array
   */
  const SharedArray<int>& getDistance() const;

  /**
   * @return non-const reference to the distance array
   */
  SharedArray<int>& getDistance();

  /**
   * @return distance of a vertex from the root
   * @param i vertex ID
   */
  int queryDistance( const int i ) const;
  
private:
  SPINDLE_DECLARE_DYNAMIC( BreadthFirstSearch )
  
};

SPINDLE_END_NAMESPACE

#endif


