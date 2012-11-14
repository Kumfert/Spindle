//
// ArrayBucketSorter.cc
//
//  $Id: ArrayBucketSorter.cc,v 1.2 2000/02/18 01:31:43 kumfert Exp $
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

#include "spindle/ArrayBucketSorter.h"

#ifndef SPINDLE_ARCHIVE_H_
#include "spindle/SpindleArchive.h"
#endif

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

ArrayBucketSorter::~ArrayBucketSorter() {
  decrementInstanceCount( ArrayBucketSorter::MetaData );
  delete[] bucket; 
  delete[] next;   
  delete[] prev;
  delete[] key;
}

void 
ArrayBucketSorter::validate() {
  if ( currentState == VALID ) { return;}
  // else do some checking...
}

bool 
ArrayBucketSorter::insertBack(const int Key, const int Item, const int bucketID) {
  int head;
  if ((head = bucket[bucketID]) == FLAG) {    // if bucket is empty
    bucket[bucketID] = Item;                   //    set bucket to object
    prev[Item] = next[Item] = Item;       //    set double linked list
  } else {                                // else (bucket non-empty)
    next[Item] = head;                    //    insert at end of list
    prev[Item] = prev[head];
    next[prev[head]] = Item;
    prev[head] = Item;
  }                                       // endif
  key[Item] = Key;
  return true;                            // insert succeeded
}


bool 
ArrayBucketSorter::insertFront(const int Key, const int Item, const int bucketID) {
  int head;
  if ((head = bucket[bucketID]) == FLAG) {    // if bucket is empty
    bucket[bucketID] = Item;                   //    set bucket to object
    prev[Item] = next[Item] = Item;       //    set double linked list
  } else {                                // else (bucket non-empty)
    next[Item] = head;                    //    insert at end of list
    prev[Item] = prev[head];
    next[prev[head]] = Item;
    prev[head] = Item;
    bucket[bucketID] = Item;
  }                                       // endif
  key[Item] = Key;
  return true;                            // insert succeeded
}

bool
ArrayBucketSorter::removeItem(const int Item) {
  int myKey=key[Item];
  if ( myKey == FLAG ) {               // if object is unowned
    return false;                       //    removal failed
  } else {                              // else 
    int myBucket = myKey % nBucketsMax;
    if (next[Item] == Item) {           //    if sole object in bucket
      bucket[myBucket] = FLAG;         //       empty bucket
    } else {                            //    else
      if (bucket[myBucket] == Item) {   //       if top of list
	bucket[myBucket] = next[Item];  //          bucket starts next
      }                                 //       endif
      next[prev[Item]] = next[Item];    //       remove from rest of list
      prev[next[Item]] = prev[Item];
    }                                   //    endif
    key[Item] = FLAG;                //    reset Item's pointers
    next[Item] = FLAG;
    prev[Item] = FLAG;
    return true;                         //    delete succeeded
  }                                      // endif
}

int 
ArrayBucketSorter::nextItem(const int Item) const {
  int Key = key[Item];
  int bucketID = Key % nBucketsMax;
  if (Key == FLAG) {                    // if item not in buckets
    return FLAG;                        //    no next
  } else { 
    int next_Item=next[Item];   
    if (next_Item == bucket[bucketID]) { // if next item is head
      return FLAG;                      //    no next
    } else {
      return next_Item;                  // return next
    }
  }
}

int 
ArrayBucketSorter::prevItem(const int Item) const {
  int Key = key[Item];
  int bucketID = Key % nBucketsMax;
  if (Key == FLAG) {                    // if item not in buckets
    return FLAG;                        //    no next
  } else { 
    if ( Item == bucket[bucketID]) {    // if item is top of bucket
      return FLAG;                      //    no previous
    } else {
      return prev[Item];
    }
  }
}

bool
ArrayBucketSorter::reset() {
  currentState = VALID;
  nInserted=0;
  setInsertFront();
  int *ip;
  ip = bucket; {for (int i=0; i<nBucketsMax ;  ++i) *ip++ = FLAG;}
  ip = next;   {for (int i=0; i<nItemsMax;  ++i) *ip++ = FLAG;}
  ip = prev;   {for (int i=0; i<nItemsMax;  ++i) *ip++ = FLAG;}
  ip = key;    {for (int i=0; i<nItemsMax;  ++i) *ip++ = FLAG;}
  return true;
}


void 
ArrayBucketSorter::printDebug(FILE * fp ) const {
  fprintf(fp,"\nnBucketsMax = %d    nItemsMax = %d\n", nBucketsMax, nItemsMax );
  fprintf(fp,"    nInserted = %d\n", nInserted );

  fprintf(fp,"bucket: ");
  {for (int i=0; i<nBucketsMax; i++ ) { fprintf(fp,"%3d  ", bucket[i] ); }}
  
  fprintf(fp,"\nnext:   ");
  {for (int i=0; i<nItemsMax; i++ ) { fprintf(fp,"%3d  ", next[i] ); }}

  fprintf(fp,"\nprev:   ");
  {for (int i=0; i<nItemsMax; i++ ) { fprintf(fp,"%3d  ", prev[i] ); }}
  
  fprintf(fp,"\nkey:    ");
  {for (int i=0; i<nItemsMax; i++ ) { fprintf(fp,"%3d  ", key[i] ); }}
  fprintf(fp,"\n");
}

void 
ArrayBucketSorter::dump( FILE * fp ) const {
  fprintf(fp,"\nnInserted = %d\n", nInserted );
  for( int i=0; i < nBucketsMax; i++) {
    fprintf(fp,"%5d : ",i);
    if (bucket[i] != FLAG) {
      int cur = bucket[i];
      do{
	fprintf(fp," %3d ", cur );
	cur = next[cur];		    
      } while ( cur != bucket[i] );
    }
    fprintf(fp,"\n");
  }
}

void
ArrayBucketSorter::storeObject( SpindleArchive& ar ) const {
  ar << nBucketsMax; 
  ar << nItemsMax;
  ar << nInserted;
  ar << isInsertFront;
  // a better way to do this (rather than storing all arrays)
  // would be to simply store (item,key) pairs.
  pair< const int*, size_t > bucket_array( bucket, (size_t)nBucketsMax );
  pair< const int*, size_t > next_array( next, (size_t)nItemsMax );
  pair< const int*, size_t > prev_array( prev, (size_t)nItemsMax );
  pair< const int*, size_t > key_array( key, (size_t)nItemsMax );  
  ar << bucket_array;
  ar << next_array;
  ar << prev_array;
  ar << key_array;
}

void
ArrayBucketSorter::loadObject( SpindleArchive& ar ) {
  ar >> nBucketsMax;
  ar >> nItemsMax;
  ar >> nInserted;
  ar >> isInsertFront;

  bucket = new int[nBucketsMax];
  next   = new int[nItemsMax];
  prev   = new int[nItemsMax];
  key    = new int[nItemsMax];

  // A better way to do this would be to reset the sorter
  // at this point and read in (item,key) pairs, inserting
  // to end
  pair< int*, size_t > bucket_array( bucket, (size_t)nBucketsMax );
  pair< int*, size_t > next_array( next, (size_t)nItemsMax );
  pair< int*, size_t > prev_array( prev, (size_t)nItemsMax );
  pair< int*, size_t > key_array( key, (size_t)nItemsMax );
  
  ar >> bucket_array;
  ar >> next_array;
  ar >> prev_array;
  ar >> key_array;
}

SPINDLE_IMPLEMENT_PERSISTANT( ArrayBucketSorter, SpindlePersistant );

/*
#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

main() {
    ArrayBucketSorter bs(10,10);
    bs.printDebug();
    bs.insert(2,0);
    bs.insert(2,2);
    bs.insert(2,4);
    bs.insert(2,6);
    bs.insert(2,8);
    bs.printDebug();
    bs.print();
}
*/
