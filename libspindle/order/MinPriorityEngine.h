//
// MinPriorityEngine.h
//
// $Id: MinPriorityEngine.h,v 1.2 2000/02/18 01:31:53 kumfert Exp $
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

#ifndef MIN_PRIORITY_ENGINE_H_
#define MIN_PRIORITY_ENGINE_H_

#ifdef DEBUG_TRACE
#undef DEBUG_TRACE
#endif

#define DEBUG_TRACE true

#ifndef SPINDLE_SYSTEM_H_
#include "spindle/SpindleSystem.h"
#endif

#ifndef SPINDLE_ORDERING_ALGORITHM_H_
#include "spindle/OrderingAlgorithm.h"
#endif

#ifndef SPINDLE_AUTO_PTR_H_
#include "spindle/SharedPtr.h"
#endif

#ifndef SPINDLE_PERMUTATION_H_
#include "spindle/PermutationMap.h"
#endif 

#ifndef SPINDLE_GRAPH_H_
#include "spindle/Graph.h"
#endif


#ifndef SPINDLE_QUOTIENT_GRAPH_H_
#include "spindle/QuotientGraph.h"
#endif

#ifndef SPINDLE_ARRAY_BUCKET_SORTER_H_
#include "spindle/ArrayBucketSorter.h"
#endif

SPINDLE_BEGIN_NAMESPACE

// Abstract Base Class for various priority strategiess.
// This will be defined below along with several 
// derived classes for external degree and approximate degree
class MinPriorityStrategies;

/**
 * @memo produced fill reducing orderings
 * @type class
 *
 * @author Gary Kumfert
 * @version #$Id: MinPriorityEngine.h,v 1.2 2000/02/18 01:31:53 kumfert Exp $#
 */
class MinPriorityEngine : public OrderingAlgorithm {
public:
  /**
   * used for moving sets of vertices around
   */
  typedef QuotientGraph::VertexList VertexList;

  /**
   * used for the priority queue
   */
  typedef ArrayBucketSorter BucketSorter;

private:
  int n; // original number of supernodes in graph
  bool randomizeGraph; // true iff we want to randomize the graph
  BucketSorter *degStruct; // The priority queue of (vertex,priority) pairs.
  QuotientGraph *qgraph;   // The quotient graph
  MinPriorityStrategies *priorityStrategy; // the priority strategy
  
  VertexList reachableVtxs;  // the list of reachable vertices
  VertexList mergedVtxs;     // the list of merged vertices
  // VertexList delayedVtxs; // the list of delayed vertices.
  VertexList workVtxList;    // a work list of vertices
  vector< pair< int, int > > purgedVtxs;  // actually degree, vtx pairs

  int delayWeight;  // 
  int delta;        // 
  int maxSteps;     // maximum number of eliminated supernodes for each update
  int *workVec;     // temporary work space
  float heavyVtxTolerance; // tolerance for removing highly connected vtxs 
  int * explicitOrder; // an explicit input ordering new2old (optional)


  //  SharedArray<int> PartitionMap; 
  stopwatch timer;
  stopwatch stageTimer;
  stopwatch stepTimer;
  stopwatch degreeComputationTimer;
  stopwatch delayedVtxsTimer;

  void processDelayedVtxs(); 
  bool initialize();
  bool run();
  bool finalize();

public:  
  /** default constructor */
  MinPriorityEngine();
  /** convenience constructor */
  MinPriorityEngine( const Graph* graph );
  /** destructor */
  virtual ~MinPriorityEngine();

  /** compute the ordering */
  virtual bool execute();
  /** reset class to a state of default constructor */
  virtual bool reset();

  /** set the graph to order */
  bool setGraph( const Graph * graph );
  /** set the priority strategy */
  bool setPriorityStrategy( MinPriorityStrategies* PriorityStrategy );
  /** force the input graph to be randomized */
  void setRandomizeGraph( bool randomizeIt ) { randomizeGraph = randomizeIt; }
  /** set an explicit input ordering for the graph */
  bool setExplicitOrder( const PermutationMap& perm );
  /** return a const pointer to current priority strategy */
  const MinPriorityStrategies* getPriorityStrategy();

  /** check if the entire quotient graph is eliminated */
  bool isAllPermuted() const { 
    for( int i=0; i<n; i++) {
      if ( !qgraph->isEliminated(i) ) {
	return false;
      }
    }
    return true;
  }
  
  /** delta is a tolerance for multiple elimination schemes */
  bool setDelta( const int i );
  /** query delta */
  int getDelta() const { return delta; }
  
  /** 
   * allows restriction of the maximum number of supernodes to
   * eliminate before updating the quotient graph.
   */
  bool setMaxSteps( const int i ) { 
    if ((i<0)||(i>n)) return false; 
    maxSteps = i; 
    return true; 
  }
  /** query the maximum number of supernodes eliminated per update */
  int getMaxSteps() const { return maxSteps; }
  
  //SharedPtr<PermutationMap> getPermutationMap() const;

  /**@name timer access */
  //@{
  /** const access to basic timer */
  const stopwatch& getTimer() const { return timer; }
  /** const access to timer that runs once per mass elimination */
  const stopwatch& lendStageTimer() const { return stageTimer; }
  /** const access to timer that runs one per eliminated supernode */
  const stopwatch& lendStepTimer() const { return stepTimer; }
  /** const access to timer that runs once per degree computation */
  const stopwatch& lendDegreeComputationTimer() const { return degreeComputationTimer; }
  /** const access to timer that tracks time spent on delayed Vtxs */
  const stopwatch& lendDelayedVtxsTimer() const { return delayedVtxsTimer; }
  //@}

  /** grant const access to the quotient graph itself */
  const QuotientGraph& lendQuotientGraph() const { return *qgraph; }
  /** (deprecated) reproduces an off by one bug in GENMMD */
  void enableIncrementByOne() { qgraph->enableIncrementByOne(); }
  /** (default) do not increment snodes adjacent to enodes by one */
  void disableIncrementByOne() { qgraph->disableIncrementByOne(); }

  // These routines are useful for indefinite problems
  //  bool enableDelayedVertices( const VertexList& delayedVertices );
  //  bool disableDelayedVertices();
  //  void setDelayWeight( const int i ) { delayWeight = i; }
  
  /** set tolerance for highly connected vertices to be pre-removed */
  bool setHeavyVertexTolerance( float tolerance ); // default 0.8

  // some pass-through settings for qgraph
  //  bool setMixAdjCompression( const int option );
  //  bool setNAdjClqCompressionBound( const int bound );
  //  int queryNPurgedVtxs() const ;
  // Partition Restrictions
  //  bool disablePartitionRestrictions() { return false; } // default
  //  bool enablePartitionRestrictions( const int partitionMap[] ) { return false; }

  SPINDLE_DECLARE_DYNAMIC( MinPriorityEngine )
};

/*
inline bool 
MinPriorityEngine::setMixAdjCompression( const int option ) {
  if ( qgraph != 0 ) { 
    return qgraph->setMixAdjCompression( option );
  } else { 
    return false;
  }
}

inline bool 
MinPriorityEngine::setNAdjClqCompressionBound(const int bound ){
  if ( qgraph != 0 ) { 
    return qgraph->setNAdjClqCompressionBound( bound );
  } else { 
    return false;
  }
}
*/

SPINDLE_END_NAMESPACE

#endif // ifndef MIN_PRIORITY_ORDERING_H_
