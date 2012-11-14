//
// EliminationForest.h
//
//  $Id: EliminationForest.h,v 1.2 2000/02/18 01:31:44 kumfert Exp $
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

#ifndef SPINDLE_ARRAY_E_FOREST_H
#define SPINDLE_ARRAY_E_FOREST_H_

#ifndef SPINDLE_AUTO_PTR_H_
#include "spindle/SharedPtr.h"
#endif

#ifndef SPINDLE_ARRAY_FOREST_H_
#include "spindle/GenericForest.h"
#endif 

#ifndef SPINDLE_GRAPH_H_
#include "spindle/Graph.h"
#endif

SPINDLE_BEGIN_NAMESPACE

class PermutationMap;

class EliminationForest : protected GenericForest { 

  SPINDLE_DECLARE_PERSISTANT( EliminationForest )

private:
  friend class constGenericForestIterator;
  friend class constGenericForestPostorderIterator;
  friend class constGenericForestPreorderIterator;

public:
  // Constructors/Destructors
  EliminationForest();
  EliminationForest( const Graph* graph );
  EliminationForest( const Graph* graph, const SharedPtr<PermutationMap>& perm );
  EliminationForest( const Graph* graph, const int n, const int* Parent );
  virtual ~EliminationForest();

private:
  const Graph* g;

public:
  typedef int Key;
  typedef int node;
  typedef constGenericForestIterator const_generic_iterator;
  typedef constGenericForestPostorderIterator const_postorder_iterator;
  typedef constGenericForestPreorderIterator const_preorder_iterator;

  virtual void validate() { GenericForest::validate(); }
  bool isValid() const { return GenericForest::isValid(); }
  void resize(const int n) { g = 0; GenericForest::resize(n); }
  bool reset() { g = 0; return GenericForest::reset(); }
  int size() const { return GenericForest::size(); }
  int queryNTrees() const { return GenericForest::queryNTrees(); }
  int queryParent( const node Child ) const { return GenericForest::queryParent(Child); }
  const SharedArray<int>& getParent() const { return GenericForest::getParent(); }

  // Iterator Access
  const_postorder_iterator begin_postorder() const { return GenericForest::begin_postorder() ; }
  const_postorder_iterator end_postorder() const { return GenericForest::end_postorder(); }
  const_preorder_iterator begin_preorder() const { return GenericForest::begin_preorder(); }
  const_preorder_iterator end_preorder() const { return GenericForest::end_preorder(); }
  const_generic_iterator create_generic_iterator() const { return GenericForest::create_generic_iterator(); }

};

SPINDLE_END_NAMESPACE 

#endif
