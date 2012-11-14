//
// BinaryHeap.h -- a specialized binary heap implementation.
//
//  $Id: BinaryHeap.h,v 1.2 2000/02/18 01:31:48 kumfert Exp $
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
// This implementation is a simple binary heap.  The perhaps
// natural recursion has been replaced with looping to relieve
// all the function call overheads.  The heap also maintains
// an index array so that individual elements can be accessed in
// constant time.
// 

#ifndef SPINDLE_BINARY_HEAP_H_
#define SPINDLE_BINARY_HEAP_H_

#ifndef SPINDLE_H_
#include "spindle/spindle.h"
#endif 

#ifndef SPINDLE_AUTO_ARRAY_H_
#include "spindle/SharedArray.h"
#endif

#ifdef REQUIRE_OLD_CXX_HEADER_SUFFIX
#include <stdio.h>
#include "pair.h"
#include "algobase.h"  // needed for swap
#else
#include <cstdio>
#include <algorithm>
#include <utility>
using namespace std;
#endif

SPINDLE_BEGIN_NAMESPACE

class BinaryHeap {
  typedef int Item;
  typedef int Key;
  typedef pair< Key, Item > NodeType;

  int  maxSize;
  Item maxItem;

  SharedArray<NodeType> node;
  SharedArray<int> map;

  int nItems;

  int heapify(const int subtree);
  void printSubTree( FILE* stream, const int root, const int depth, int& maxDepth) const;
public:
  // constructors
  BinaryHeap(const int maxSize_, const int maxItem_);
  BinaryHeap(const int maxSize_, const int maxItem_, const int values[], const int nvals );
  ~BinaryHeap() {}

  bool resize( const int maxSize_, const int maxItem_ );
  int extractMax();
  int insert( const int Key, const int Item );
  int incrementPriority( const int addToKey, const int Item );
  int decrementPriority( const int subtractFromKey, const int Item );
  void print( FILE* stream=stdout ) const;
  const int size() const { return nItems; }
  bool peekMax( int &Key, int &Item ) const;
};

SPINDLE_END_NAMESPACE

#endif

