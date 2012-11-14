//
// BinaryHeap.h -- a specialized binary heap implementation.
//
//  $Id: BinaryHeap.cc,v 1.2 2000/02/18 01:31:47 kumfert Exp $
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

#include "spindle/BinaryHeap.h"

/*
 * Warning!!! This heap uses an array from 1..n   _NOT_  0..(n-1)
 *            so that the following macros are always valid.
 *
 */

#define PARENT(A)  ((A)>>1)
#define LEFT(A)    ((A)<<1)
#define RIGHT(A)   (((A)<<1) | 0x0001)

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

BinaryHeap::BinaryHeap(const int maxSize_, const int maxItem_) 
  : maxSize(maxSize_), maxItem(maxItem_), node( maxSize_ + 1 ), map( maxItem ), nItems(0) {
}

BinaryHeap::BinaryHeap(const int maxSize_, const int maxItem_, const int values[], const int nValues)
: maxSize(maxSize_), maxItem(maxItem_), node( maxSize_ + 1 ), map( maxItem ), nItems(nValues) {
  { for (int i=0;i<nItems;i++) {
    node[i+1].first = values[i];
    node[i+1].second = i;
    map[i] = i+1;
  }}
  { for (int i=nItems/2; i>0; i--) {
    heapify(i);
  }}
}

bool
BinaryHeap::resize(const int maxSize_, const int maxItem_) {
  maxSize = maxSize_;
  maxItem = maxItem_;
  node.resize( maxSize_ + 1 );
  map.resize( maxItem_ );
  nItems = 0;
  return true;
}

int 
BinaryHeap::heapify(const int subtree) {
  int i = subtree;
  int L = LEFT(i);
  int R = RIGHT(i);

  int largest = ( (L<=nItems) && (node[L].first>node[i].first) ) ? L : i;
  largest = ( (R<=nItems) && (node[R].first>node[largest].first) ) ? R : largest;

  while ( largest != i ) { // used to be "if", but removed recursion
    // swap i and largest 
    swap( node[i], node[largest] );
    // update map 
    map[node[i].second] = i;
    map[node[largest].second] = largest;
    //     heapify(largest); // the old recursive call
    i = largest;
    L = LEFT(i);
    R = RIGHT(i);
    largest = ( (L<=nItems) && (node[L].first>node[i].first) ) ? L : i;
    largest = ( (R<=nItems) && (node[R].first>node[largest].first) ) ? R : largest;
  }
  return 0;
}


int 
BinaryHeap::extractMax() {
  if ( nItems == 0 ) { return -1; }  // if heap is empty return -1
  int max = node[1].second;        // else max is always the first in the heap

  // Now put a new node at the top of the heap
  node[1] = node[nItems];
  map[node[1].second] = 1;
  map[max] = 0;    
  nItems--;

  // But new node violates heap property, so heapify
  heapify(1);
  return max;
}

int 
BinaryHeap::insert(const int Key, const int Item) {
  if (Item > maxItem) { return -1; }
  if (nItems == maxSize) { return -1; }
  nItems++; // remember node[] goes from 1..nItems
  int i = nItems;
  int parent = PARENT(i);
  // search from the leaf up for the place to insert...
  // ... shifting others down the heap as you go
  while ( (i>1) && (node[parent].first<Key) ) {
    node[i] = node[parent];
    map[node[i].second] = i;
    i = parent;
    parent = PARENT(i);
  }
  // ASSERT:  i is the place in the heap to insert
  node[i].first = Key;
  node[i].second = Item;
  map[Item] = i;
  return 0;
}



int 
BinaryHeap::incrementPriority( const int addToKey, const int Item ) {
  if ( addToKey < 0 ) {
    return decrementPriority( -addToKey, Item );
  }
  int i = map[Item];
  if ( (i<0) || (i>nItems) ) { return -1; }
  int Key = (node[i].first += addToKey) ;
 
  // Now start searching up the tree, shifting others
  // down as you go.
  int parent = PARENT(i);
  while ( (i>1) && (node[parent].first<Key) ) {
    node[i] = node[parent];
    map[node[i].second] = i;
    i = parent;
    parent = PARENT(i);
  }
  // ASSERT:  i is the place in the heap to insert
  node[i].first = Key;
  node[i].second = Item;
  map[Item] = i;
  return 0;
}


int 
BinaryHeap::decrementPriority( const int subtractFromKey, const int Item ) {
  if ( subtractFromKey < 0 ) {
    return incrementPriority( -subtractFromKey, Item );
  }
  int i = map[Item];
  if ( (i<0) || (i>nItems) ) { return -1; }
  node[i].first -= subtractFromKey ; 
 
  // Now start searching down the tree until this item
  // is bigger than both its children
  int L = LEFT(i);
  int R = RIGHT(i);

  int largest = ( (L<=nItems) && (node[L].first>node[i].first) ) ? L : i;
  largest = ( (R<=nItems) && (node[R].first>node[largest].first) ) ? R : largest;

  while ( largest != i ) { 
    // swap i and largest 
    swap( node[i], node[largest] );
    // update map 
    map[node[i].second] = i;
    map[node[largest].second] = largest;
    i = largest;
    L = LEFT(i);
    R = RIGHT(i);
    largest = ( (L<=nItems) && (node[L].first>node[i].first) ) ? L : i;
    largest = ( (R<=nItems) && (node[R].first>node[largest].first) ) ? R : largest;
  }
  return 0;
}

void 
BinaryHeap::print( FILE* stream ) const {
  fprintf( stream, "Heap.size = %d\n", size() );
  int maxDepth = -1;
  printSubTree( stream, 1, 1, maxDepth );
}


void 
BinaryHeap::printSubTree( FILE* stream, const int root, const int depth, int& maxDepth) const {
  int L = LEFT(root);
  int R = RIGHT(root);

  if ((L > nItems) && (maxDepth == -1)) { 
    maxDepth = depth; 
   }
  if (L <= nItems) { printSubTree(stream, L, depth+1, maxDepth); }
  fprintf( stream,"%*s<%d, %d>\n", (maxDepth-depth)*5, "", node[root].first, node[root].second );
  if (R <= nItems) { printSubTree(stream, R, depth+1, maxDepth); }
}


bool
BinaryHeap::peekMax( int &Key, int &Item ) const {
  if ( nItems == 0 ) { return false; }
  Key = node[1].first;
  Item = node[1].second;
  return true;
}
