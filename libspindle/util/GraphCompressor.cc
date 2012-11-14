//
// GraphCompressor.cc
//
// $Id: GraphCompressor.cc,v 1.2 2000/02/18 01:32:03 kumfert Exp $
//
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



#include "spindle/GraphCompressor.h"

#ifndef SPINDLE_SYSTEM_H_
#include "spindle/SpindleSystem.h"
#endif

#include "pair.h"
#include "algo.h"
#include "spindle/spindle_func.h"

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

#ifdef __FUNC__
#undef __FUNC__
#endif

GraphCompressor::GraphCompressor() {
  reset();
}

int
GraphCompressor::queryCompressedNVtxs() const {
  if ( algorithmicState != DONE ) {
    return -1;
  } else { 
    return cNVtxs;
  }
}


#define __FUNC__ "bool GraphCompressor::setGraph( const Graph* SGraph )"
bool 
GraphCompressor::setGraph( const Graph* SGraph ) {
  if (algorithmicState == INVALID ) {
    WARNING("Must reset() from ERROR state");
    return false;
  }
  if ( SGraph->isValid() ) {
    sGraph = SGraph;
    fine2Coarse.resize( sGraph->queryNVtxs() );
    algorithmicState = READY;
    return true;
  } 
  return false;
}
#undef __FUNC__

bool
GraphCompressor::reset() {
  algorithmicState = EMPTY;
  sGraph = 0;
  cNVtxs = 0;
  executionTimer.reset();
  cGraphCreationTimer.reset();
  return true;
}

const SharedArray<int>*
GraphCompressor::getFine2Coarse() const {
  if ( algorithmicState == DONE ) {
    return &fine2Coarse; 
  } else {
    return 0;
  }
}

SharedArray<int>*
GraphCompressor::getFine2Coarse() {
  if ( algorithmicState == DONE ) {
    return &fine2Coarse;
  } else {
    return 0;
  }
}

bool 
GraphCompressor::execute() {
  if ( algorithmicState != READY ) { return false; } 
  executionTimer.start();
  cNVtxs = createCompressionMap();
  executionTimer.stop();
  if ( cNVtxs != 0 ) {
    algorithmicState = DONE;
    permuteCompressionMap();
    return true;
  } else {
    algorithmicState = INVALID;
    return false;
  }
}


#define __FUNC__ "Graph* GraphCompressor::createCompressedGraph() const"
Graph* 
GraphCompressor::createCompressedGraph()  {
  if ( algorithmicState != DONE ) { return 0; }
  cGraphCreationTimer.start();
  int cN = cNVtxs;
  int cNNZ = computeCompressedGraphStorage();
  SharedArray<int> cAdjHead( cN + 1 );
  cAdjHead[cN] = cNNZ;  // set number of non-zeros for graph constructor
  SharedArray<int> cAdjList( cNNZ );
  Graph* cGraph = new Graph( cN, cAdjHead.give(), cAdjList.give() );
  computeCompressedGraphIndices( cGraph );
  computeCompressedGraphWeights( cGraph );
  cGraph->validate();
  cGraphCreationTimer.stop();
  if ( ! cGraph->isValid() ) {
    ERROR( SPINDLE_ERROR, "Cannot create valid compressed graph.  ");
    delete cGraph;
    return 0;
  } else {
    return cGraph;
  }
}
#undef __FUNC__


int
GraphCompressor::createCompressionMap() {
  //
  // initialize data
  //
  typedef vector< pair< int , int > > SorterType;

  const int n = sGraph->size();
  const int * adjHead = sGraph->getAdjHead().lend();
  const int * adjList = sGraph->getAdjList().lend();
  int curCompressedVtx = 0;
  SorterType sorter;

  sorter.reserve(n);
  fine2Coarse.resize( n );
  fine2Coarse.init( -1 );
  temp.resize( n );
  temp.init(-1);

  // hash the adjacency lists
  { 
    // purify complains here using egcs 1.1.x.  Don't know why
    for( int i=0; i<n; ++i ) {
      int hashValue = (adjHead[i] == adjHead[i+1]) ? 0 : i+1;
      // if item has no adjacencies, mark its hash value as 0.
      // Thus, they will all get merged into one node
      for( int jj=adjHead[i]; jj<adjHead[i+1]; ++jj ) {
	hashValue += (adjList[jj] + 1) ;  // add one to distinguish from zero
      }
      sorter.push_back( pair<int,int>( hashValue, i ) );
    }
  }

  //
  // Sort the vertices by their hash number
  //
  sort( sorter.begin(), sorter.end(),  pair_first_less< int , int >() );

  // 
  // Now examine pairs of vertices with the same hash value
  //
  SorterType::const_iterator cur = sorter.begin();
  SorterType::const_iterator candidate;
  SorterType::const_iterator stop = sorter.end();
  int * adjTo = temp.begin();

  for( ; cur != stop; ++cur ) {  // for all vtxs in sorter
    int i = (*cur).second;
    if ( fine2Coarse[i] >= 0 ) { // skip if its already been mapped
      continue; 
    }
    fine2Coarse[i] = curCompressedVtx;
    int hash_i = (*cur).first;   // get hash value for current
    int nadj_i = adjHead[i+1] - adjHead[i];
    bool scatteredAdjList = false; // 
    for( candidate = cur + 1;( candidate != stop) && ( hash_i == (*candidate).first )  ;
	 ++candidate ) { 
      // for all other vertices in sorter with same hash value
      int j = (*candidate).second;
      if ( nadj_i != (adjHead[j+1] - adjHead[j]) ) { 
	// if i and j don't have same size adjlist
	continue;  // j cannot be compressed into i
      }
      if ( !scatteredAdjList ) { // if haven't flagged all my adj vtxs in adjTo
	adjTo[ i ] = i;          // do so now.
	for( int k = adjHead[i], stop_k = adjHead[i+1]; k < stop_k; ++k ) { 
	  adjTo[ adjList[ k ] ] = i;
	}
	scatteredAdjList = true;
      }
      // now check if j shares all the same adjacencies
      if ( adjTo[ j ] != i ) { 
	continue;
      }
      bool compressThem = true;
      {for( int k = adjHead[j], stop_k = adjHead[j+1]; k < stop_k; ++k ) { 
	if ( adjTo[ adjList[ k] ] != i ) { 
	  compressThem = false;
	  break;
	}
      }}
      if ( compressThem == true ) { 
	fine2Coarse[j] = curCompressedVtx;
      }
    } // end for all candidates of the same  hash value
    ++curCompressedVtx;
  } // end for all vertices
  return curCompressedVtx;
}
    
/*
    if ( (*cur).first = 


  // group "indistinguishable" vertices
  int curCompressedVtx = 0;
  int * fine2coarse = fine2Coarse.begin();
  {for(int i=0; i<n; ++i ) {
    if ( fine2coarse[i] == -1 ) {
      fine2coarse[i] = curCompressedVtx;
      int hash_i = hash[i];
      for(int jj=adjSelf[i]; jj<adjHead[i+1]; ++jj) {
	int j = adjList[jj];
	if ( ( i!=j) && (hash_i == hash[j])) {
	  fine2coarse[j] = curCompressedVtx;
	}
      }
      curCompressedVtx++;
    }
  }}
  return curCompressedVtx;
}
*/
int 
GraphCompressor::permuteCompressionMap() {
  int n = sGraph->size();
  // change fine2coarse so that the compression
  // is sorted.  That is 0->0 and 1->0 or 1, etc.
  int * fine2coarse = fine2Coarse.begin();
  SharedArray< int > old2new( cNVtxs );
  old2new.init( -1 );
  int newCoarseNumber = 0;
  // first fill temp_map with 
  for( int i=0; i<n; ++i ) { 
    int i_C = fine2Coarse[ i ];
    if ( old2new[ i_C ] == -1 ) { 
      old2new[ i_C ] = newCoarseNumber;
      ++newCoarseNumber;
    }
  }
  for ( int i=0; i<n; ++i ) { 
    fine2coarse[ i ] = old2new[ fine2coarse[ i ] ];
  }
  return 0;
}

int 
GraphCompressor::computeCompressedGraphStorage() const {
  int n = sGraph->size();
  int i_C = -1;
  int nnz = 0;
  const int * adjHead = sGraph->getAdjHead().lend();
  const int * adjList = sGraph->getAdjList().lend();
  const int * fine2coarse = fine2Coarse.lend();
  for(int i=0; i<n; ++i) {
    if( fine2coarse[i] > i_C ) { // if new vertex
      i_C = fine2coarse[i];
      int j_C = -1;
      for( int jj=adjHead[i]; jj<adjHead[i+1]; ++jj ) {
	int j = adjList[jj];
	if ( fine2coarse[j] == i_C ) { // if edge becomes a self edge
	  continue; // skip it
	} 
	if ( fine2coarse[j] > j_C ) { // if new edge
	  j_C = fine2coarse[j];
	  ++nnz;
	  }
      }
    }
  }
 return nnz;
}


#define __FUNC__ "int GraphCompressor::computeCompressedGraphIndices( Graph* cGraph ) const"
bool 
GraphCompressor::computeCompressedGraphIndices( Graph* cGraph ) const {
  const int n = sGraph->queryNVtxs();
  int i_C = -1;
  int idx_C = 0;
  const int* adjHead = sGraph->getAdjHead().lend();
  const int* adjList = sGraph->getAdjList().lend();
  int cN = cGraph->getAdjHead().size() - 1;
  //  int cNNZ = cGraph->getAdjList().size(); // unused 
  int *cAdjHead = cGraph->getAdjHead().begin();
  int *cAdjList = cGraph->getAdjList().begin();
  const int *fine2coarse = fine2Coarse.lend();

  {for(int i=0; i<n; ++i ) {
    if( fine2Coarse[i] > i_C ) { // new vertex
      i_C = fine2Coarse[i]; // assert: equivalently i_C++
      int j_C = -1;
      cAdjHead[i_C] =  idx_C;
      for(int jj=adjHead[i]; jj<adjHead[i+1]; ++jj ) {
	int j = adjList[jj];
	if ( fine2coarse[j] == i_C ) { // if new edge becomes self edge
	  continue; // skip it
	}
	if ( fine2coarse[j] > j_C ) { // new edge
	  j_C = fine2Coarse[j];
	  cAdjList[idx_C++] = j_C;
	}
      }
    }
  }}
  cAdjHead[cN] = idx_C;
  return true;
}
#undef __FUNC__

#define __FUNC__ "bool GraphCompressor::computeCompressedGraphWeights( Graph* cgraph ) const"
bool 
GraphCompressor::computeCompressedGraphWeights( Graph* cgraph ) const {
  int n = sGraph->queryNVtxs();
  int cN = cgraph->getAdjHead().size() - 1;
  //  int cNNZ = cgraph->getAdjList().size(); // unused ;
  const int * fine2coarse = fine2Coarse.begin();

  cgraph->getVtxWeight().resize( cN );
  cgraph->getVtxWeight().init(0);
  int * vtxWeight = cgraph->getVtxWeight().begin();

  if ( sGraph->getVtxWeight().size() == 0 ) { 
    // create weights from unweighted graph
    for( int i=0; i<n; ++i ) {
      ++vtxWeight[ fine2coarse[i] ];
    }
  } else {                                     
    // create new vertex weights from vertex weighted graph
    const int * sVtxWeight = sGraph->getVtxWeight().lend();
    for( int i=0; i<n; ++i ) {
      vtxWeight[ fine2coarse[i] ] += sVtxWeight[i];
    }
  }
  if ( sGraph->getEdgeWeight().size() != 0 ) { 
    ERROR( SPINDLE_ERROR_UNIMP_FUNC, "Graph compression for edge weights not implemented" );
  }
  return true;
}
#undef __FUNC__

SPINDLE_IMPLEMENT_DYNAMIC( GraphCompressor, SpindleAlgorithm )

/* 
void 
GraphCompressor::createINVMap(const int n, const int nC) {
  // first find the max in fine2Coarse;
  INVptr.reserve( nC + 1);
  INVptr.assign( nC + 1, 0 );
  INVidx.reserve( n );
  temp.reserve( nC );
  
  // now construct reverse map
  {for(int i=0;i<n;++i) {
    INVptr[fine2Coarse[i]]++;
  }}
  {for(int i=0;i<nC;++i) {
    INVptr[i+1]+=INVptr[i];
    temp[i] = INVptr[i];
  }}
  {for(int i=0;i<n;++i) {
    int iC = fine2Coarse[i];
    INVidx[ temp[iC]++ ] = i;
  }}
  
  // check
  // {for(i=0;i<nC;++i) {
  //   if( temp[i] != INVptr[i+1]) ERROR("Indices do not add up!");
  // }}
  return;
}

SharedPtr<PermutationMap>& 
GraphCompressor::createUncompressedPermutationMap( const PermutationMap& perm ) {
  if ( ( algorithmicState != DONE ) || ( !perm.isValid() ) ) {
    return SharedPtr<PermutationMap> p(0);
  } else {
    const int n = fine2Coarse.size();
    const int nC = perm.size();
    createINVMap(n,nC);

    PermutationMap* perm = new PermutationMap(n);
    int* old2new = perm->getOld2New().give();
    
    // Now that we have the INV matrix formed, we can extract
    // the ordering for the vertices in each supernode
    int nextidx = 0;
    {for(int i=0;i<nC;++i) {
      for(int jj=INVptr[i]; jj<INVptr[i+1]; ++jj ) {
	j = INVind[jj];
	old2new[j] = nextidx++;
      }
    }}

    perm->getOld2New().take( old2new );
    perm->validate();
    return SharedPtr<PermutationMap> ptr(perm);
    
  } // end else  
}
*/
