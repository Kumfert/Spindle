//
// testGenericForest_1.cc
//
//  $Id: test01.cc,v 1.2 2000/02/18 01:32:05 kumfert Exp $
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
#include "spindle/GenericForest.h"

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
    
int main() {
  ios::sync_with_stdio();

  cout << "Simple forest of 10 trees " << endl;
  Forest f(10);
  f.validate();
  if ( !f.isValid() ) {
    cerr << "Error: failed to create valid forest" << endl;
    exit(-1);
  }
  preprint( f ); 

  cout << "  Make 2 a subtree of 1... " << TEST( f.graftTreeToOther( 2, 1) ) << endl;
  cout << "  Make 4 a subtree of 2... " << TEST( f.graftTreeToOther( 4, 2) ) << endl;
  cout << "  Make 9 a subtree of 7... " << TEST( f.graftTreeToOther( 9, 7) ) << endl;

  preprint(f);

  cout << "  Try to make 9 a subtree of 3... " << TEST( f.graftTreeToOther( 9, 7) )
       << "  (should fail because 9 isn't a root)" << endl;
  cout << "  Try to make 1 a subree of 1 ... " << TEST( f.graftTreeToOther( 1, 1) )
       << "  (should fail because node can't be its own parent)" << endl;

  preprint(f);

  cout << "  Make 6 a subtree of 2... " << TEST( f.graftTreeToOther( 6, 2) ) << endl;
  cout << "  Make 8 a subtree of 2... " << TEST( f.graftTreeToOther( 8, 2) ) << endl;
  cout << "  Make 0 a subtree of 2... " << TEST( f.graftTreeToOther( 0, 2) ) << endl;

  preprint(f);
  postprint(f);

  cout << "  Make 5 a subtree of 7... " << TEST( f.graftTreeToOther( 5, 7) ) << endl;
  cout << "  Make 3 a subtree of 2... " << TEST( f.graftTreeToOther( 3, 2) ) << endl;
  cout << "  Make 7 a subtree of 2... " << TEST( f.graftTreeToOther( 7, 2) ) << endl;

  preprint(f);
  postprint(f);

  cout << "  Try to prune new tree at 1... " << TEST( f.pruneNewTree(1) )
       << "  (should fail because 1 is already a root)" << endl;

  preprint(f);

  cout << "  Prune new tree at 7... " << TEST( f.pruneNewTree(7) ) << endl;
  cout << "  Prune new tree at 5... " << TEST( f.pruneNewTree(5) ) << endl;
  cout << "  Prune new tree at 2... " << TEST( f.pruneNewTree(2) ) << endl;

  preprint(f);
  
}
