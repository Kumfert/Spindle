//
// testSymbolicFactorization.cc
//
//
//

#include <iostream.h>
#include "spindle/Graph.h"
#include "spindle/EliminationForest.h"
#include "spindle/SymbolicFactorization.h"

#include "samples/small_mesh.h"

static const int mmd_new2old_perm[]   = { 0, 2, 6, 8, 1, 7, 3, 4, 5 };
static const int mmd_parent_pointer[] = { 1, 3, 1, 4, 5, 5, 7, 3, 7 }; 
#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

int main() {
  ios::sync_with_stdio();
  typedef SymbolicFactorization::ETree ETree;

  Graph graph(nvtxs,Aptr,Aind);
  graph.validate();

  if ( ! graph.isValid() ) {
    cerr << "Cannot create valid graph." << endl;
  }

  // first try for an unordered graph
  {
    SymbolicFactorization symbfact;
    
    if ( ! symbfact.setGraph( &graph ) ) {
      cerr << "Error setting Graph" << endl;
      cerr << symbfact.queryErrMsg() << endl;
      exit(-1);
    }
    if ( ! symbfact.execute() ) {
      cerr << "Error doing symbolic factorization on unordered graph." << endl;
      cerr << symbfact.queryErrMsg() << endl;
      exit(-1);
    }
    
    cout << "For unordered graph: " << endl;
    cout << "     Size of the factor     = " << symbfact.querySize() << endl;
    cout << "     Size of fill           = " << symbfact.queryFill() << endl;
    cout << "     Amount of work (flops) = " << symbfact.queryWork() << endl;
    cout << endl;
  }

  // now try again with the graph permuted
  {
    SymbolicFactorization symbfact;
    
    if ( ! symbfact.setGraph( &graph ) ) {
      cerr << "Error setting Graph" << endl;
      cerr << symbfact.queryErrMsg() << endl;
      exit(-1);
    }
    PermutationMap* p = new PermutationMap(9);
    p->getNew2Old().import( mmd_new2old_perm , 9 );
    p->validate();
    if (! p->isValid() ) {
      cerr << "PermutationMap Object creation failed." << endl;
      exit(-1);
    }
    if ( ! symbfact.setPermutationMap( p ) ) {
      cerr << "Error seting permutation in symbolic factorization" << endl;
      cerr << symbfact.queryErrMsg() << endl;
      exit(-1);
    }
    if ( ! symbfact.execute() ) {
      cerr << "Error doing symbolic factorization on unordered graph." << endl;
      cerr << symbfact.queryErrMsg() << endl;
      exit(-1);
    }
    
    cout << "For permuted graph: " << endl;
    cout << "     Size of the factor     = " << symbfact.querySize() << endl;
    cout << "     Size of fill           = " << symbfact.queryFill() << endl;
    cout << "     Amount of work (flops) = " << symbfact.queryWork() << endl;
    cout << endl;
    delete p;
  }

  // finally with a predefined elimination tree
  {
    SymbolicFactorization symbfact;
    
    if ( ! symbfact.setGraph( &graph ) ) {
      cerr << "Error setting Graph" << endl;
      cerr << symbfact.queryErrMsg() << endl;
      exit(-1);
    }
    ETree* et = new ETree( &graph, graph.size(), mmd_parent_pointer );
    et->validate();
    if (! et->isValid() ) {
      cerr << "Elimination Forest Object creation failed." << endl;
      exit(-1);
    }
    if ( ! symbfact.setETree( et ) ) {
      cerr << "Error seting elimination tree in symbolic factorization" << endl;
      cerr << symbfact.queryErrMsg() << endl;
      exit(-1);
    }
    if ( ! symbfact.execute() ) {
      cerr << "Error doing symbolic factorization on unordered graph." << endl;
      cerr << symbfact.queryErrMsg() << endl;
      exit(-1);
    }
    
    cout << "For graph with etree: " << endl;
    cout << "     Size of the factor     = " << symbfact.querySize() << endl;
    cout << "     Size of fill           = " << symbfact.queryFill() << endl;
    cout << "     Amount of work (flops) = " << symbfact.queryWork() << endl;
    cout << endl;
    delete et;
  }
}
