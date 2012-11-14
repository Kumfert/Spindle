//
// GraphUtils.cc
//
//  $Id: GraphUtils.cc,v 1.2 2000/02/18 01:31:45 kumfert Exp $
//
//  Gary Kumfert, Old Dominion University
//  Copyright(c) 1998, Old Dominion University.  All rights reserved.
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

#include "spindle/GraphUtils.h"

#ifndef SPINDLE_GRAPH_H_
#include "spindle/Graph.h"
#endif

#ifndef SPINDLE_PERMUTATION_MAP_H_
#include "spindle/PermutationMap.h"
#endif 

#ifndef SPINDLE_SCATTER_MAP_H_
#include "spindle/ScatterMap.h"
#endif

#ifdef REQUIRE_OLD_CXX_HEADER_SUFFIX
#include "algo.h" // sort
#else
#include <algorithm>
using std::sort;
#endif

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

Graph* 
GraphUtils::createPermutedGraph( const Graph* graph, const PermutationMap * perm ) { 
  // create a new graph based on the preceding one and a permutation.
  if ( ( !graph->isValid() ) || ( !perm->isValid() ) ) { 
    return 0;
  }
  const int * new2old = perm->getNew2Old().lend();
  const int * old2new = perm->getOld2New().lend();

  const int * oldAdjHead = graph->getAdjHead().lend();
  const int * oldAdjList = graph->getAdjList().lend();
  const int nVtxs = graph->queryNVtxs();

  int * newAdjHead = graph->getAdjHead().export();
  int * newAdjList = graph->getAdjList().export();
  
  // first copy the adjacency lists in the new order
  // ii jj kk, etc is the old numbers
  // i, j, k  corresponds to the new ones
  int curidx = 0;
  newAdjHead[0] = 0;
  for ( int i=0; i<nVtxs; ++i ) { 
    int ii = new2old[i];
    int nEdges = oldAdjHead[ii+1] - oldAdjHead[ii];
    newAdjHead[i+1] = curidx + nEdges;
    for (int jj = oldAdjHead[ii]; jj<oldAdjHead[ii+1]; ++jj ) {
      int kk = oldAdjList[jj];
      int k = old2new[ kk ];
      newAdjList[ curidx++ ] = k;
    }
    // now that we have the i'th row laid out, we need to sort
    sort( newAdjList + newAdjHead[i], newAdjList + newAdjHead[i+1] );
  }

  // now we only need to create the Graph

  Graph * g = new Graph( nVtxs, newAdjHead, newAdjList );  // graph class claims ownership of arrays.
  g->validate();
  return g;
}


bool 
GraphUtils::createSubgraph( const Graph* originalGraph, const int * mask, 
			    Graph ** subGraph, ScatterMap ** scatterMap,
			    bool includeGhostNodes ) { 
  
  // 1. Check arguments
  {
    if ( originalGraph == 0 ) {
      ERROR( SPINDLE_ERROR_ARG_BADPTR, "arg#1: originalGraph == NULL" );
      return false;
    } else if ( ! originalGraph->isValid() ){ 
      ERROR( SPINDLE_ERROR_ARG_WRONGSTATE, "arg#1: originalGraph not Valid" );
    } else if ( mask == 0 ) { 
      ERROR( SPINDLE_ERROR_ARG_BADPTR," arg#2: mask == NULL" );
      return false;
    }
  }
  const int nVtxs = originalGraph->queryNVtxs();

  // 2. Determine number of new vertices
  int newNVtxs = 0;
  { 
    for ( int i = 0; i<nVtxs; ++i ) { 
      if ( mask[i] ) { 
	++newNVtxs; 
      }
    }
  }
  
  // 2.5 Compute number of ghost nodes (if applicable)
  int nBoundVtxs = 0;
  int nCutEdges = 0;
  int * mask2= 0;
  const int * adjHead = originalGraph->getAdjHead().lend();
  const int * adjList = originalGraph->getAdjList().lend();
  if ( includeGhostNodes ) { 
    mask2 = new int[ nVtxs ];       //mask2 will be true for nodes and boundary
    copy( mask, mask+nVtxs, mask2 );
    for ( int i=0; i<nVtxs; ++i ) {
      if ( mask[i] ) { 
	for ( int j = adjHead[i]; j<adjHead[i+1]; ++j ) {  // for k = adj(i)
	  int k = adjList[j];
	  if (!mask[k]) { 
	    ++nCutEdges;
	    if (!mask2[k]) { 
	      mask2[k] = 1;
	      ++nBoundVtxs;
	    } // end if !mask[k]
	  } // end if !mask2[k]
	} // end for k = adj(i)
      } // end if mask[i]
    } // end for all i in nVtxs
  } // end if include GhostNodes
  
  // 3. Create loc2glob array
  ScatterMap *scatter_map;
  { 
    int * loc2glob = new int[ newNVtxs + nBoundVtxs ];
    int curIdx = 0;
    for ( int i = 0; i<nVtxs; ++i ) { 
      if ( mask[ i ] ) { 
	loc2glob[ curIdx++ ] = i;
      }
    }
    if ( includeGhostNodes ) { 
      for ( int i = 0; i<nVtxs; ++i ) { 
	if ( (!mask[i]) && (mask2[i]) ) {  // if its a ghost node.
	  loc2glob[ curIdx++ ] = i;
	}
      }
    }
    scatter_map = new ScatterMap( newNVtxs + nBoundVtxs, loc2glob );
    // scatter_map seizes ownership of loc2glob
    scatter_map->validate();
    if ( !scatter_map->isValid() ) { 
      ERROR( SPINDLE_ERROR_MISC, "Could not create valid scatter_map" );
      delete scatter_map;
      scatter_map = 0;
      return false;
    }
  }
  // We don't need to mess with scatter_map anymore.
  const ScatterMap * const_scatter_map = scatter_map;

  // Now determine number of nonzeros in the new graph
  // and create the newAdjHead[]
  int * newAdjHead = new int[ newNVtxs + nBoundVtxs + 1 ];
  const int * loc2glob = const_scatter_map->getLoc2Glob().lend();
  const ScatterMap::glob2loc_t &glob2loc = *(const_scatter_map->getGlob2Loc().lend());
  { 
    newAdjHead[0] = 0;
    for (int ii = 0; ii<newNVtxs + nBoundVtxs; ++ii ) { 
      int i = loc2glob[ii];
      int deg_ii = 0;
      for ( int j = adjHead[i]; j<adjHead[i+1]; ++j ) { 
	if ( mask[ adjList[ j ] ] ) { 
	  ++deg_ii;
	}
      }
      newAdjHead[ii+1] = newAdjHead[ii] + deg_ii;
    }
  }

  // Now create the adjList array.
  int * newAdjList = new int[ newAdjHead[ newNVtxs ] + nCutEdges ];
  { 
    int curIdx = 0;
    for (int ii = 0; ii<newNVtxs + nBoundVtxs ; ++ii ) { 
      int i = loc2glob[ii];
      for ( int k = adjHead[i]; k<adjHead[i+1]; ++k ) { 
	int j = adjList[ k ];
	if ( mask[ j ] ) {
	  ScatterMap::glob2loc_t::const_iterator iter= glob2loc.find(j);
	  if ( iter != glob2loc.end() ) {
	    int jj = (*iter).second;
	    newAdjList[ curIdx++ ] = jj;
	  }
	}
      }
    }
    if ( curIdx != newAdjHead[ newNVtxs + nBoundVtxs ] ) { 
      WARNING(" curIdx = %d != newAdjHead[ newNVtxs + nBoundVtxs] = %d", 
	      curIdx, newAdjHead[ newNVtxs + nBoundVtxs  ] );
    }
  }

  // Finally, create the sub graph
  Graph * sub_graph = new Graph( newNVtxs + nBoundVtxs , newAdjHead, newAdjList );
  if ( nBoundVtxs > 0 ) { 
    if (!sub_graph->setGhostNodes( nBoundVtxs, nCutEdges ) ) {
      ERROR( SPINDLE_ERROR_MISC,"Graph won't set ghost nodes");
    }
  }

  delete[] mask2;

  sub_graph->validate();
  if ( !sub_graph->isValid() ) { 
    delete sub_graph;
    sub_graph = 0;
    delete scatter_map;
    scatter_map = 0;
    ERROR( SPINDLE_ERROR_MISC, "Cannot create valid subgraph" );
    return false;
  }
  *subGraph = sub_graph;
  *scatterMap = scatter_map;

  return true;
}

