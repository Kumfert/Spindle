//
// Graph.h
//
// $Id: Graph.h,v 1.2 2000/02/18 01:31:45 kumfert Exp $
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

#ifndef SPINDLE_GRAPH_H_
#define SPINDLE_GRAPH_H_

#ifndef SPINDLE_H_
#include "spindle/spindle.h"
#endif

#ifndef SPINDLE_GRAPH_BASE_H_
#include "spindle/GraphBase.h"
#endif

#ifndef SPINDLE_PERSISTANT_H_
#include "spindle/SpindlePersistant.h"
#endif

#ifdef MATLAB_MEX_FILE
extern "C" { 
#include "mex.h"
} // end extern "C"
#endif

SPINDLE_BEGIN_NAMESPACE

class MatrixBase;

class Graph: public GraphBase, public SpindlePersistant { 

  SPINDLE_DECLARE_PERSISTANT( Graph )
  
public:
  typedef const int* const_iterator;

  //constructors/destructors
  Graph();
  Graph( const int NVtxs, int *AdjHead, int *AdjList ); //assume ownership of arrays, unweighted
  Graph( const int NVtxs, const int *AdjHead, const int *AdjList ); // do not own arrays, unweighted
  Graph( const int NVtxs, int *AdjHead, int *AdjList, int *VtxWeights, int* EdgeWeights );
  Graph( const int NVtxs, const int *AdjHead, const int *AdjList, const int *VtxWeights, const int* EdgeWeights );
  Graph( GraphBase* inputGraph );
  Graph( const GraphBase* inputGraph );
  Graph( MatrixBase* inputMatrix );
  Graph( const MatrixBase* inputMatrix );
#ifdef MATLAB_MEX_FILE
  Graph( const mxArray* mat ); // constructor on matlab's Matrix
#endif 
  virtual ~Graph();

  //iterate over adjacency list of vertex i
  const_iterator begin_adj( int i ) const { return  (isValid()) ? (adjList.begin() + adjHead[i]) : 0 ; }
  const_iterator   end_adj( int i ) const { return  (isValid()) ? (adjList.begin() + adjHead[i+1]) : 0 ; }
  
  //iterate over low adjacency list of vertex i
  const_iterator begin_ladj( int i ) const { return  (isValid()) ? (adjList.begin() + adjHead[i]) : 0 ; }
  const_iterator   end_ladj( int i ) const { return  (isValid()) ? (adjList.begin() + adjSelf[i]) : 0 ; }

  //iterate over high adjacency list of vertex i
  const_iterator begin_hadj( int i ) const { return  (isValid()) ? (adjList.begin() + adjSelf[i]) : 0 ; }
  const_iterator   end_hadj( int i ) const { return  (isValid()) ? (adjList.begin() + adjHead[i+1]) : 0 ; }

  // required for setting variables
  virtual void validate();
  virtual void dump( FILE * fp ) const ;
  virtual bool reset();
  void prettyPrint( FILE * fp ) const ;
  //  bool isValid() const { return isValid(); } defined by SpindlePersistant

  const SharedArray<int>& getAdjHead()  const{ return adjHead; }
  const SharedArray<int>& getAdjList()  const{ return adjList; }
  const SharedArray<int>& getAdjSelf()  const{ return adjSelf; } 
  const SharedArray<int>& getVtxWeight()  const{ return vtxWeight; }
  const SharedArray<int>& getEdgeWeight()  const{ return edgeWeight; }

  SharedArray<int>& getAdjHead() { currentState=UNKNOWN; return adjHead; }
  SharedArray<int>& getAdjList() { currentState=UNKNOWN; return adjList; }
  SharedArray<int>& getAdjSelf() { currentState=UNKNOWN; return adjSelf; } 
  SharedArray<int>& getVtxWeight() { currentState=UNKNOWN; return vtxWeight; }
  SharedArray<int>& getEdgeWeight() { currentState=UNKNOWN; return edgeWeight; }

  bool setGhostNodes( const int nBoundVtxs, const int nCutEdges );
};

SPINDLE_END_NAMESPACE

#endif
