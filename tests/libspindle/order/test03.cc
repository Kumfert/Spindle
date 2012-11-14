//
// src/order/drivers/tests/test03.cc -- sloan on an simple graph
//
// $Id: test03.cc,v 1.2 2000/02/18 01:32:09 kumfert Exp $
//
//  Gary Kumfert, Old Dominion University
//  Copyright(c) 1997-1998, Old Dominion University.  All rights reserved.
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


#include <iostream.h>
#include "spindle/Graph.h"
#include "spindle/SloanEngine.h"
#include "samples/small_mesh.h"

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

int main() {
  Graph graph(nvtxs, (const int*) Aptr, (const int*)Aind);
  graph.validate();
  if ( ! graph.isValid() ) {
    cerr << "Error: Cannot create valid graph." << endl;
  }
  SloanEngine ordering(&graph);

  ordering.setStartVtx( 0 );
  ordering.setEndVtx( 8 );
  ordering.execute();

  const PermutationMap * perm = ordering.getPermutation();

  if ( ! perm->isValid() ) {
    cerr << "Error: could not confirm a valid permutation." << endl;
  }
  const int * new2Old = perm->getNew2Old().lend();

  for (int i=0; i<9; i++) {
    cout << "perm[" << i << "] = " << new2Old[i] << endl;
  }

}
