//
// GraphCompressor.h
//
// $Id: GraphCompressor.h,v 1.2 2000/02/18 01:32:03 kumfert Exp $
//
//  Gary Kumfert, Old Dominion University
//  Copyright(c) 1997-1999, Old Dominion University.  All rights reserved.
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

#ifndef SPINDLE_GRAPH_COMPRESSOR_H_
#define SPINDLE_GRAPH_COMPRESSOR_H_

#ifndef SPINDLE_SYSTEM_H_
#include "spindle/SpindleSystem.h"
#endif

#ifndef SPINDLE_BASE_CLASS_H_
#include "spindle/SpindleBaseClass.h"
#endif

#ifndef SPINDLE_ALGORITHM_H_
#include "spindle/SpindleAlgorithm.h"
#endif

#ifndef SPINDLE_GRAPH_H_
#include "spindle/Graph.h"
#endif

#ifndef AUTO_ARRAY_H_
#include "spindle/SharedArray.h"
#endif

#ifndef AUTO_PTR_H_
#include "spindle/SharedPtr.h"
#endif

#ifndef PERMUTATION_H_
#include "spindle/PermutationMap.h"
#endif

#ifdef REQUIRE_OLD_CXX_HEADER_SUFFIX
#include "vector.h"
#else
#include <vector>
using std::vector;
#endif

SPINDLE_BEGIN_NAMESPACE

class GraphCompressor : SpindleAlgorithm { 
private:
  const Graph* sGraph;            // source graph
  SharedArray<int> fine2Coarse;     // compression map
  SharedArray<int> temp;
  int cNVtxs;                     // nVtxs compressed graph
  stopwatch executionTimer;
  stopwatch cGraphCreationTimer;

public:
  GraphCompressor();

  bool setGraph(const Graph* SGraph);
  Graph* createCompressedGraph();

  const SharedArray<int>* getFine2Coarse() const;
  SharedArray<int>* getFine2Coarse();
  
  virtual bool execute();
  virtual bool reset();
  
  int queryCompressedNVtxs() const;
  const stopwatch& getExecutionTimer() const { return executionTimer; }
  const stopwatch& getCGraphCreationTimer() const { return cGraphCreationTimer; }

private:
  int createCompressionMap();
  int permuteCompressionMap();
  int computeCompressedGraphStorage() const ;
  bool computeCompressedGraphIndices( Graph* cgraph ) const ;
  bool computeCompressedGraphWeights( Graph* cgraph ) const ;

SPINDLE_DECLARE_DYNAMIC( GraphCompressor )

};

SPINDLE_END_NAMESPACE

#endif
