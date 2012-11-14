//
//  PermutationMap.h -- an abstraction of a permutation vector.
//
//  $Id: PermutationMap.h,v 1.2 2000/02/18 01:31:49 kumfert Exp $
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
//  The PermutationMap class is meant to abstract away the concept of
//  a permutation vector which may be of old2New or new2old varieties.
//
//  A PermutationMap is not in a valid state until validate() is called
//  to insure that both old2New and new2Old are consistent with each
//  other.
//

#ifndef SPINDLE_PERMUTATION_H_
#define SPINDLE_PERMUTATION_H_

#ifndef SPINDLE_H_
#include "spindle/spindle.h"
#endif

#ifndef SPINDLE_AUTO_ARRAY_H_
#include "spindle/SharedArray.h"
#endif

#ifndef SPINDLE_PERSISTANT_H_
#include "spindle/SpindlePersistant.h"
#endif

SPINDLE_BEGIN_NAMESPACE

class PermutationMap : public SpindlePersistant {
  SPINDLE_DECLARE_PERSISTANT( PermutationMap )

private:
  PermutationMap();
  SharedArray<int> old2New;
  SharedArray<int> new2Old;

  enum { NEITHER, OLD2NEW, NEW2OLD } changedMostRecent;
  int sz;
public:
  // constructors/destructors
  PermutationMap( const int n );
  PermutationMap( const PermutationMap& perm );
  virtual ~PermutationMap();

  int size() const;

  // access specific permuation vectors
  const SharedArray<int>& getOld2New() const;
  SharedArray<int>& getOld2New();

  const SharedArray<int>& getNew2Old() const;
  SharedArray<int>& getNew2Old();


  virtual void validate();
  virtual void dump( FILE * stream ) const;

  virtual bool reset();
  void resize( const int n );
  void prettyPrint( FILE * fp, bool new2old_perm=true, bool printLength=false, 
		    bool onesOffset=false ) const; 

};

inline 
int PermutationMap::size() const {
  return sz;
}

inline 
PermutationMap::PermutationMap( const int n )
  : old2New(n), new2Old(n), sz(n) {
    incrementInstanceCount( PermutationMap::MetaData );
    reset();
}

inline 
PermutationMap::PermutationMap( const PermutationMap& perm ) 
  : sz( perm.size() ) {
    incrementInstanceCount( PermutationMap::MetaData );
    old2New.import( perm.old2New );
    new2Old.import( perm.new2Old );
    changedMostRecent = perm.changedMostRecent;
    currentState = perm.currentState;
}

inline 
PermutationMap::~PermutationMap() {
  decrementInstanceCount( PermutationMap::MetaData );
}

SPINDLE_END_NAMESPACE

#endif


