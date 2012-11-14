//
// GraphBase.h
//
//
//  Graphs are structurally symmetric, but not necessarily numerically
//  symmetric.  In the latter case, edge weights are neccessary if for
//  nothing else than to hold ones and zeros
//
//  Furthermore, graphs can be weighted
//
//  Furthermore, graphs can be partitioned.  
//  A partitioned graph stores the list of cut edges and vertices.
//
//  Not all graph algorithms are guaranteed to recognize a graph with boundary
//  vertices
//
//            nVtxs nBndryVtxs
//           +-----+---+
//           |     |   |
//           |     |   |
//           |     |   |
//           +-----+---+
//
#ifndef SPINDLE_GRAPH_BASE_H_
#define SPINDLE_GRAPH_BASE_H_

#ifndef SPINDLE_H_
#include "spindle/spindle.h"
#endif

#ifndef SPINDLE_AUTO_ARRAY_H_
#include "spindle/SharedArray.h"
#endif

SPINDLE_BEGIN_NAMESPACE
//class Graph;
class GraphBase {
  // friend class Graph;
protected:
  int nVtxs;      // number of vertices in graph... not including boundary vtxs
  int nEdges;     // number of edges in the graph ... not including cut edges
  int nBoundVtxs; // number of boundary vertices
  int nNonZeros;

  SharedArray<int> adjHead;   // length = nVtxs + nBoundVtxs + 1;
  SharedArray<int> adjSelf;   // length = nVtxs + nBoundVtxs;
  SharedArray<int> adjList;   // length = 2 (nEdges + nCutEdges) 

  SharedArray<int> vtxWeight; // length = nVtxs or zero
  SharedArray<int> edgeWeight;// length = 2 * nEdges or zero
  
  GraphBase() { } // protected default constructor

public:

  bool resize( const int newNVtxs, const int newNEdges, const int newNBoundVtxs=0, const int newNCutEdges=0 );

  int queryNVtxs() const { return nVtxs; }
  int queryNEdges() const { return nEdges; }
  int queryNNonZeros() const { return 2*nEdges; }
  int queryNBoundVtxs() const { return nBoundVtxs; }
  int queryNCutEdges() const { return adjList.size() - 2*nEdges; }

  int size() const { return  nVtxs; }
  int deg(const int i) const;
 
  //data structure access
  const SharedArray<int>& getAdjHead() const { return adjHead; }
  const SharedArray<int>& getAdjList() const { return adjList; }
  const SharedArray<int>& getAdjSelf() const { return adjSelf; } 
  const SharedArray<int>& getVtxWeight() const { return vtxWeight; }
  const SharedArray<int>& getEdgeWeight() const { return edgeWeight; }
};

SPINDLE_END_NAMESPACE

#endif
