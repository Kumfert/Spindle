//
//  adt/Graph/drivers/tests/test12C.cc -- tests QuotientGraph outmatching
//
//  $Id: test12.cc,v 1.2 2000/02/18 01:32:06 kumfert Exp $
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
// tests QuotientGraph
// 

#include "spindle/Graph.h"

#include "spindle/QuotientGraph.h"

//   eliminate 2 and 4, and 0 and 5 should compress
//   and should outmatch vtx 1.
//
//   when the (0,5) supernode is eliminated, vtx 1 should
//   come back.
//
//                      2                                                          
//                     /|\                                                             
//                    / | \                                                             
//                   /  |  \                                                           
//                  0   5   1-----3                                                    
//                   \  |  /                                                          
//                    \ | /                                                           
//                     \|/ 
//                      4                                                      

int nvtxs = 6;
int Aptr[] = {0,3,7,11,13,17,20};
int Aind[] = {0,2,4,1,2,3,4,0,1,2,5,1,3,0,1,4,5,2,4,5};

#include <iostream.h>
#include <iomanip.h>

#ifdef __FUNC__
#undef __FUNC__
#endif

#define __FUNC__ "main()"
#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

int main() {
  FENTER;
  typedef QuotientGraph::VertexList VertexList;
  
  Graph graph( nvtxs, Aptr, Aind );
  graph.validate();
  if ( !graph.isValid() ) {
    cerr << "cannot form valid graph" << endl;
  }

  QuotientGraph g( &graph );

  VertexList eliminatedVtxs, reachableVtxs, mergedVtxs;

  cout << "Original Graph" << endl;
  g.printShortGraph( cout );

  cout << "********** Eliminate 2 and 4 ***********" << endl;
  g.eliminateSupernode( 2 );
  g.eliminateSupernode( 4 );
  g.update( reachableVtxs, mergedVtxs );
  g.printShortGraph( cout );

  cout << "reachableVtxs: ";
  for( VertexList::const_iterator it = reachableVtxs.begin(), stop=reachableVtxs.end();
       it != stop; ++it ) { 
    cout << *it << ", ";
  } 
  cout << endl;

  cout << "mergedVtxs: ";
  for( VertexList::const_iterator it = mergedVtxs.begin(), stop=mergedVtxs.end();
       it != stop; ++it ) { 
    cout << *it << ", ";
  } 
  cout << endl;


  cout << "********** Eliminate 0 and 3 ***********" << endl;
  g.eliminateSupernode( 0 );
  g.eliminateSupernode( 3 );
  g.update( reachableVtxs, mergedVtxs );
  g.printShortGraph( cout );

  cout << "reachableVtxs: ";
  for( VertexList::const_iterator it = reachableVtxs.begin(), stop=reachableVtxs.end();
       it != stop; ++it ) { 
    cout << *it << ", ";
  } 
  cout << endl;

  cout << "mergedVtxs: ";
  for( VertexList::const_iterator it = mergedVtxs.begin(), stop=mergedVtxs.end();
       it != stop; ++it ) { 
    cout << *it << ", ";
  } 
  cout << endl;

  cout << "********** Eliminate 1 ***********" << endl;
  g.eliminateSupernode( 1 );
  g.update( reachableVtxs, mergedVtxs );
  g.printShortGraph( cout );

  FRETURN(0);
}
#undef __FUNC__
