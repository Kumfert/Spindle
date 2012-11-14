//
// SloanEngine.h
//
// $Id: SloanEngine.h,v 1.2 2000/02/18 01:31:54 kumfert Exp $
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

#ifndef SPINDLE_SLOAN_ENGINE_H_
#define SPINDLE_SLOAN_ENGINE_H_

#ifndef SPINDLE_H_
#include "spindle/spindle.h"
#endif 

#ifndef SPINDLE_BFS_H_
#include "spindle/BreadthFirstSearch.h"
#endif

#ifndef SPINDLE_PSEUDO_DIAM_H_
#include "spindle/PseudoDiameter.h"
#endif

#ifndef SPINDLE_GRAPH_H_
#include "spindle/Graph.h"
#endif 

#ifndef SPINDLE_BINARY_HEAP_H_
#include "spindle/BinaryHeap.h"
#endif

#ifndef SPINDLE_PERMUTATION_H_
#include "spindle/PermutationMap.h"
#endif

#ifndef SPINDLE_AUTO_PTR_H_
#include "spindle/SharedPtr.h"
#endif

#ifndef SPINDLE_ORDERING_ALGORITHM_H_
#include "spindle/OrderingAlgorithm.h"
#endif

SPINDLE_BEGIN_NAMESPACE

/**
 * @memo produces wavefront reducing orderings.
 * @type class
 *
 * @author Gary Kumfert
 * @version #$Id: SloanEngine.h,v 1.2 2000/02/18 01:31:54 kumfert Exp $#
 */
class SloanEngine : public OrderingAlgorithm {
public:
  typedef BreadthFirstSearch BFS;
  typedef BinaryHeap Heap;

  //
  // constructors/destructors
  //
  /** 
   * default constructor
   */
  SloanEngine();                              
  /**
   * convenience constructor
   */
  SloanEngine( const Graph* graph );          
  /**
   * destructor
   */
  virtual   ~SloanEngine();                              

  /**
   * executes the actual ordering
   */
  virtual bool execute();
  /**
   * Resets the object to the state it was after 
   * default constructor.
   */
  virtual bool reset();

  virtual bool setGraph( const Graph* graph );


  /** @name explicit manipulation of start and end nodes*/
  //@{
  bool setStartVtx( const int newStartVtx );
  bool setStartVtxs( const int newStartVtxs[], const int nNewStartVtxs);
  bool setEndVtx( const int newEndVtx );
  bool setEndVtxs( const int newEndVtxs[], const int nNewEndVtxs);
  bool resetStartVtxs();
  bool resetEndVtxs();
  //@}

  /**
   * Returns true if all vertices in graph are numbered
   */
  bool isAllPermuted();                    

  // 
  // get/set functions to modify the behavior of the algorithm.
  // The Sloan Ordering class will perform well enough without touching any of these, 
  // but some fine-tuning or alternate uses require lower level access.
  //
  /**
   * Changes the shrinking strategy
   * @see PseudoDiameter
   */
  bool setPseudoDiameterShrinkingStrategy( PseudoDiamShrinkingStrategy * shrink );

  /**
   * Changes the assumed class of the matrix
   * @arg classNumber can be 0==Don't Know, 1==Local Bounded, 2==Global Bounded
   * 0 => two runs, and best one chosen.  disabled if explicit weights are chosen.
   * See Kumfert & Pothen for details.
   */
  bool setClass(const int classNumber); 
  /** return class */
  int getClass();

  /** @name Set individual weights */
  //@{
  void setGlobalWeight(const int weight); 
  void setLocalWeight(const int weight);
  void setVertexWeight(const int weight);
  void setRefinementWeight(const int weight);
  //@}

  /** @name NOTE:  Weights are normalized only when weightingScheme == DETERMINDED_BY_CLASS
   *        If you want to normalize explicitly set weights, you must do so
   *        manually.
   */
  //@{
  /** enables normalized weights (default) */
  void enableNormalizedWeights();
  /** disables normalized weights  */
  void disableNormalizedWeights();
  //@}

  /**
   * @name Refinement: used to refine an existing ordering, whether implicit or explicit
   */
  //@{
  /** no refinement (default) */
  void disableRefinement();                          
  /** refine implicit ordering */
  void enableRefinement();                           
  /** refine explicit ordering */
  bool enableRefinement( const PermutationMap* explicit_order ); 
  //@}

  // enables user to compute their own initial priority altogether.
  // This replaces the distance function term with this array
  void disableUserGlobalPriority();                     // (default)
  void enableUserGlobalPriority( const int * userGlobalPriority );

  /** @name Partition Restrictions */
  //@{
  /** Unsupported */
  bool disablePartitionRestrictions();   // default
  /** Unsupported */
  bool enablePartitionRestrictions( const int partitionMap[] );
  //@}

  /** @name Boundary restrictions */
  //@{
  /** do not force all end vtxs to be last (default) */
  void disableForceEndVtxsLast() { forceEndVtxsLast = false; }
  /** ensure that all end vtxs must be last */
  void enableForceEndVtxsLast() { forceEndVtxsLast = true; }
  /** do not force all start nodes to be in wavefront (default) */
  void disableMakeStartVtxsInWavefront() { makeStartVtxsInWavefront = false; }
  /** force all start nodes to be in wavefront */
  void enableMakeStartVtxsInWavefront() { makeStartVtxsInWavefront = true; } 
  //@}

  /** @name const access to internal classes */
  //@{
  /** @name timers */
  //@{
  /** total execution time */
  const stopwatch& getTotalExecutionTimer() const;
  /** actual time spent ordering connected components */
  const stopwatch& getOrderComponentTimer() const;
  /** another timer */
  const stopwatch& getNumberVerticesKeptTimer() const;
  /** another timer */
  const stopwatch& getNumberVerticesRejectedTimer() const;
  //@}
  /** @name other internal classes */
  //@{
  /** 
   * Pseudo Diameter is used if a pseudo peripheral node
   * was not given.
   */
  const PseudoDiameter* getPseudoDiameter() const;
  /**
   * BFS is used for various reasons
   */
  const BFS* getBFS() const; 
  /**
   * Heap is used to select each node
   */
  const Heap* getHeap() const;
  //@}
  /** @name wavefront */
  //@{
  /** Returns the wavefront of the permuted graph */
  const SharedArray<int>& getWavefront() const;         
  /** Returns the wavefront of the permuted graph */
  SharedArray<int>& getWavefront();
  //@}
  //@}

private:
  typedef enum{ INACTIVE, PREACTIVE, ACTIVE, NUMBERED } nodeStatus;
  enum{ DONT_KNOW, CLASS_1, CLASS_2 } classOfGraph;
  enum{ DETERMINED_BY_CLASS, DETERMINED_BY_USER } weightingScheme;
  enum{ ENABLED, DISABLED } normalizedWeighting;

  int GlobalWeight;
  int LocalWeight; 
  int VertexWeight;
  int RefinementWeight;

  int nVtxsNumbered;
  int nVtxsTotal;

  bool forceEndVtxsLast; 
  bool makeStartVtxsInWavefront;

  SharedArray<nodeStatus> status;           
  SharedArray<int> wavefront; 
  SharedArray<int> oldWavefront;
  SharedArray<int> initial_priority;        
  SharedArray<int> vtxWeight;                   
  SharedArray<int> new2Old_1;
  SharedArray<int> new2Old_2;

  const int * alternateGlobalPriority;     
  const PermutationMap * explicitOrder;

  const Graph* g;
  BFS* bfs;
  Heap* heap;
  SharedPtr< PseudoDiameter > diam;

  stopwatch totalExecutionTimer;    // total execution
  stopwatch orderComponentTimer;    // for each component
  stopwatch numberVerticesKeptTimer; // for the good ordering
  stopwatch numberVerticesRejectedTimer; // for the rejected ordering

  vector<int> startVtxs;
  vector<int> endVtxs;

  void setNVtxsNumbered( const int new_nVtxsNumbered ) ;
  void numberVertices( const int vtxList[], const int nVtxs );
  void computeInitialPriority( const int vtxList[], const int nVtxs );
  void unweightedOrdering( const int nVtxs );
  void weightedOrdering( const int nVtxs );
  void secondOrderNeighbors( const int weight, const int vtx );
  
  bool orderEverything();
  bool orderComponent();
  void findStartVtxs();
  void findEndVtxs();

  SPINDLE_DECLARE_DYNAMIC( SloanEngine )
};

// Methods for extracting the result
inline bool 
SloanEngine::isAllPermuted() { 
  return algorithmicState == DONE;
} 

inline void 
SloanEngine::setGlobalWeight(const int weight) {     
  weightingScheme = DETERMINED_BY_USER; 
  GlobalWeight = weight; 
}

inline void 
SloanEngine::setLocalWeight(const int weight) {
  weightingScheme = DETERMINED_BY_USER; 
  LocalWeight = weight; 
}

inline void 
SloanEngine::setVertexWeight(const int weight) {
  weightingScheme = DETERMINED_BY_USER;
  VertexWeight = weight; 
}

inline void 
SloanEngine::setRefinementWeight(const int weight) {
  weightingScheme = DETERMINED_BY_USER; 
  RefinementWeight = weight;
}


inline void 
SloanEngine::disableNormalizedWeights() {
  normalizedWeighting = DISABLED; 
} 

inline void 
SloanEngine::enableNormalizedWeights() {
  normalizedWeighting = ENABLED; 
}

inline void 
SloanEngine::disableRefinement() { 
  RefinementWeight = 0; 
} 

inline void
SloanEngine::enableRefinement() {
  RefinementWeight = 1; 
}

inline bool
SloanEngine::enableRefinement(const PermutationMap* explicit_order) {
  if ( explicit_order == 0 ) { return false; }
  if ( !explicit_order->isValid() ) { return false; }
  RefinementWeight = 1; 
  explicitOrder = explicit_order;
  return true;
} 

inline void 
SloanEngine::disableUserGlobalPriority() {
  alternateGlobalPriority = 0; 
} 

inline void 
SloanEngine::enableUserGlobalPriority(const int * userGlobalPriority) { 
  alternateGlobalPriority = userGlobalPriority;
}
  
inline bool 
SloanEngine::disablePartitionRestrictions() {
  if ( bfs == 0 ) { return false; }
  return bfs->disablePartitionRestrictions(); 
}


inline bool 
SloanEngine::enablePartitionRestrictions( const int partitionMap[] ) {
  if ( bfs== 0 ) { return false; }
  return bfs->enablePartitionRestrictions( partitionMap ); 
}

inline const stopwatch&
SloanEngine::getTotalExecutionTimer() const {
  return totalExecutionTimer;
}

inline const stopwatch&
SloanEngine::getOrderComponentTimer() const { 
  return orderComponentTimer;
}

inline const stopwatch&
SloanEngine::getNumberVerticesKeptTimer() const {
  return numberVerticesKeptTimer;
}

inline const stopwatch&
SloanEngine::getNumberVerticesRejectedTimer() const {
  return numberVerticesRejectedTimer;
}

//  // This is actual time spent doing the ordering work.
// ticks queryOrderingUserTicks() const { return ordering_timer.queryTotalUserTicks() - orderingOverheadUserTicks; }
// ticks queryOrderingSystemTicks() const { return ordering_timer.queryTotalSystemTicks() - orderingOverheadSystemTicks; }


inline const PseudoDiameter*
SloanEngine::getPseudoDiameter() const {
  return &(*diam);
}




inline const SloanEngine::BFS*
SloanEngine::getBFS() const { 
  return bfs; 
}

inline const SloanEngine::Heap*
SloanEngine::getHeap() const { 
  return heap; 
}


inline const SharedArray<int>&
SloanEngine::getWavefront() const {
  return wavefront;
}

inline SharedArray<int>&
SloanEngine::getWavefront() {
  return wavefront;
}

inline void
SloanEngine::setNVtxsNumbered( const int new_nVtxsNumbered ) { 
  nVtxsNumbered = new_nVtxsNumbered;
}

SPINDLE_END_NAMESPACE

#endif
