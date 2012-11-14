//
// GenericForest.cc
//
//  $Id: GenericForest.cc,v 1.2 2000/02/18 01:31:44 kumfert Exp $
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

#include "spindle/GenericForest.h"

#ifndef SPINDLE_ARCHIVE_H_
#include "spindle/SpindleArchive.h"
#endif

#ifndef SPINDLE_SYSTEM_H_
#include "spindle/SpindleSystem.h"
#endif

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

#ifdef __FUNC__
#undef __FUNC__
#endif

SPINDLE_IMPLEMENT_PERSISTANT( GenericForest, SpindlePersistant )

GenericForest::GenericForest() {
  // create an empty class
  incrementInstanceCount( GenericForest::MetaData );
  reset();
}

GenericForest::GenericForest(const int n) {
  // creates a Forest of n trees
  incrementInstanceCount( GenericForest::MetaData );
  nNodes = n;
  nTrees = 0;
  firstRoot = -1;
  parent.resize( nNodes );       
  parent.init( -1 );
  child.resize( nNodes );        
  child.init( -1 );
  nextSibling.resize( nNodes );  
  nextSibling.init(  1, 1 ); // counts from 1 to size, step=+1
  prevSibling.resize( nNodes );  
  prevSibling.init( -1, 1 ); // counts from -1 to size-2, step=+1
  nChildren.resize( nNodes );    
  nChildren.init( 0 );
  
  // final touches
  nextSibling[nNodes-1] = -1;
  prevSibling[0] = -1;
  nTrees = nNodes;
  firstRoot = 0;
  currentState = VALID;
}

GenericForest::GenericForest( const int n, int* Parent ) {
  // create a class based on a parent pointer
  incrementInstanceCount( GenericForest::MetaData );
  // non-const so use this parent pointer.
  nNodes = 0;
  nTrees = 0;
  firstRoot = -1;
  resize(n);
  parent.take( Parent, n );
  currentState = UNKNOWN;
  createChildSibling();
  createDoubleLink();
}


GenericForest::GenericForest( const int n, const int* Parent ) { 
  incrementInstanceCount( GenericForest::MetaData );
  // non-const so use this parent pointer.
  nNodes = 0;
  nTrees = 0;
  firstRoot = -1;
  resize(n);
  parent.import( Parent, n );

  currentState = UNKNOWN;
  createChildSibling();
  createDoubleLink();
}

GenericForest::GenericForest(const int n, int* Parent, int *Child, int* Sibling) { 
  incrementInstanceCount( GenericForest::MetaData );
  // non-const structures, so we use the given copy
  nNodes = n;
  nTrees = 0;
  firstRoot = -1;
  parent.take( Parent, n );
  child.take( Child, n );
  nextSibling.take( Sibling, n );

  currentState = UNKNOWN;
  createDoubleLink();
}

GenericForest::GenericForest(const int n, const int* Parent, const int *Child, const int* Sibling) { 
  incrementInstanceCount( GenericForest::MetaData );
  // const structures, so we make a private copy
  nNodes = n;
  nTrees = 0;
  firstRoot = -1;

  // first allocate our own space.
  parent.import(Parent, nNodes);
  child.import(Child, nNodes);
  nextSibling.import(Sibling, nNodes);

  currentState = UNKNOWN;
  createDoubleLink();
}

GenericForest::~GenericForest() {
  decrementInstanceCount( GenericForest::MetaData );
}

bool
GenericForest::reset() {
  nNodes = 0;
  nTrees = 0;
  firstRoot = -1;
  currentState = EMPTY;
  return true;
}

void
GenericForest::resize( const int n ) {
  nNodes = n;
  parent.resize( nNodes );       
  child.resize( nNodes );        
  nextSibling.resize( nNodes );  
  prevSibling.resize( nNodes );  
  nChildren.resize( nNodes );    
  currentState = EMPTY;
}

bool 
GenericForest::setParent( const int length, const int* Parent ) {
  if ( currentState != EMPTY ) { return false; }
  if ( length <= 0 ) { return false; }
  resize(length);
  parent.import( Parent, length );
  createChildSibling();
  createDoubleLink();
  currentState = UNKNOWN;
  return true;
}


bool 
GenericForest::setParent( const int length, int* Parent ) {
  if ( currentState != EMPTY ) { return false; }
  if ( length <= 0 ) { return false; }
  resize(length);
  parent.take( Parent, length );
  createChildSibling();
  createDoubleLink();
  currentState = UNKNOWN;
  return true;
}


void
GenericForest::validate() {
  if ( currentState != UNKNOWN )  { return ; }
  // should do some checking here
  currentState = VALID;
}

void 
GenericForest::createChildSibling() {
  //
  // PRECONDITIONS: assumes parent and nNodes is valid, nothing else
  //                currentState must equal "UNKNOWN";
  // POSTCONDITION: child and nextSibling are initialized.
  //
  if ( currentState != UNKNOWN ) { return ; }

  // 1 resize and initialize child and nextSibling arrays
  child.resize( nNodes );
  child.init( -1 );

  nextSibling.resize( nNodes );
  nextSibling.init( -1 );

  // 2. traverse the parent pointer in reverse
  //    using nextSibling as a stack and child as the pointer into the stack
  for( int i=nNodes-1; i>=0; --i ) {      // for all nodes (in reverse)
    if ( parent[i] == i ) {
      parent[i] = -1;                     // set parent[root] to -1
    } else if ( parent[i] >= 0 ) {        // else if it has a parent
      nextSibling[i] = child[parent[i]];  //     next sibling is the 1st child of parent       
      child[parent[i]] = i;               //     first child of parent becomes this node.
    }
  } 
}

void 
GenericForest::createDoubleLink() { 
  //
  // PRECONDITION:   currentState == UNKNOWN and nNodes, parent, child, and nextSibling are initialized
  //
  // POSTCONDITION:  nChildren and prevSibling is initialized.  List of roots is initialized
  // 
  nTrees = 0;
  firstRoot = -1;
  {for(int i=0; i<nNodes; i++) { 
    if (parent[i] == i) { parent[i] = -1; } // if representing root by self edge
    if (parent[i] == -1) { 
      nTrees++;
      nextSibling[i] = firstRoot;
      firstRoot = i;
    }
  }}
  
  // next initialize prevSibling array
  // start by making it all -1;
  prevSibling.resize( nNodes );
  prevSibling.init( -1 );

  // next, invert nextSibling array
  {for( int i=0; i<nNodes; i++ ) { 
    if ( nextSibling[i] != -1 ) {
      prevSibling[ nextSibling[i] ] = i;
    }
  }}

  // finally initialize nChildren array
  nChildren.resize( nNodes );
  nChildren.init( 0 );
  {for (int i=0; i<nNodes; i++ ) { 
    node parent_i = parent[i];
    if ( parent_i != -1 ) { 
      nChildren[ parent_i ]++;
    }
  }}
}

void
GenericForest::dump( FILE * stream ) const {
  if ( isValid() ) {
    for( int i=0; i<nNodes; ++i ) {
      fprintf(stream, "%d %d\n", i, parent[i] );
    }
  } else { 
    fprintf(stream, "invalid\n" );
  }
  return;
}
  
bool 
GenericForest::pruneNewTree( const node newTreeRoot ) { 
  if ( currentState != VALID ) { return false; }
  if ( (newTreeRoot < 0) || (newTreeRoot >= nNodes) ) { return false; }
  if ( isRoot( newTreeRoot ) ) { return false; }  // no pruning necessary
  node oldparent = parent[ newTreeRoot ];

  // First remove new root from current sibling list
  node old_sibling = child[ oldparent ];
  if ( old_sibling == newTreeRoot ) {                   // if new root is the first child of old parent
    child[ oldparent ] = nextSibling[ newTreeRoot ]; //   set child of old parent to be next sibling
  }
  node next = nextSibling [ newTreeRoot ];
  node prev = prevSibling [ newTreeRoot ];
  if ( next != -1 ) prevSibling[ next ] = prev;
  if ( prev != -1 ) nextSibling[ prev ] = next;
  nChildren[ oldparent ]--; // reduce number of children of old parent

  // now attach newTreeRoot to the list of roots
  nextSibling[ newTreeRoot ] = firstRoot;
  prevSibling[ firstRoot ] = newTreeRoot;
  firstRoot = newTreeRoot;
  parent[ newTreeRoot ] = -1;
  nTrees++;
  return true;
}

bool GenericForest::graftTreeToOther( const node oldRoot, const node newParent ) { 
  if ( currentState != VALID ) { return false; }
  if ( (oldRoot < 0) || (oldRoot >= nNodes) || (newParent < 0) || (newParent >= nNodes) ) { 
    // bad argument: out of range
    return false;
  }
  if ( oldRoot == newParent ) { return false; } // cannot set parent to itself
  if ( ! isRoot(oldRoot) ) { return false; }  // bad argument: not a root
  if ( nTrees == 1 ) { return false; } // cannot graft when there is only one tree
  // first remove oldRoot from its list of roots.
  if ( firstRoot == oldRoot ) { 
    firstRoot = nextSibling[oldRoot];
  }
  node next = nextSibling [ oldRoot ];
  node prev = prevSibling [ oldRoot ];
  if ( next != -1 ) prevSibling[ next ] = prev;
  if ( prev != -1 ) nextSibling[ prev ] = next;
  nTrees--;

  // Now attach oldRoot to child list of newParent
  nextSibling[ oldRoot ] = child[ newParent ];
  prevSibling[ oldRoot ] = -1;
  child[ newParent ] = oldRoot;
  nChildren[ newParent ]++;
  parent[ oldRoot ] = newParent;
  return true;
}

void 
GenericForest::storeObject( SpindleArchive& ar ) const {
  switch(currentState) {
  case INVALID:
    ar << -1;
    break;
  case EMPTY:
    ar << 0 ;
    break;
  case UNKNOWN:
    ar << 1 ;
    break;
  case VALID:
    ar << 2;
    break;
  default:
    ar << -1;
    break;
  }
  if ( currentState==UNKNOWN || currentState==VALID ) {
    const int *p = parent.lend();
    size_t my_size = nNodes;
    pair< const int*, size_t> my_pair( p, my_size );
    ar << my_pair;
  }
}

#define __FUNC__ "GenericForest::loadObject( SpindleArchive& ar )"
void
GenericForest::loadObject( SpindleArchive& ar ) {
  int state;
  ar >> state;
  switch(state) {
  case -1:
    currentState = INVALID;
    break;
  case 0:
    currentState = EMPTY;
    break;
  case 1:
    currentState = UNKNOWN;
    break;
  case 2:
    currentState = VALID;
    break;
  default:
    currentState = INVALID;
    break;
  }
  if ( (currentState==UNKNOWN) || (currentState==VALID) ) {
    size_t new_size = ar.peekLength();
    nNodes = (int) new_size;
    parent.resize(nNodes);
    int* p = parent.give();
    pair<int*, size_t> new_pair( p, new_size );
    
    ar >> new_pair;
    WARN_IF( new_pair.second != new_size, "Warning, expected size=%d, but extracted only %d.", 
	     new_pair.second, new_size );
    parent.take( new_pair.first, new_pair.second );
  }
  createChildSibling();
  createDoubleLink();
  if ( currentState==VALID ) { // if it was saved as valid, we need to revalidate here.
    currentState=UNKNOWN;
    validate();
  }
}
#undef __FUNC__

constGenericForestPostorderIterator::constGenericForestPostorderIterator (const GenericForest& forest) 
  : myForest(forest) { 
    curNode = myForest.firstRoot; 
    curDepth = 0;
    GenericForest::node next;
    while( ( next = myForest.child[ curNode ] ) != -1 ) { curNode = next; curDepth++; }
}
 

constGenericForestPostorderIterator& 
constGenericForestPostorderIterator::operator++() {
  if ( ! myForest.isLastSibling( curNode ) ) {         // if we have a sibling
    curNode = myForest.nextSibling[ curNode ];        //    advance to sibling.
    GenericForest::node next;                            //    then advance to lowest descendant
    while( ( next = myForest.child[ curNode ] ) != -1 ) { curNode = next; curDepth++; }
  } else if ( ! myForest.isRoot(curNode) ) {           // else  if we have a  parent
    curNode = myForest.parent[ curNode ];             //    advance to parent
    curDepth--;                                        // 
  } else {    	                                   // else (we are at last root)
    curNode = -1;                                      //    set iterator to end()
  } 
  return *this;
}

constGenericForestPreorderIterator& 
constGenericForestPreorderIterator::operator++() {
  if ( ! myForest.isLeaf(curNode) ) {                  // if not at a leaf
    curNode = myForest.child[ curNode ];              //    advance to child
    curDepth++;
  } else if ( ! myForest.isLastSibling( curNode ) ) {  // else if not last sibling
    curNode = myForest.nextSibling[ curNode ];        //    advance to sibling.
  } else {                                             // else 
    for ( ; curNode != -1; curNode = myForest.parent[ curNode ]) {
      //    for each ancestor
      if ( ! myForest.isLastSibling( curNode ) ){      //        if ancestor has a sibling
	curNode = myForest.nextSibling[ curNode ];    //           advance to this 
	return *this;                                  //           return
      }
      curDepth--;
    }
    // ASSERT:  if we get here, then curNode == -1 which is the flag for end.
  }
  return *this;
}
