//
//  adt/Graph/drivers/tests/test11C.cc -- tests QuotientGraph single elimination
//
//  $Id: test11.cc,v 1.2 2000/02/18 01:32:06 kumfert Exp $
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

#include "samples/small_mesh.h"

#include <iostream.h>
#include <iomanip.h>

#ifdef __FUNC__
#undef __FUNC__
#endif

#define __FUNC__ "int main()"
#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

int main() {
  FENTER;
  Graph graph( nvtxs, Aptr, Aind );
  graph.validate();
  if ( !graph.isValid() ) {
    cerr << "cannot form valid graph" << endl;
  }

  QuotientGraph g( &graph );
  g.enableSingleElimination();

  QuotientGraph::VertexList eliminatedVtxs, reachableVtxs, mergedVtxs;

  cout << "Original Graph" << endl;
  g.printShortGraph( cout );

  cout << "********** Eliminate 0, 2, 6, 8 ***********" << endl;
  g.eliminateSupernode( 0 );  
  g.update( reachableVtxs, mergedVtxs );
  g.printShortGraph( cout );
  g.eliminateSupernode( 2 );
  g.update( reachableVtxs, mergedVtxs );
  g.printShortGraph( cout );
  g.eliminateSupernode( 6 );
  g.update( reachableVtxs, mergedVtxs );
  g.printShortGraph( cout );
  g.eliminateSupernode( 8 );
  g.update( reachableVtxs, mergedVtxs );
  g.printShortGraph( cout );

  cout << "Now eliminate vertices 1, 5" << endl;
  g.eliminateSupernode(1);
  g.update( reachableVtxs, mergedVtxs );
  g.printShortGraph( cout );
  g.eliminateSupernode(7);
  g.update( reachableVtxs, mergedVtxs );
  g.printShortGraph( cout );

  cout << "Eliminating supernode 4" << endl;
  g.eliminateSupernode(4);
  g.printShortGraph( cout ); // can't print degree's without compressing first.
  
  cout << "Exhausted all vertices.  Do compression?" << endl;

  g.update( reachableVtxs, mergedVtxs );
  g.printShortGraph( cout );

  FRETURN(0);
}
#undef __FUNC__
