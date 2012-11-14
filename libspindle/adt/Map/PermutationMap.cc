//
// PermutationMap.cc
//
//  $Id: PermutationMap.cc,v 1.2 2000/02/18 01:31:48 kumfert Exp $
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
//  purpose.  It is povided "as is" without express or implied warranty.
//
///////////////////////////////////////////////////////////////////////
//
//  The PermutationMap class is meant to abstract away the concept of
//  a permutation vector which may be of old2New or new2old varieties.
//
//  A PermutationMap is not in a valid state until finalize() is called
//  to insure that both old2New and new2Old are consistent with each
//  other.
//

#include "spindle/PermutationMap.h"

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

PermutationMap::PermutationMap() { 
  incrementInstanceCount( PermutationMap::MetaData );
  currentState = EMPTY;
}

void
PermutationMap::validate() {
  if ( currentState == VALID ) { return; }
  int *src; 
  int *dest;
  if ( changedMostRecent == NEITHER ) {
    src = old2New.begin();
    dest = new2Old.begin();
    if ((*src >= 0)&&(*dest <= 0)) { 
      changedMostRecent = OLD2NEW; 
    } else if ((*src <= 0)&&(*dest >= 0)) { 
      changedMostRecent = NEW2OLD; 
      src = new2Old.begin();
      dest = old2New.begin();
    }
  } else if ( changedMostRecent == OLD2NEW ) { 
    src = old2New.begin();
    new2Old.init( 0 );
    dest = new2Old.begin();
  } else { // if ( changedMostRecent == NEW2OLD ) {
    src = new2Old.begin();
    old2New.init( 0 );
    dest = old2New.begin();
  }
  {for (int i=0; i<sz; i++) { 
    int perm_i = src[i];
    if ( perm_i == -1 ) { continue; }
    if ( ( perm_i < 0) ||  ( perm_i >= sz ) ) { 
      currentState = INVALID;
      return; 
    }
    dest[perm_i]++;
  }}
  // ... now make sure each vertex only occurred once
  int * p = dest;
  { for (int i=0; i<sz; i++) { 
    if ( ( *p != 1 ) && (*p != 0) ) { 
      currentState = INVALID;
      return;
    }
  }}

  // ... finally copy the reverse permutation 
  {for (int i=0; i<sz; i++) { 
    dest[ src[ i ] ] = i; 
  }}
  currentState = VALID;
  changedMostRecent = NEITHER;
  return;
}

bool
PermutationMap::reset() {
  old2New.init( -1 );
  new2Old.init( -1 );
  currentState = EMPTY;
  changedMostRecent = NEITHER;
  return true;
}


const SharedArray<int>& 
PermutationMap::getOld2New() const {
  return old2New;
}

SharedArray<int>& 
PermutationMap::getOld2New() {
  if ( (!isValid() ) && (changedMostRecent == NEW2OLD) ) {
    validate();
    assert( isValid() );
  }
  currentState = UNKNOWN;
  changedMostRecent = OLD2NEW;
  return old2New;
}

const SharedArray<int>& 
PermutationMap::getNew2Old() const {
  return new2Old;
}

SharedArray<int>& 
PermutationMap::getNew2Old() {
  if ((!isValid()) && (changedMostRecent == OLD2NEW) ) {
    validate();
    assert( isValid() );
  }
  currentState = UNKNOWN;
  changedMostRecent = NEW2OLD;
  return new2Old;
}

void 
PermutationMap::resize( const int n ) { 
  currentState = EMPTY;
  changedMostRecent = NEITHER;
  sz = n;
  old2New.resize(n);
  new2Old.resize(n);
}

void
PermutationMap::dump( FILE * fp ) const {
  if ( isValid() ) {
    fprintf(fp,"PermutationMap:");
    const int* p = getNew2Old().lend();
    for( int i=0; i<size(); i++) {
      fprintf(fp,"%d ", p[i] );
    }
  }
}

void
PermutationMap::prettyPrint( FILE * fp, bool new2old_perm, bool printLength, 
			     bool onesOffset ) const { 

  if ( !isValid() ) {
    return ;
  }
  int offset = (onesOffset) ? 1 : 0 ;
  
  if ( printLength == true ) {
    fprintf( fp,"%d\n", size() );
  }
  
  const int * p;
  if ( new2old_perm ) { 
    p = new2Old.lend();
  } else { 
    p = old2New.lend();
  }

  for(const int* cur = p, *stop=p + size(); cur <stop; ++cur ) {
    fprintf( fp, "%d\n", *cur + offset);
  }
} 

void
PermutationMap::storeObject( SpindleArchive& ar ) const {
  if ( isValid() ) {
    const int *p = getNew2Old().lend();
    size_t my_size = size();
    pair< const int*, size_t> my_pair( p,my_size );
    ar << my_pair;
  } else {
    ar << -1;
  }
}
#define __FUNC__ "PermutationMap::loadObject( SpindleArchive& ar )"
void
PermutationMap::loadObject( SpindleArchive& ar ) {
  size_t new_size = ar.peekLength();
  if ( (int)new_size == -1 ) { 
    int junk;
    ar >> junk;
    currentState = INVALID;
  } else {
    resize(new_size);
    int* p = getNew2Old().begin();
    pair<int*, size_t> new_pair( p, new_size );
    
    ar >> new_pair;
    
    WARN_IF( new_pair.second != new_size, "Warning, expected size=%d, but extracted only %d.", 
	     new_pair.second, new_size );
    changedMostRecent = NEW2OLD;
    validate();
  }
}
#undef __FUNC__

SPINDLE_IMPLEMENT_PERSISTANT( PermutationMap, SpindlePersistant )


