//
// GenericForest.h
//
//  $Id: GenericForest.h,v 1.2 2000/02/18 01:31:44 kumfert Exp $
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

#ifndef SPINDLE_ARRAY_FOREST_H
#define SPINDLE_ARRAY_FOREST_H_

#ifndef SPINDLE_H_
#include "spindle/spindle.h"
#endif

#ifndef SPINDLE_AUTO_ARRAY_H_
#include "spindle/SharedArray.h"
#endif

#ifndef SPINDLE_PERSISTANT_H_
#include "spindle/SpindlePersistant.h"
#endif

#ifdef REQUIRE_OLD_CXX_HEADER_SUFFIX
#include "iterator.h"
#else
#include <iterator>
using namespace std;
#endif

SPINDLE_BEGIN_NAMESPACE // 

//
// Iterator Class Declarations
//

class constGenericForestIterator;
class constGenericForestPostorderIterator;
class constGenericForestPreorderIterator;

class constGenericForestIterator;
class constGenericForestPostorderIterator;
class constGenericForestPreorderIterator;

class GenericForest : public SpindlePersistant { 

  SPINDLE_DECLARE_PERSISTANT( GenericForest )

public:
  friend class constGenericForestIterator;
  friend class constGenericForestPostorderIterator;
  friend class constGenericForestPreorderIterator;

  typedef int Key;
  typedef int node;
  typedef constGenericForestIterator const_generic_iterator;
  typedef constGenericForestPostorderIterator const_postorder_iterator;
  typedef constGenericForestPreorderIterator const_preorder_iterator;
 
  // Constructors/Destructors
  GenericForest();                                // creates empty forest
  GenericForest(const int n);                     // creates forest of n trees
  GenericForest(const int n, int* Parent);        // assumes ownership of parent pointer
  GenericForest(const int n, const int* Parent);  // copies parent pointer
  GenericForest(const int n, int* Parent, int *Child, int* Sibling); // assumes ownership of all
  GenericForest(const int n, const int* parentNode, const int* firstChild, const int* nextSibling);
  virtual ~GenericForest();
  
protected:
  SharedArray<int> parent;          // Parent of each node
  void createChildSibling();      // given parent, creates firstChild and nextSibling
  void createDoubleLink();        // given parent, firstChild, and nextSibling, creates nChildren & prevChild

private:
  int nNodes;            // number of nodes in all trees
  int nTrees;            // number of trees 
  int firstRoot;         // root of first tree in the forest
  
  SharedArray<int> child;           // first child of each node
  SharedArray<int> nextSibling;     // next sibling of each node
  SharedArray<int> prevSibling;     // next sibling of each node
  SharedArray<int> nChildren;       // number of children of a node.

 protected:
  // Simple tests
  bool isLeaf( const node cur ) const;
  bool isRoot( const node cur ) const;
  bool isLastSibling( const node cur ) const;
  bool isFirstSibling ( const node cur ) const;
  bool operator==( const GenericForest& f) const;

 public:
  bool setParent( const int length, const int* Parent );
  bool setParent( const int length, int * Parent );


  virtual void validate();
  virtual bool isValid() const;
  virtual void dump( FILE * stream ) const;

  bool reset();
  void resize( const int n );

  int size() const;
  int queryNTrees() const;
  int queryParent( const node Child ) const;
  bool pruneNewTree( const node newTreeRoot );
  bool graftTreeToOther( const node oldRoot, const node newParent );
  const SharedArray<int>& getParent() const { return parent; }

  // Iterator Access
  const_postorder_iterator begin_postorder() const ;
  const_postorder_iterator end_postorder() const ;
  const_preorder_iterator begin_preorder() const ;
  const_preorder_iterator end_preorder() const ;
  const_generic_iterator create_generic_iterator() const ;

};

class constGenericForestIterator { 
  friend class GenericForest;
private:
  const GenericForest& myForest;
  GenericForest::node curNode;
  GenericForest::node curTree;
  int curDepth;
protected:
  // Constructor/Destructor.  
  // NOTE:  protected constructor restricts construction to derived classes and friends
  constGenericForestIterator( const GenericForest& forest);
  
public:
  bool nextTree(); // point to next tree in forest.  Returns false if no more trees exist.
  bool prevTree(); // point to prev tree in forest.  Returns false if no more trees exist.
  bool parent(); // point to the parent of the current node.  Returns false if current is a root.
  bool nextSibling(); // point to next sibling, return false if current is last sibling or 
                      // if current is root.  Use nextTree() to advance to another tree in the forest.
  bool prevSibling(); // point to next sibling, return false if current is last sibling or 
                      // if current is root.  Use prevTree() to advance to another tree in the forest.
  bool firstChild(); // point to the first child of current, false if current is a leaf.
  int queryNSiblings() const ; // return total of siblings, if root, return number of trees in forest.
  int queryNChildren() const ; // return the total number of children.
  int queryDepth() const;
  
  GenericForest::node operator*() const; // dereference pointer
};

class constGenericForestPostorderIterator : public input_iterator< int, ptrdiff_t > { 
  friend class GenericForest;
private:
  const GenericForest& myForest;
  GenericForest::node curNode;
  int curDepth;
protected:
  // Constructor/Destructor.  
  // NOTE:  protected constructor restricts construction to derived classes and friends
  constGenericForestPostorderIterator(const GenericForest& forest);
  constGenericForestPostorderIterator(const GenericForest& forest, int);
  
public:
  bool operator==(const constGenericForestPostorderIterator& p) const;
  GenericForest::node operator*() const;
  
  constGenericForestPostorderIterator& operator++();
  constGenericForestPostorderIterator operator++(int);
  
  bool isRoot() const;
  bool isLeaf() const;
  int  queryDepth() const;
  
}; // end class constGenericForestPostorderIterator

class constGenericForestPreorderIterator : public input_iterator< int, ptrdiff_t > { 
  friend class GenericForest;
private:
  const GenericForest& myForest;
  GenericForest::node curNode;
  int curDepth;
protected:
  // Constructor/Destructor.  
  // NOTE:  protected constructor restricts construction to derived classes and friends
  constGenericForestPreorderIterator(const GenericForest& forest);
  constGenericForestPreorderIterator(const GenericForest& forest, int);
  
public:
  bool operator==(const constGenericForestPreorderIterator& p) const;
  GenericForest::node operator*() const;
  
  constGenericForestPreorderIterator&  operator++();
  constGenericForestPreorderIterator operator++(int);
  
  bool isRoot() const;
  bool isLeaf() const;
  int  queryDepth() const;
  
}; // end class constGenericForestPreorderIterator

////////////////////////////////////////////////////////////
//
// GenericForest inlined functions
//

inline bool
GenericForest::isValid() const { 
  return (currentState == VALID ); 
}

inline bool 
GenericForest::isLeaf( const node cur ) const {
  return ( child[ cur ] == -1 ) ; 
}

inline bool 
GenericForest::isRoot( const node cur ) const {
  return ( parent[ cur ] == -1 ) ; 
}

inline bool 
GenericForest::isLastSibling( const node cur ) const {
  return  ( nextSibling[ cur ] == -1 ) ;  
}


inline bool 
GenericForest::isFirstSibling ( const node cur ) const {
  return ( prevSibling[ cur ] == -1 ) ; 
}

inline bool 
GenericForest::operator==( const GenericForest& f) const {
 return ( (void *) this == (void *) &f ); 
}

inline int 
GenericForest::size() const { 
  return nNodes; 
}

inline int 
GenericForest::queryNTrees() const { 
  return nTrees; 
}

inline int 
GenericForest::queryParent( const node Child ) const { 
  if ((Child<0) || (Child>=nNodes)) { return -1; }
  return  (parent[Child]==-1) ? Child : parent[Child];
}


inline GenericForest::const_postorder_iterator 
GenericForest::begin_postorder() const { 
  return constGenericForestPostorderIterator( *this ); 
}

inline  GenericForest::const_postorder_iterator 
GenericForest::end_postorder() const { 
  return constGenericForestPostorderIterator(*this, 0); 
}

inline  GenericForest::const_preorder_iterator  
GenericForest::begin_preorder() const { 
  return constGenericForestPreorderIterator( *this ); 
}

inline GenericForest::const_preorder_iterator  
GenericForest::end_preorder() const { 
  return constGenericForestPreorderIterator(*this, 0); 
}

inline GenericForest::const_generic_iterator   
GenericForest::create_generic_iterator() const { 
  return constGenericForestIterator( *this ); 
}

//////////////////////////////////////////////////////////////////////////////////////
//
// constGenericForestIterator methods.
//
inline
constGenericForestIterator::constGenericForestIterator( const GenericForest& forest) : myForest(forest) { 
  curNode = curTree = myForest.firstRoot; 
  curDepth = 0;
}

inline bool 
constGenericForestIterator::nextTree() {  
  // point to next tree in forest.  Returns false if no more trees exist.
  if ( myForest.isLastSibling(curTree) ) { return false; }
  curNode = curTree = myForest.nextSibling[curTree];
  curDepth = 0;
  return true;
}

inline bool 
constGenericForestIterator::prevTree() {  
  // point to prev tree in forest.  Returns false if no more trees exist.
  if ( myForest.isFirstSibling(curTree) ) { return false; }
  curNode = curTree = myForest.prevSibling[curTree];
  curDepth = 0;
  return true;
}

inline bool 
constGenericForestIterator::parent() {  
  // point to the parent of the current node.  Returns false if current is a root.
  if ( myForest.isRoot(curNode) ) { return false; }
  curNode = myForest.parent[curNode];
  curDepth--;
  return true;
}

inline bool 
constGenericForestIterator::nextSibling() { 
  // point to next sibling, return false if current is last sibling or 
  // if current is root.  Use nextTree() to advance to another tree in the forest.
  if ( myForest.isRoot(curNode) ) { return false; }
  if ( myForest.isLastSibling(curNode) ) { return false; }
  curNode = myForest.nextSibling[curNode];
  return true;
}

inline bool 
constGenericForestIterator::prevSibling() { 
  // point to next sibling, return false if current is last sibling or 
  // if current is root.  Use nextTree() to advance to another tree in the forest.
  if ( myForest.isRoot(curNode) ) { return false; }
  if ( myForest.isFirstSibling(curNode) ) { return false; }
  curNode = myForest.prevSibling[curNode];
  return true;
}

inline bool 
constGenericForestIterator::firstChild() { // point to the first child of current, false if current is a leaf.
  if ( myForest.isLeaf(curNode) ) { return false; }
  curNode = myForest.child[curNode];
  curDepth++;
  return true;
}

inline int 
constGenericForestIterator::queryNSiblings() const { 
  // return total of siblings, if root, return number of trees in forest.
  return ( myForest.isRoot( curNode ) ) ? myForest.nTrees : myForest.nChildren[ myForest.parent [ curNode ] ] ; 
}
  
inline int 
constGenericForestIterator::queryNChildren() const { 
  // return the total number of children.
  return myForest.nChildren[ curNode ] ;
}

inline int 
constGenericForestIterator::queryDepth() const { 
  return curDepth; 
}

inline GenericForest::node
constGenericForestIterator::operator*() const { 
  return curNode; 
}

////////////////////////////////////////////////////////////////////////////
// 
// constGenericForestPostorderIterator methods
//
inline 
constGenericForestPostorderIterator::constGenericForestPostorderIterator(const GenericForest& forest, int) 
  : myForest(forest) { 
    curNode = -1; 
}
 
inline bool
constGenericForestPostorderIterator::operator==(const constGenericForestPostorderIterator& p) const { 
  return ( ( myForest == p.myForest) && (curNode == p.curNode) );
}
 
inline GenericForest::node 
constGenericForestPostorderIterator::operator*() const { 
  return curNode; 
}

inline constGenericForestPostorderIterator
constGenericForestPostorderIterator::operator++(int) {
  constGenericForestPostorderIterator tmp = *this;
  ++(*this);
  return tmp;
}

inline bool 
constGenericForestPostorderIterator::isRoot() const {
  return myForest.isRoot(curNode); 
}

inline bool 
constGenericForestPostorderIterator::isLeaf() const { 
  return myForest.isLeaf(curNode); 
}

inline int  
constGenericForestPostorderIterator::queryDepth() const { 
  return curDepth; 
}

////////////////////////////////////////////////////////////////////////////////////////
//
// constGenericForestPreorderIterator
// 
inline 
constGenericForestPreorderIterator::constGenericForestPreorderIterator(const GenericForest& forest) 
  : myForest(forest) { 
    curNode = myForest.firstRoot; 
    curDepth = 0;
}

inline 
constGenericForestPreorderIterator::constGenericForestPreorderIterator(const GenericForest& forest, int) 
  : myForest(forest) { 
    curNode = -1; 
}
  
inline bool 
constGenericForestPreorderIterator::operator==(const constGenericForestPreorderIterator& p) const { 
  return ( (myForest == p.myForest) && (curNode == p.curNode) );
}
  
inline GenericForest::Key 
constGenericForestPreorderIterator::operator*() const { 
  return curNode; 
}

inline constGenericForestPreorderIterator
constGenericForestPreorderIterator::operator++(int) {
  constGenericForestPreorderIterator tmp = *this;
  ++(*this);
  return tmp;
}
  
inline	bool 
constGenericForestPreorderIterator::isRoot() const { 
  return myForest.isRoot(curNode); 
}

inline	bool 
constGenericForestPreorderIterator::isLeaf() const { 
  return myForest.isLeaf(curNode); 
}

inline	int  
constGenericForestPreorderIterator::queryDepth() const { 
  return curDepth; 
}

SPINDLE_END_NAMESPACE


#endif
