//
// CompressionMap.cc
//
//  $Id: CompressionMap.cc,v 1.2 2000/02/18 01:31:48 kumfert Exp $
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

#include "spindle/CompressionMap.h"

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

CompressionMap::CompressionMap() {
  incrementInstanceCount( CompressionMap::MetaData );
  coarse2Fine.take( new MatrixBase );
  reset();
}

bool
CompressionMap::reset() {
  currentState = EMPTY;
  return true;
}

bool 
CompressionMap:: resize( const int nUncompressed, const int nCompressed ) {
  if ( currentState == INVALID ) { return false; }
  if ( nUncompressed <= 0 ) { return false; }
  if ( nCompressed <= 0 ) { return false; }
  if ( nCompressed > nUncompressed ) { return false; }
  szCoarse = nCompressed;
  szFine = nUncompressed;
  fine2Coarse.resize( szFine );
  coarse2Fine->resize( szCoarse, szFine, szFine );
  currentState = UNKNOWN;
  return true;
}

void
CompressionMap::validate() {
  if ( currentState == VALID ) { return; }
  if ( currentState == INVALID ) { return; }
  bool fine2CoarseOkay = false;
  bool coarse2FineOkay = false;
  if ( changedMostRecent == NEITHER ) {
    // pick one
    if ( fine2CoarseOkay = validateFine2Coarse() ) {
      changedMostRecent = FINE2COARSE;
    } else if ( coarse2FineOkay = validateCoarse2Fine() ) {
      changedMostRecent = COARSE2FINE;
    } else {
      currentState = INVALID;
      return;
    }
  }
  if ( changedMostRecent == FINE2COARSE ) {
    // validate fine2coarse if neccessary
    if ( !fine2CoarseOkay ) {
      if ( !validateFine2Coarse() ) {
	currentState = INVALID;
	return;
      }
    }
    // create coarse2fine
    createCoarse2Fine();
  } else if ( changedMostRecent == COARSE2FINE ) {
    // validate coarse2Fine if neccessary
    if ( !coarse2FineOkay ) {
      if ( !validateCoarse2Fine() ) {
	currentState = INVALID;
	return;
      }
    }
    // create fine2coarse
    createFine2Coarse();
  } else {
    // error
    currentState = INVALID;
    return;
  }
  currentState = VALID;
  return;
}

bool
CompressionMap::validateFine2Coarse() {
  // If reported size and actual size differ.
  // the user must have changed the actual size
  // most recent.
  if ( szFine < fine2Coarse.size() ) { 
    currentState = INVALID;
    return false;
  }
  int max=0;
  int stop = fine2Coarse.size();
  const int * cur = fine2Coarse.begin();
  {for ( int i=0; i<stop; ++i, ++cur ) {
    if ( *cur < 0 ) {
      currentState = INVALID;
      return false;
    }
    max = ( *cur > max ) ? *cur : max ;
  }}
  szFine = fine2Coarse.size();
  szCoarse = max + 1; // 0 to n-1
  return true;
}

bool
CompressionMap::validateCoarse2Fine() {
  int nC = coarse2Fine->getColPtr().size() - 1;
  int n = coarse2Fine->getRowIdx().size();
  if ( nC > n ) { 
    currentState = INVALID;
    return false;
  }

  // const int * colPtr = coarse2Fine->getColPtr().lend(); // unused 
  const int * rowIdx = coarse2Fine->getRowIdx().lend();
  SharedArray<int> Temp( n );
  Temp.init( 0 );
  int * temp = Temp.begin();

  // check that each uncompressed vertex is represented.
  {for ( int i=0; i<n; ++i ) {
    temp[ rowIdx[i] ]++;
  }}
  {for ( int i=0; i<n; ++i ) {
    if (temp[ i ] != 1 ) {
      currentState = INVALID;
      return false;
    }
  }}
  szCoarse = nC;
  szFine = n;
  return true;
}


void 
CompressionMap::createCoarse2Fine() {
  int n = szFine;
  int nC = szCoarse;
  const int * fine2coarse = fine2Coarse.begin();

  // first find the max in fine2Coarse;
  coarse2Fine->getColPtr().resize( nC + 1);
  coarse2Fine->getColPtr().init( 0 );
  coarse2Fine->getRowIdx().resize( n );
  SharedArray<int> Temp( n );

  int * colPtr = coarse2Fine->getColPtr().begin();
  int * rowIdx = coarse2Fine->getRowIdx().begin();
  int * temp = Temp.begin();
  
  // now construct reverse map
  {for(int i=0;i<n;++i) { // count instances of each supernode
    colPtr[fine2coarse[i]+1]++;
  }}
  {for(int i=0;i<nC;++i) { // compute a cumulative sum
    colPtr[i+1]+=colPtr[i];
    temp[i] = colPtr[i];  // temp hold the next available spot in head column
  } }
  {for(int i=0;i<n;++i) {
    int j = fine2coarse[i];
    rowIdx[ temp[j]++ ] = i;
  }}
  
  // check
  // {for(i=0;i<nC;++i) {
  //   if( temp[i] != adjhead[i+1]) ERROR(SPINDLE_ERROR, "Indices do not add up!");
  // }}
  return;
}

void 
CompressionMap::createFine2Coarse() {
  int nC = szCoarse;
  //  int n = szFine; // unused 
  fine2Coarse.resize( szFine );
  fine2Coarse.init( -1 );
  const int * colPtr = coarse2Fine->getColPtr().lend();
  const int * rowIdx = coarse2Fine->getRowIdx().lend();
  int * fine2coarse = fine2Coarse.begin();
  {for (int i=0; i<nC; ++i) {
    for (int jj=colPtr[i]; jj<colPtr[i+1]; ++jj ) {
      int j = rowIdx[jj];
      fine2coarse[j]=i;
    }
  }}
}

void
CompressionMap::dump( FILE * fp ) const {
  if ( isValid() ) {
    fprintf(fp,"CompressionMap:");
    const int * p = fine2Coarse.begin(); 
    const int n = fine2Coarse.size();
    for( int i=0; i<n; i++) {
      fprintf(fp,"%d ", p[i] );
    }
  }
}

void
CompressionMap::storeObject( SpindleArchive& ar ) const {
  if ( isValid() ) {
    const int *p = fine2Coarse.lend();
    size_t my_size = fine2Coarse.size();
    pair< const int *, size_t> my_pair( p, my_size );
    ar << my_pair;
  } else { 
    ar << -1;
  }
}

void
CompressionMap::loadObject( SpindleArchive& ar ) {
  size_t new_size = ar.peekLength();
  if ( (int)new_size == -1 ) { 
    int junk;
    ar >> junk;
    currentState = INVALID;
  } else { 
    fine2Coarse.resize( new_size );
    int* p = fine2Coarse.begin();
    pair< int*, size_t > new_pair( p, new_size );
    
    ar >> new_pair;
    
    changedMostRecent = FINE2COARSE;
    validate();
  }
}

SPINDLE_IMPLEMENT_PERSISTANT( CompressionMap, SpindlePersistant )
