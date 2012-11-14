//
// src/order/drivers/tests/test01.cc - unweighted RCM ordering
//
// $Id: test01.cc,v 1.2 2000/02/18 01:32:09 kumfert Exp $
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


#include <iostream.h>
#include "spindle/Graph.h"
#include "spindle/RCMEngine.h"
#include "spindle/PermutationMap.h"
#include "samples/small_mesh.h"

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

int main() {
  Graph graph(nvtxs, Aptr, Aind);
  graph.validate();
  if ( ! graph.isValid() ) {
    cerr << "ERROR: cannot create valid graph" << endl;
  }
  RCMEngine ordering(&graph);
   
  ordering.execute();

  const PermutationMap *perm = ordering.getPermutation();

  //  perm->validate();
  if ( !perm || !perm->isValid() ) {
    cerr << "Error: cannot validate permutation" << endl;
    exit(-1);
  }

  const int* new2old = perm->getNew2Old().lend();

  for (int i=0; i<9; i++) {
    cout << "new2old[" << i << "] = " << new2old[i] << endl;
  }

}
