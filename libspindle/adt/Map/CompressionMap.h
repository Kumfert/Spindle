//
//  CompressionMap.h -- an abstraction of a compression mapping
//
//  $Id: CompressionMap.h,v 1.2 2000/02/18 01:31:48 kumfert Exp $
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
//  This class is meant to abstract away the details of a compression
//  mapping.  It has a fine2coarse vector and a coarse2fine matrix.
//  Given one, it will create the other.
//
//  A CompressionMap is not in a valid state until validate() is called
//  to insure that both fine2coarse and coarse2fine are consistent with 
//  each other.
//

#ifndef SPINDLE_COMPRESSION_MAP_H_
#define SPINDLE_COMPRESSION_MAP_H_

#ifndef SPINDLE_H_
#include "spindle/spindle.h"
#endif

#ifndef SPINDLE_AUTO_ARRAY_H_
#include "spindle/SharedArray.h"
#endif

#ifndef SPINDLE_AUTO_PTR_H_
#include "spindle/SharedPtr.h"
#endif

#ifndef SPINDLE_MATRIX_BASE_H_
#include "spindle/MatrixBase.h"
#endif

#ifndef SPINDLE_PERSISTANT_H_
#include "spindle/SpindlePersistant.h"
#endif

SPINDLE_BEGIN_NAMESPACE

class CompressionMap : public SpindlePersistant {
  SPINDLE_DECLARE_PERSISTANT( CompressionMap )
    
private:
  CompressionMap();
  bool validateFine2Coarse();
  bool validateCoarse2Fine();
  void createFine2Coarse();
  void createCoarse2Fine();
  SharedArray<int>      fine2Coarse;
  SharedPtr<MatrixBase> coarse2Fine;

  enum { NEITHER, FINE2COARSE, COARSE2FINE } changedMostRecent;
  int szCoarse;
  int szFine;
public:
  CompressionMap( const int nUncompressed, const int nCompressed );
  CompressionMap( const CompressionMap& cmap );
  virtual ~CompressionMap();

  int queryNCompressed() const;
  int queryNUncompressed() const;

  const SharedArray<int>& getFine2Coarse() const { return fine2Coarse; }
  SharedArray<int>& getFine2Coarse() { changedMostRecent=FINE2COARSE; return fine2Coarse; }

  const SharedPtr<MatrixBase>& getCoarse2Fine() const { return coarse2Fine; }
  SharedPtr<MatrixBase>& getCoarse2Fine() { changedMostRecent=COARSE2FINE; return coarse2Fine; }

  virtual void validate();
  virtual void dump( FILE * fp ) const;
  virtual bool reset();
  bool resize(  const int nUncompressed, const int nCompressed );

};

inline
int CompressionMap::queryNCompressed() const {
  return szCoarse;
}

inline 
int CompressionMap::queryNUncompressed() const {
  return szFine;
}

inline
CompressionMap::CompressionMap( const int nUncompressed, const int nCompressed ) {
  incrementInstanceCount( CompressionMap::MetaData );
  reset();
  coarse2Fine.take( new MatrixBase );
  resize( nUncompressed, nCompressed );
}

inline
CompressionMap::CompressionMap( const CompressionMap& cmap ) 
  : szCoarse( cmap.szCoarse ), szFine( cmap.szFine ) {
    incrementInstanceCount( CompressionMap::MetaData );
    fine2Coarse.import( cmap.fine2Coarse );
    coarse2Fine.import( cmap.coarse2Fine.operator->() );
    changedMostRecent = cmap.changedMostRecent;
    currentState = cmap.currentState;
}

inline
CompressionMap::~CompressionMap() {
  decrementInstanceCount( CompressionMap::MetaData );
}

SPINDLE_END_NAMESPACE

#endif
