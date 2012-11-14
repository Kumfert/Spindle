//
// ArrayBucketSorter.h
//
//  $Id: ArrayBucketSorter.h,v 1.2 2000/02/18 01:31:44 kumfert Exp $
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
//=====================================================================
//
// This bucket sorter can sort `nItemsMax' items into `nBucketsMax' buckets
// using a key which is hashed into the buckets (usually modulo nBucketsMax).
// Each Item must be unique, but the key (obviously) may not.
//
// Each bucket contains a circular doubly linked list.  Inserts are made 
// at the front or back of each list.  The bracket operator `[Key]' returns 
// the Item at the top of that bucket, or -1 if the bucket is empty.
//
// Items can be removed from anywhere in the ArrayBucketSorter in O(1).  This was
// a design constraint since this object is used in a Multiple Minimum Degree
// code.  This is also a reason why the Items must be unique.
//
// The user can query what the original key was, since the key is not
// necessarily equal to the bucket number.

#ifndef SPINDLE_ARRAY_BUCKET_SORTER_H_
#define SPINDLE_ARRAY_BUCKET_SORTER_H_

#ifndef SPINDLE_SYSTEM_H_
#include "spindle/SpindleSystem.h"
#endif

#ifndef SPINDLE_PERSISTANT_H_
#include "spindle/SpindlePersistant.h"
#endif

SPINDLE_BEGIN_NAMESPACE 

/**
 * @memo sorts 'nItemsMax' items with 'nBucketsMax' unique keys.
 * @type class
 *
 * @author Gary Kumfert
 */
class ArrayBucketSorter : public SpindlePersistant {
  SPINDLE_DECLARE_PERSISTANT( ArrayBucketSorter )

protected:
  enum{ FLAG = -1 };    // C++ enum hack for a constant flag  
  /// number of unique keys 
  int nBucketsMax;    

  /// number of unique item ID numbers
  int nItemsMax;   
  int isInsertFront;

  /// Simple counter of number Items in ArrayBucketSorter
  int nInserted;         
  
  /// array of heads for each list length==(nBucketsMax)
  int *bucket;           

  /// array of forward pointers length==(nItemsMax)
  int *next;             
  
  /// array of back pointers length==(nItemsMax)
  int *prev;            

  /// array mapping each vertex in structure to its bucket
  int *key;              
  //    length==(nItemsMax)
public:
  
  /**
   * default constructor
   */
  ArrayBucketSorter();

  /** 
   * constructor
   */
  ArrayBucketSorter(const int nBuckets, const int nItems);

  /**
   * destructor
   */
  virtual ~ArrayBucketSorter();

  /**
   *
   */
  virtual void validate();

  /**
   *
   */
  virtual bool isValid() const;

  /**
   *  pretty prints the bucket structure
   */
  virtual void dump( FILE * stream ) const;

  /**
   *  pretty prints the bucket structure
   */
  void dump() const; 

  /**
   * @return true if there are no more Items to be removed
   */
  bool isEmpty() const;    

  /**
   * get the number inserted
   */
  int size() const;        

  /**
   * @return the minimum key of a non-empty bucket, -1 if none.
   */
  int queryMinBucket() const;  

  /**
   * @return minimum key of next non-empty bucket, -1 if none.
   */
  int queryNextBucket(const int Bucket) const;
  
  /**
   * @return the first item in the requested bucket, 
   *         -1 if out of range or bucket is empty
   */ 
  int queryFirstItem(const int Bucket) const;

  /**
   * @return the next item in the same bucket
   *         -1 if out of range or Item is last in bucket
   */
  int queryNextItem(const int Item) const; 

  /**
   * @return prev item in same bucket 
   *         -1 if out of range or Item is first in bucket
   */
  int queryPrevItem(const int Item) const;
  
  /**
   * @return the key of an item
   */
  int queryKey( const int Item );   

  /**
   * @return the bucket an item is in
   */
  int queryBucket( const int Item ); 

  /**
   * @return the number of items inserted into the bucketsorter
   */
  int queryNInserted() const;

  /**
   * @return true if insert was successful, 
   *         false if item was already in some bucket
   */
  bool insert(const int Key, const int Item);

  /** 
   * make all inserts occur to the front of the bucket
   */
  void setInsertFront();
  
  /**
   * make all inserts occur in the back of a bucket
   */
  void setInsertBack();
    
  /**
   * @return true if item is found and removed, 
   *         false if item is not found
   */
  bool remove(const int Item);
  
  /**
   * resets the ArrayBucketSorter to its initial (empty) state
   */
  bool reset();
  
  /**
   * print out complete internal state
   */
  void printDebug(FILE * fp) const;

  /**
   * print out complete internal state
   */
  void printDebug() const;
  
protected:
  /**
   * insert 'Item' with 'Key' into the front of bucket 'bucketID'
   * all arguments are assumed in range
   * @return true if insert succeeds, false otherwise
   */
  bool insertFront(const int Key, const int Item, const int bucketID);

  /**
   * insert 'Item' with 'Key' into the back of bucket 'bucketID'
   * all arguments are assumed in range
   * @return true if insert succeeds, false otherwise
   */
  bool insertBack(const int Key, const int Item, const int bucketID);

  /**
   * remove an 'Item' (assumed to be in range)
   * @return true if insert succeeds, false otherwise
   */
  bool removeItem(const int Item);

  /**
   * @return next item if it exists, -1 otherwise
   */
  int nextItem(const int Item) const;

  /**
   * @return next item if it exists, -1 otherwise
   */
  int prevItem(const int Item) const;
};

inline ArrayBucketSorter::ArrayBucketSorter() 
: nBucketsMax(0), nItemsMax(0){
  currentState = EMPTY;
}
  

inline ArrayBucketSorter::ArrayBucketSorter(const int nBucketsMax_, const int nItems) 
  : nBucketsMax(nBucketsMax_), nItemsMax(nItems) {
    incrementInstanceCount( ArrayBucketSorter::MetaData );
    bucket = new int[nBucketsMax];  
    next   = new int[nItemsMax]; 
    prev   = new int[nItemsMax]; 
    key    = new int[nItemsMax]; 
    reset();
}

inline bool
ArrayBucketSorter::isValid() const {
  return (currentState == VALID);
}

inline void 
ArrayBucketSorter::setInsertFront() {
  isInsertFront = true; 
}

inline void 
ArrayBucketSorter::setInsertBack() { 
  isInsertFront = false; 
}

inline bool 
ArrayBucketSorter::insert(const int Key, const int Item) { 
  if ((Item < 0) || (Item >= nItemsMax)) { return false; } // out of range
  if ( key[Item] != FLAG ) { return false; } // already inserted
  int bucketID = ( Key < nBucketsMax ) ? Key : Key % nBucketsMax ;
  bool insertSucceeded = (isInsertFront) ? insertFront(Key,Item,bucketID) : insertBack(Key,Item,bucketID);
  if (insertSucceeded) { ++nInserted; }
  return insertSucceeded;
}
  
// returns true if item is found and removed, false if item is not found
inline bool 
ArrayBucketSorter::remove(const int Item){
  bool removeSucceeded = ((Item < 0) || (Item >= nItemsMax)) ? false : removeItem(Item);
  if (removeSucceeded) { --nInserted; }
  return removeSucceeded;
}
  
inline bool 
ArrayBucketSorter::isEmpty() const { 
  return (nInserted == 0); 
}

inline int 
ArrayBucketSorter::queryKey( const int Item ) {
  if ( (Item<0) || (Item>nItemsMax) ) { 
    return FLAG; 
  } else {
    return key[Item];
  }
}


inline int 
ArrayBucketSorter::queryBucket( const int Item ) {
  if ( (Item<0) || (Item>nItemsMax) ) { 
    return FLAG; 
  } else {
    return key[Item] % nBucketsMax ;
  }
}

inline int 
ArrayBucketSorter::queryNInserted() const { 
  return nInserted; 
}

inline int 
ArrayBucketSorter::queryMinBucket() const { 
  if (isEmpty()) return FLAG;
  int i=0;
  while ((i<nBucketsMax) && (bucket[i] == FLAG)) ++i;
  return ( i == nBucketsMax ) ? FLAG : i;
}
  
// returns the minimum non-empty bucket, -1 if empty
#define __FUNC__ "Array ArrayBucketSorter::queryNextBucket(const int Bucket) const"
inline int 
ArrayBucketSorter::queryNextBucket(const int Bucket) const {       
  FENTER;
  if (isEmpty()) FRETURN( FLAG );
  ASSERT( Bucket>=0 , "Requested Bucket %d is invalid, must be positive", Bucket );
  ASSERT( Bucket<nBucketsMax, "Requested Bucket %d is invalid, number of Buckets=%d", Bucket,nBucketsMax);
  int i=Bucket;
  while ((i<nBucketsMax) && (bucket[i] == FLAG)) ++i;
  FRETURN( ( i == nBucketsMax ) ? FLAG : i );
}
#undef __FUNC__
  
inline int 
ArrayBucketSorter::queryFirstItem(const int Key) const { 
  return ( (Key >= 0) && ( Key < nBucketsMax) ) ? bucket[Key] : FLAG ; 
}

inline int 
ArrayBucketSorter::queryNextItem(const int Item) const {
  return ((Item<0) || (Item>=nItemsMax)) ? FLAG : nextItem(Item) ;
}  
  
inline int 
ArrayBucketSorter::queryPrevItem(const int Item) const {
  return ((Item<0) || (Item>=nItemsMax)) ? FLAG : prevItem(Item) ;
}  
  
inline void 
ArrayBucketSorter::dump() const{
  dump( stdout ); 
}
  
inline int  
ArrayBucketSorter::size() const {
  return nInserted; 
}


inline void  
ArrayBucketSorter::printDebug() const { 
  printDebug( stdout ); 
}

SPINDLE_END_NAMESPACE

#endif
