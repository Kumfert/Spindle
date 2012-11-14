//
// SymbolicFactorization.h
//
// $Id: SymbolicFactorization.h,v 1.2 2000/02/18 01:32:04 kumfert Exp $
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

#ifndef SPINDLE_SYMBOLIC_FACTORIZATION_H_
#define SPINDLE_SYMBOLIC_FACTORIZATION_H_

#ifndef SPINDLE_H_
#include "spindle/spindle.h"
#endif

#ifndef SPINDLE_AUTO_ARRAY_H_
#include "spindle/SharedArray.h"
#endif

#ifndef SPINDLE_AUTO_PTR_H_
#include "spindle/SharedPtr.h"
#endif

#ifndef SPINDLE_ALGORITHM_H_
#include "spindle/SpindleAlgorithm.h"
#endif 

#ifndef SPINDLE_PERMUTATION_H_
#include "spindle/PermutationMap.h"
#endif

SPINDLE_BEGIN_NAMESPACE

class Graph;
class EliminationForest;

class SymbolicFactorization : public SpindleAlgorithm {

  SPINDLE_DECLARE_DYNAMIC( SymbolicFactorization )

public:
  typedef EliminationForest ETree;
  
private:
  const Graph *graph;
  SharedPtr<ETree> etree;
  SharedPtr<PermutationMap> perm;
  SharedArray<int> rowCount;
  SharedArray<int> colCount;

  bool run();
public:
  SymbolicFactorization();
  virtual ~SymbolicFactorization();

  virtual bool reset();
  bool setGraph( const Graph* g );  // sets to ready

  bool setETree( const ETree* et );  // must be ready
  bool setPermutationMap(  const PermutationMap* p ); // must be ready

  // to compute the symbolic factorization
  virtual bool execute(); // may create new permutation and etree as needed

  // to get output

  // NOTE: Even when factoring an unordered matrix, a permutation
  //       may be generated.  This permutation is a postordering of 
  //       the elimination tree.  The indices of the elimination tree
  //       are also in terms of this postordering.
  const SharedPtr<PermutationMap>& getPermutationMap() const { return perm; }
  SharedPtr<PermutationMap>& getPermutationMap() { return perm; }

  // NOTE: etree returned may not be identical to the one 
  //       provided before execution.  The factorization will 
  //       convert its copy to indices in the postordered ordering
  //       not neccessarily the original ordering
  const SharedPtr<ETree>& getETree() const { return etree; }
  SharedPtr<ETree>& getETree() { return etree; }

  long queryFill(); // = sum_{i=0}^{n} colCount[i] NNZ(A)
  long querySize(); // = sum_{i=0}^{n} colCount[i]
  double queryWork(); // = sum_{i=0}^{n} colCount[i]^2
  
};

SPINDLE_END_NAMESPACE

#endif // ifndef SPINDLE_SYMBOLIC_FACTORIZATION_H_
