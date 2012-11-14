//
// EliminationForest.cc
//
//  $Id: EliminationForest.cc,v 1.2 2000/02/18 01:31:44 kumfert Exp $
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
//

#include "spindle/EliminationForest.h"

#ifndef SPINDLE_PERMUTATION_H_
#include "spindle/PermutationMap.h"
#endif

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

SPINDLE_IMPLEMENT_PERSISTANT( EliminationForest, GenericForest )

EliminationForest::EliminationForest() {
  g = 0;
  currentState = EMPTY;
}

EliminationForest::EliminationForest( const Graph* graph ) {
  incrementInstanceCount( EliminationForest::MetaData );
  g = graph;
  // set a null parent and ancestor
  const int n = g->size();
  SharedArray<int> ancestor( n );
  ancestor.init(-1);
  parent.resize(0);
  GenericForest::resize(n);
  parent.resize(n);
  parent.init(-1);
  
  for( int i=0; i<n; ++i) {
    for( const int* jj=g->begin_adj(i), *stop_jj = g->end_adj(i); 
	 jj < stop_jj; ++jj ) {
      int j = *jj;
      if ( j < i ) { // ii is an ancestor of jj
	// find the root of the tree containing the node `jj'
	while( (ancestor[j]!=-1) & (ancestor[j]!=i) ) {
	  int temp = ancestor[j];
	  ancestor[j] = i; 
	  j = temp;
	}
	// make node `ii' the root of this subtree, if not already true
	if ( ancestor[j] == -1 ) {
	  ancestor[j] = i;
	  parent[j] = i;
	}
      } // end if `ii' is ancestor of `jj'
    } // end for all jj adj to ii
  } // end for all ii
  currentState = UNKNOWN;
  createChildSibling();
  createDoubleLink();
}


EliminationForest::EliminationForest( const Graph* graph, const SharedPtr<PermutationMap>& perm ) {
  incrementInstanceCount( EliminationForest::MetaData );
  g = graph;
  
  // set a null parent and ancestor
  const int n = g->size();
  SharedArray<int> ancestor( n );
  ancestor.init(-1);
  GenericForest::resize(n);
  parent.resize(n);
  parent.init(-1);
  
  const int* old2new = perm->getOld2New().lend();
  const int* new2old = perm->getNew2Old().lend();
  
  // programmer's note:  `ii' & `jj' refer to the vertices in the new ordering
  //                      `i' & `j' refer to the same vertices in the old ordering
  
  for( int ii=0; ii<n; ++ii) {
    int i = new2old[ii];
    for( const int* jp=g->begin_adj(i), *stop_jp = g->end_adj(i); 
	 jp < stop_jp; ++jp ) {
      int j = *jp;
      int jj = old2new[ j ];
      if ( jj < ii ) { // ii is an ancestor of jj
	/* old code.  put the tree in terms of new indices, not old
	// find the root of the tree containing the node `jj'
	while( (ancestor[jj]!=-1) & (ancestor[jj]!=ii) ) {
	  int temp = ancestor[jj];
	  ancestor[jj] = ii; 
	  jj = temp;
	}
	// make node `ii' the root of this subtree, if not already true
	if ( ancestor[jj] == -1 ) {
	  ancestor[jj] = ii;
	  parent[jj] = ii;
	}
	*/
	// find the root of the tree containing the node `j'
	while( (ancestor[j]!=-1) & (ancestor[j]!=i) ) {
	  int temp = ancestor[j];
	  ancestor[j] = i; 
	  j = temp;
	}
	// make node `i' the root of this subtree, if not already true
	if ( ancestor[j] == -1 ) {
	  ancestor[j] = i;
	  parent[j] = i;
	}
      } // end if `ii' is ancestor of `jj'
    } // end for all jj adj to ii
  } // end for all ii
  currentState = UNKNOWN;
  createChildSibling();
  createDoubleLink();
}// end


EliminationForest::EliminationForest( const Graph* graph, const int n, const int* Parent )
  : GenericForest( n, Parent ) {
    incrementInstanceCount( EliminationForest::MetaData );
    g = graph;
}

EliminationForest::~EliminationForest() {
  decrementInstanceCount( EliminationForest::MetaData );
}

#define __FUNC__ "void EliminationForest::loadObject( SpindleArchive & ar )"
void
EliminationForest::loadObject( SpindleArchive & ar ) {
  ERROR( SPINDLE_ERROR_UNIMP_FUNC , "Method not yet implemented.");
  if ( &ar );
}
#undef __FUNC__

#define __FUNC__ "void EliminationForest::loadObject( SpindleArchive & ar )"
void
EliminationForest::storeObject( SpindleArchive& ar ) const {
  ERROR( SPINDLE_ERROR_UNIMP_FUNC , "Method not yet implemented.");
  if( &ar ) ;
}
#undef __FUNC__

  
