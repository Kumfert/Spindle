//
// SymbolicFactorization.h
//
// $Id: SymbolicFactorization.cc,v 1.2 2000/02/18 01:32:03 kumfert Exp $
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

#include "spindle/SymbolicFactorization.h"

#ifndef SPINDLE_GRAPH_H_
#include "spindle/Graph.h"
#endif

#ifndef SPINDLE_ARRAY_E_FOREST_H_
#include "spindle/EliminationForest.h"
#endif

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

static char invalidState[] = "Invalid algorithmicState detected";
static char invalidGraph[] = "Invalid graph detected";
static char invalidETree[] = "Invalid etree detected";
static char invalidPerm[]  = "Invalid permutation detected";
static char runtimeErr[]   = "Unspecified execution error detected";
static char invalidPermFromEtree[] = "execution error detected: problem creating postordering of etree";

SymbolicFactorization::SymbolicFactorization() {
  incrementInstanceCount( SymbolicFactorization::MetaData );
  reset();
}


SymbolicFactorization::~SymbolicFactorization() {
  decrementInstanceCount( SymbolicFactorization::MetaData );
}

bool
SymbolicFactorization::reset() {
  graph = 0;
  algorithmicState= EMPTY;
  return true;
}

bool
SymbolicFactorization::setGraph( const Graph* g ) {
  if ( algorithmicState==INVALID ) { 
    return false; 
  }
  if ( ( g==0 ) || (! g->isValid()) ) { 
    algorithmicState = INVALID;
    errMsg = invalidGraph;
    return false; 
  }

  switch( algorithmicState ) {
  case DONE:
  case READY:
    reset(); // resets perm and/or etree
    // no break;
  case EMPTY:
    graph = g;
    algorithmicState = READY;
    break;
  default:
    algorithmicState = INVALID;
    errMsg = invalidState;
    return false;
  }
  return true;
}

bool
SymbolicFactorization::setETree( const ETree* et ) {
  if ( algorithmicState == INVALID ) { 
    return false; 
  }
  if ( (et == 0) || (!et->isValid()) ) { 
    algorithmicState = INVALID;
    errMsg = invalidETree;
    return false;
  }
  if ( algorithmicState == READY ) {
    if ( et->size() == graph->size() ) {
      etree.borrow( et ); // requires only const access 
      if ( perm.notNull() ) {
	perm->reset();
      }
      return true;
    }
  }
  return false;
}



bool
SymbolicFactorization::setPermutationMap( const PermutationMap* p ) {
  if ( algorithmicState == INVALID ) { 
    return false; 
  }
  if ( (p == 0) || (! p->isValid()) ) { 
    algorithmicState = INVALID;
    errMsg = invalidPerm;
    return false;
  }
  if ( algorithmicState == READY ) {
    if ( p->size() == graph->size() ) {
      perm.borrow( p ); // require only const-access
      if ( etree.notNull() ) {
	etree->reset();
      }
      return true;
    }
  }
  return false;
}


bool
SymbolicFactorization::execute() {
  if ( algorithmicState == INVALID ) { return false; }
  if ( algorithmicState != READY ) { return false; }
  
  // First we need an etree.
  if ( perm.isNull()  && etree.isNull() ) {  // if need etree and perm
    // create an etree based on the implicit ordering of the matrix
    etree.take( new ETree( graph ) );
    etree->validate();
  } else if ( perm.notNull() ) { 
    // create an elimination tree based on the permutation
    etree.take( new ETree( graph, perm ) );
    etree->validate();
  }

  if ( !etree.lend()->isValid() ) {
    algorithmicState = INVALID;
    errMsg = invalidETree;
    return false;
  }
  // ASSERT: now have a valid etree.

  // Now that we have an etree, construct a valid Permutation
  // that is a postordering of the etree
  perm.take( new PermutationMap( graph->size() ) );
  const SharedPtr<ETree>& const_etree = etree; // HACK to force const-access
  //  int * dest = perm->getNew2Old().begin();
  //  for( ETree::const_postorder_iterator it = const_etree->begin_postorder(), 
  //	 stop_it = const_etree->end_postorder(); it != stop_it; ++it ) { 
  //    *dest++ = *it;
  //    cout << *it << endl;
  //  }
  copy( const_etree->begin_postorder(), const_etree->end_postorder(), 
	perm->getNew2Old().begin() ); 
  perm->validate();
  if ( ! perm->isValid() ) {
    algorithmicState = INVALID;
    errMsg = invalidPermFromEtree;
    return false;
  }   
  // ASSERT: we now have a valid permutation and etree
  
  bool okay = run ();
  if ( !okay ) {
    algorithmicState = INVALID;
    if ( errMsg == 0 ) {
      errMsg = runtimeErr;
    }
  }
  return okay;
}

bool
SymbolicFactorization::run() {
  // PRECONDITION:  graph, permutation, and etree are all valid
  // POSTCONDITION: colCount and rowCount contain # nonzeros in factor
  //                graph, permutation and etree are unchanged

  // HACK to force const access.
  const SharedPtr<PermutationMap>& const_perm = perm;
  const SharedPtr<ETree>& const_etree = etree;

  const int n = graph->size();

  // get const access to perm, invp, and parent pointer
  register const int* old2new = const_perm->getOld2New().lend();
  register const int* new2old = const_perm->getNew2Old().lend();
  register const int* parent = const_etree->getParent().lend();

  // resize rowCount and colCount, initialize them to all 1's
  rowCount.resize(n);
  rowCount.init(1);
  colCount.resize(n);
  colCount.init(1);

  register int * row_count = rowCount.begin();
  register int * col_count = colCount.begin();

  // get a temporary array for the vertex coloring
  SharedArray<int> color(n);
  color.init(-1);
  register int * color_array = color.begin();

  // programmer's note: `ii' & `jj' refers to vertices in the new ordering
  //                     `i' &  `j' refers to the same vertices in the old ordering.
  for( int ii=0; ii<n; ++ii ) {
    color_array[ii] = ii;
    
    // get the row index in the original matrix
    int i = new2old[ii];

    // for each element in the corresponding adjacency list
    for(const int *jp = graph->begin_adj(i), *stop_jp = graph->end_adj(i);
	 jp < stop_jp; ++jp ) {
      int j = *jp;
      int jj = old2new[ j ];
      
      if ( jj < ii ) { // ii is an ancestor of jj in the permutation
	while ( color_array[jj] < ii ) { 
	  row_count[ii]++;
	  col_count[jj]++;
	  color_array[jj] = ii;
	  jj = old2new[ parent[ j ] ]; // this is right
	  if ( jj == -1 ) { // if got to root.
	    break;
	  }
	  j = new2old[ jj ];
	} // end while
      } // end if
    } // end forall j adj to i
  } // end for all ii in new ordering
  algorithmicState = DONE;
  return true;
}

long
SymbolicFactorization::querySize() {
  const int n = colCount.size();
  if ( algorithmicState == DONE ) {
    long temp = 0;
    const int *cur = colCount.begin();
    for( int i=0; i<n; ++i ) {
      temp += *cur++;
    }
    return temp;
  } else {
    return false; 
  }
}

long
SymbolicFactorization::queryFill() {
  const int n = colCount.size();
  if ( algorithmicState == DONE ) {
    long temp = 0;
    const int *cur = colCount.begin();
    for( int i=0; i<n; ++i ) {
      temp += *cur++;
    }
    temp -=  graph->queryNEdges() + graph->queryNVtxs();
    return temp;
  } else {
    return false; 
  }
}

double
SymbolicFactorization::queryWork() {
  const int n = colCount.size();
  if ( algorithmicState == DONE ) {
    double temp = 0.0;
    for( int i=0; i<n; ++i ) {
      double j = colCount[i];
      temp += (j-1.0) * (j+2.0);
    }
    return temp;
  } else {
    return false; 
  }
}


SPINDLE_IMPLEMENT_DYNAMIC( SymbolicFactorization, SpindleAlgorithm )

