//
// testEliminationForest.cc
//
//  $Id: test03.cc,v 1.2 2000/02/18 01:32:05 kumfert Exp $
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


#include <iostream.h>
#include <iomanip.h>
#include "spindle/Graph.h"
#include "spindle/EliminationForest.h"
#include "spindle/PermutationMap.h"
 
#include "samples/small_mesh.h"
 
static int mmd_new2old_perm[] = { 0, 2, 6, 8, 1, 7, 3, 4, 5 };
 
#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

int main() {
  ios::sync_with_stdio();
  Graph graph(nvtxs,Aptr,Aind);
  graph.validate();
  
  if ( ! graph.isValid() ) {
    cerr << "Cannot create valid graph" << endl;
  }

  const int n=graph.size();
  // first work on unpermuted graph...
  { 
    EliminationForest etree( &graph );
    etree.validate();
    if (! etree.isValid() ) {
      cerr << "Error: cannot create a valid etree on unpermuted graph." << endl;
      exit(-1);
    }
    cout << "Preorder and Postorder of unpermuted graph." << endl;
    EliminationForest::const_preorder_iterator pre = etree.begin_preorder();
    EliminationForest::const_postorder_iterator post = etree.begin_postorder();
  
    for(int i=0; i<n; ++i ) {
      cout << setw(5) << i << ") " << setw(5) << *pre++ << " " << setw(5) << *post++ << endl;
    }
    cout << endl;
  }

  // now do the same with a permuted graph
  { 
    PermutationMap* p = new PermutationMap(9);
    p->getNew2Old().take( mmd_new2old_perm , 9 );
    p->validate();
    if (! p->isValid() ) {
      cerr << "PermutationMap Object creation failed." << endl;
      exit(-1);
    }
    SharedPtr<PermutationMap> perm( p );
    EliminationForest etree( &graph , perm );
    etree.validate();
    if (! etree.isValid() ) {
      cerr << "Error: cannot create a valid etree on unpermuted graph." << endl;
      exit(-1);
    }
    EliminationForest::const_preorder_iterator pre = etree.begin_preorder();
    EliminationForest::const_postorder_iterator post = etree.begin_postorder();
    cout << "Preorder and Postorder of permuted graph." << endl; 
    for(int i=0; i<n; ++i ) {
      cout << setw(5) << i << ") " << setw(5) << *pre++ // mmd_new2old_perm[*pre++] 
	   << " " << setw(5) << *post++ << endl;
    }
    cout << endl;
  }
}

