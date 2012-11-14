//
//
//

#include "spindle/GraphBase.h"

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

bool 
GraphBase::resize( const int newNVtxs, const int newNEdges, const int newNBoundVtxs=0, const int newNCutEdges=0 ) {
  
  adjHead.resize( newNVtxs + newNBoundVtxs + 1 );
  adjSelf.resize( newNVtxs + newNBoundVtxs );
  adjList.resize( newNEdges + newNCutEdges );
  if ( vtxWeight.notNull() ) { 
    vtxWeight.resize( newNVtxs );
  }
  if ( edgeWeight.notNull() ) { 
    edgeWeight.resize( 2 * newNEdges );
  }
  
  nVtxs = newNVtxs;
  nEdges = newNEdges;
  nBoundVtxs = newNBoundVtxs;
  nNonZeros = newNCutEdges + 2*newNEdges;

  return true;
}

int
GraphBase::deg( const int i ) const {
  if ( ( i>=0 ) && (i<nVtxs) ) {
    if ( vtxWeight.size() == 0 ) {
      return ( adjHead[i+1] - adjHead[i] );
    } else { 
      int temp = 0;
      for (int j = adjHead[i]; j<adjHead[i+1]; ++j) {
	if ( adjList[j] != i ) {
	  temp += vtxWeight[ adjList[ j ] ];
	}
      }
      return temp;
    }
  } else {
    return -1;
  }
}
