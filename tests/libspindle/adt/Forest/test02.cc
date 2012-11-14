//
// testGenericForest_1b.cc
//
//  $Id: test02.cc,v 1.2 2000/02/18 01:32:05 kumfert Exp $
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
//  purpose.  It is povided "as is" without express or implied warranty.
//
///////////////////////////////////////////////////////////////////////
//
//
//

#include <iostream.h>
#include "spindle/GenericForest.h"
#include  <iomanip.h>

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

typedef GenericForest Forest;

#define TEST(A)  ( ((A)) ? "Pass" : "Fail" )

void preprint( const Forest& f, ostream& outstr = cout, int space = 4 ) {
  outstr << "------ begin forest (preorder) " << f.queryNTrees() << " tree(s) ----- " << endl;
  for ( Forest::const_preorder_iterator cur = f.begin_preorder(), stop = f.end_preorder(); 
       cur != stop; ++cur ) { 
    outstr << setw( ( cur.queryDepth() + 1) * space ) << "" << *cur << endl;
  }
  outstr << "------  end  forest (preorder) ----- " << endl;
}

void postprint( const Forest& f, ostream& outstr = cout, int space = 4 ) {
  outstr << "------ begin forest (postorder) " << f.queryNTrees() << " tree(s) ----- " << endl;
  for ( Forest::const_postorder_iterator cur = f.begin_postorder(), stop = f.end_postorder(); 
       cur != stop; ++cur ) { 
    outstr << setw( ( cur.queryDepth() + 1 ) * space ) << "" << *cur << endl;
  }
  outstr << "------  end  forest (postorder)----- " << endl;
}
    
/*
                    8
                  /   \
                 0     1 
                 |     | \  \
                 4     7 9 10
             / / | \     | \ 
            5 6 12 13    2  3
            |            |     
            14          11 

*/

const int Parent[] = { 8, 8, 10, 10, 0, 4, 4, 1, 8, 1, 1, 2, 4, 4, 5 };

int main() {
  ios::sync_with_stdio();

  cout << "A tree created straight from a parent-pointer" << endl;

  Forest f(15, Parent);
  f.validate();
  if (! f.isValid() ) {
    cerr << "Error: failed to create a valid forest" << endl;
    exit(-1);
  }
  preprint( f ); 

  cout << "  Prune new tree at 7... " << TEST( f.pruneNewTree(7) ) << endl;
  cout << "  Prune new tree at 5... " << TEST( f.pruneNewTree(5) ) << endl;
  cout << "  Prune new tree at 2... " << TEST( f.pruneNewTree(2) ) << endl;

  preprint(f);

  cout << "  Try to make 9 a subtree of 3... " << TEST( f.graftTreeToOther( 9, 7) )
       << "  (should fail because 9 isn't a root)" << endl;
  cout << "  Try to make 1 a subree of 1 ... " << TEST( f.graftTreeToOther( 1, 1) )
       << "  (should fail because node can't be its own parent)" << endl;

  preprint(f);
  
}
