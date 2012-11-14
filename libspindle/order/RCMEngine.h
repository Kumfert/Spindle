//
// RCMEngine.h
//
// $Id: RCMEngine.h,v 1.2 2000/02/18 01:31:53 kumfert Exp $
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
//=======================================================================
//
//


#ifndef SPINDLE_RCM_ORDERING_H_
#define SPINDLE_RCM_ORDERING_H_

#ifndef SPINDLE_H_
#include "spindle/spindle.h"
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

#ifndef SPINDLE_BFS_H_
#include "spindle/BreadthFirstSearch.h"
#endif

#ifndef SPINDLE_PSEUDO_DIAM_H_
#include "spindle/PseudoDiameter.h"
#endif

#include "spindle/Graph.h"

SPINDLE_BEGIN_NAMESPACE

/**
 * @memo does a reverse cuthill-mckee ordering
 * @type class
 *
 * @author Gary Kumfert
 * @version #$Id: RCMEngine.h,v 1.2 2000/02/18 01:31:53 kumfert Exp $#
 *
 */
class RCMEngine : public OrderingAlgorithm {

public:
  /**
   * default constructor
   */
  RCMEngine();

  /**
   * convenience constructor
   */
  RCMEngine( const Graph* graph );

  /**
   * virtual destructor
   */ 
  virtual ~RCMEngine();

  /**
   * execute algorithm on a connected component
   */
  virtual bool execute();   

  /**
   * reset algorithm to EMPTY state
   */
  virtual bool reset();     

  /**
   * set graph
   */
  virtual bool setGraph( const Graph* graph );

  /**
   * set shrinking strategy
   */
  bool setPseudoDiamShrinkingStrategy( PseudoDiamShrinkingStrategy* shrink );

  // Returns true if all vertices in graph are numbered
  //  bool isAllPermuted()                    
  // { return nVtxsNumbered == g->size(); } 

  /**
   * Deprecated
   */ 
  bool enablePartitionRestrictions( const int partitionMap[] ) 
    { return bfs->enablePartitionRestrictions( partitionMap ); }

  /**
   * Deprecated
   */
  bool disablePartitionRestrictions()  // default
    { return bfs->disablePartitionRestrictions(); }

  /**
   * grant const access to the breadth first search object.
   */
  const BreadthFirstSearch* getBFS() const { return bfs; }

  /**
   * grant const access to the pseudo diameter object
   */
  const PseudoDiameter* getPseudoDiameter() const { return diam; }

  /**
   * This is actual time spent doing the ordering work.
   */
  const stopwatch& getOrderingTimer() const { return ordering_timer; }

  /**
   * Diagnostic time is Ordering Time + time spent finding
   * pseudo peripheral nodes and connected components.
   * This is circumvented if a pseudo peripheral node is given.
   */
  const stopwatch& getDiagnosticTimer() const { return diagnostic_timer; }

private:
  stopwatch ordering_timer;
  stopwatch diagnostic_timer;

  int nVtxsNumbered;

  //  const Graph* g;
  BreadthFirstSearch* bfs;
  PseudoDiameter* diam;

  //  SharedArray<int> old2New;

  void numberVertices( const int startVtx, const int vtxList[], const int nVtxs );
  bool executeFrom( const int endVtx, const Graph* g, int * old2New  );

  SPINDLE_DECLARE_DYNAMIC( RCMEngine )

};

SPINDLE_END_NAMESPACE

#endif
