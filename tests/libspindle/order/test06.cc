//
// src/order/drivers/tests/test06.cc -- test MinPriorityEngine
//
// $Id: test06.cc,v 1.2 2000/02/18 01:32:09 kumfert Exp $
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


#include <iostream.h>

#define DEBUG_TRACE true
#define DEBUG_OUT cout

#include "spindle/ArrayBucketSorter.h"
#include "spindle/MinPriorityEngine.h"
#include "samples/small_mesh.h"

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

#define __FUNC__ "int main"
int main() {
  FENTER;

  //  SpindleSystem::setTraceLevel( SPINDLE_TRACE_FLOW | SPINDLE_TRACE_DEBUG );
  SpindleSystem::setTraceFile( stdout );

  FCALL;
  Graph graph(nvtxs, (const int*) Aptr, (const int*)Aind);
  graph.validate();
  if ( !graph.isValid() ) {
    cerr << "Error: could not finalize graph " << endl;
    exit(-1);
  }

  FCALL;
  MinPriorityEngine ordering(&graph);
  if (DEBUG_TRACE) { cout << "Created Ordering" << endl; }
  
  //   ordering.setDegreeScheme( ordering.EXTERNAL_DEGREE );

  FCALL ordering.execute();
  if (DEBUG_TRACE) { cout << "Executing Ordering" << endl; }

  PermutationMap perm(9);
  perm.getOld2New().import( ordering.lendQuotientGraph().getOld2New(), 9);
  perm.validate();
  if ( perm.isValid() ) {
    const int * new2Old = perm.getNew2Old().lend();
    
    for (int i=0; i<9; i++) {
      cout << "perm[" << i << "] = " << new2Old[i] << endl;
    }
  } else {
    cout << "ERROR: PermutationMap is not valid" << endl;
  }
  FRETURN(0);
}
