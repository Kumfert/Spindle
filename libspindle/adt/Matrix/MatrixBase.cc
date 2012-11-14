//
// MatrixBase.h -- a base class for all matrix/graph classes
//
//  $Id: MatrixBase.cc,v 1.2 2000/02/18 01:31:49 kumfert Exp $
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


#include "spindle/MatrixBase.h"

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

bool 
MatrixBase::resize( const int newNCols, const int newNRows, const int newNNonZeros ) {
  
  colPtr.resize( newNCols + 1 );
  rowIdx.resize( newNNonZeros );
  if ( diagPtr.notNull() ) {
    if ( diagPtr.size() == nCols ) {
      diagPtr.resize( newNCols );
    } else { 
      diagPtr.resize( 0 );
    }
  }

  if ( realValues.notNull() ) {
    if ( realValues.size() == nNonZeros ) {
      realValues.resize( newNNonZeros );
    } else {
      realValues.resize( 0 );
    } 
  }
  
  if ( imgValues.notNull() ) {
    if ( imgValues.size() == nNonZeros ) {
      realValues.resize( newNNonZeros );
    } else {
      realValues.resize( 0 );
    }
  }

  nRows = newNRows;
  nCols = newNCols; 
  nNonZeros = newNNonZeros;
  return true;
}

bool
MatrixBase::setDiagPtr() {
  bool recomputeDiagPtr = false; 
  const int n = nCols;
  if ( diagPtr.size() != n ) {
    // there is no diagPtr, so create it
    diagPtr.resize( n );
    recomputeDiagPtr = true;
  }
  // if we aren't already sure that we must recompute
  if (!recomputeDiagPtr) { // test all values
    for(int i=0; i<n; ++i ) {
      if (rowIdx[diagPtr[i]] != i) {
        recomputeDiagPtr = true;
        break;
      }
    }
  }
  // Now recompute if neccessary
  if (recomputeDiagPtr) {
    const int* colptr = colPtr.lend();
    const int* rowidx = rowIdx.lend();
    for(int i=0; i<n ;i++) {
      for(int j=colptr[i];j<colptr[i+1];j++){
        diagPtr[i]=j;
        if(rowidx[j] == i) {
          break;
        } else if (rowidx[j] > i) {
          // error, should have explicit self edges
          return false;
        }
      }
    }
  }
  return true;
}
