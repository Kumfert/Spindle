//
// OrderingAlgorith.h
//
// $Id: OrderingAlgorithm.h,v 1.2 2000/02/18 01:31:53 kumfert Exp $
//
//  Gary Kumfert, Old Dominion University
//  Copyright(c) 1999, Old Dominion University.  All rights reserved.
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


#ifndef SPINDLE_ORDERING_ALGORITHM_H_
#define SPINDLE_ORDERING_ALGORITHM_H_

#ifndef SPINDLE_H_
#include "spindle/spindle.h"
#endif 

#ifndef SPINDLE_ALGORITHM_H_
#include "spindle/SpindleAlgorithm.h"
#endif
//
//#ifndef SPINDLE_GRAPH_H_
//#include "spindle/Graph.h"
//#endif
//
//#ifndef SPINDLE_GRAPH_COMPRESSOR_H_
//#include "spindle/GraphCompressor.h"
//#endif
//
//#ifndef SPINDLE_PERMUTATION_MAP_H_
//#include "spindle/PermutationMap.h"
//#endif
//
SPINDLE_BEGIN_NAMESPACE
// forward declarations
class PermutationMap;
class Graph;
class GraphCompressor;


/**
 * @memo Parent for all ordering algorithms
 * @type class
 *
 * This class handles the standard interface for all
 * ordering algorithms, including graph compression.
 *
 * The basic use is the following:
 * #OrderingAlgorithm * order = new //Some Derived Class #
 * #   assert( order->isEmpty() );#
 * #   assert( graph->isValid() );#
 * #order->setGraph( graph );#
 * #   assert( order->isReady() );#
 * #order->execute();#
 * #   assert( order->isDone() );#
 * #PermutationMap * perm = order->givePermutation();#
 * #   assert( perm->isValid() );#
 *
 * @author Gary Kumfert
 * @version #$Id: OrderingAlgorithm.h,v 1.2 2000/02/18 01:31:53 kumfert Exp $#
 */
class OrderingAlgorithm : public SpindleAlgorithm {  
protected:
  /**
   * points to the input graph
   */
  const Graph * fineGraph;
  /**
   * points to the internal compressed graph, or 0
   */
  Graph * coarseGraph;
  /**
   * defines when a compressed graph is used.
   * If the ratio of compressed vertices to uncompressed is greater than
   * this value, graph compression is not used.
   */
  float compressionRatio;
  /**
   * A simple flag to test for compression.
   */
  bool useGraphCompression; 
  /**
   * Points to a utility that does the graph compression.
   */
  GraphCompressor * compressor;

  /**
   * A permutation on the original graph
   */
  PermutationMap * finePermutation;

  /**
   * A permutation on the compressed graph, if any
   */
  PermutationMap * coarsePermutation;

  /**
   * Protected constructor
   */
  OrderingAlgorithm();
  /**
   * Protected constructor
   */
  OrderingAlgorithm( const Graph* graph );

private:
  /**
   * called from setCompressionRatio() and setGraph().
   */
  void recomputeCompression();

protected:
  /**
   *
   * A convenience function for derived classes.
   * After the derived class orders all the vertices, 
   * set #algorithmicState# to #DONE#
   * call this function to set up the #finePermutation# and #coarsePermutation#.
   *
   * If the derived class writes directly into the appropriate PermutationClass, 
   * then no arguments are necessary.  If both arguments are specified, 
   * it will only use the first one.
   * 
   * @param new2old a simple new2old array
   * @param old2new a simple old2new array
   */
  bool recomputePermutations(const int *new2old=0, const int *old2new=0);


public:
  /**
   * Destructor
   */
  virtual ~OrderingAlgorithm();

  /**
   * any derived class should override this, but call it internally.
   */
  virtual bool reset();

  /**
   * execute the ordering algorithm
   * NOTE: whenever the ordering algorithm thinks it
   * has reordered the entire graph, it should validate
   * the permutations by #return recomputePermutations();#
   */
  virtual bool execute()=0;

  /**
   *
   * @param graph should be valid.
   */
  virtual bool setGraph( const Graph* graph );

  /**
   * set the tolerance for the compression ratio.
   * if the ratio of vertices in the compressed graph to the 
   * number of vertices in the original graph is greater than
   * this tolerance, then the ordering will be performed on
   * the original graph.
   * 
   * Note that if the ratio is 0.0, then compression is
   * totally disabled.
   * 
   * @param ratio  a float in the range of [0.0, 1.0]
   */
  bool setCompressionRatio( const float ratio );


  /**
   * simple query to see if graph compression is being used.
   */
  bool isCompressed() const { return useGraphCompression; }

  /**
   * grant const access to the resulting permutation, or zero on error
   */
  const PermutationMap* getPermutation() const;

  /**
   * surrender the permutation.
   */
  PermutationMap* givePermutation();

  /**
   * grant const access to the graph compressor
   */
  const GraphCompressor* getGraphCompressor() const { return compressor; }
  
  /** 
   * grant const access to the coarse pemutation
   * @return 0 if no compression is used.
   */
  const PermutationMap * getCoarsePermutation() const 
    { return ( useGraphCompression ) ?  coarsePermutation : 0 ; }

  /**
   * grant const access to the compressed graph.
   * @return 0 if no compression is used.
   */
  const Graph * getCGraph() const 
    { return ( useGraphCompression ) ? coarseGraph : 0 ; }

SPINDLE_DECLARE_DYNAMIC( OrderingAlgorithm )

};

SPINDLE_END_NAMESPACE

#endif
