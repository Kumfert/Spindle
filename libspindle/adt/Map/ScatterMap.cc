//
//  ScatterMap.cc -- an abstraction of a loc2glob | glob2loc mapping.
//
//  $Id: ScatterMap.cc,v 1.2 2000/02/18 01:31:49 kumfert Exp $
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
//  The ScatterMap class is meant to abstract away the concept of
//  a scattering of global items into a subset of local ones.
//  It encapsulates the one-to-one, but not onto relationship
//  of loc2glob and glob2loc (local to global and global to local)
//
//  A ScatterMap is not in a valid state until validate() is called
//  to insure that both loc2glob and glob2loc are consistent with each
//  other.
//

#include "spindle/ScatterMap.h"

#ifndef SPINDLE_ARCHIVE_H_
#include "spindle/SpindleArchive.h"
#endif
 
#ifndef SPINDLE_SYSTEM_H_
#include "spindle/SpindleSystem.h"
#endif

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

ScatterMap::ScatterMap() {
  incrementInstanceCount( ScatterMap::MetaData );
  currentState = EMPTY;
  reset();
}

// create an empty map of size n
ScatterMap::ScatterMap( int n ) {
  incrementInstanceCount( ScatterMap::MetaData );
  currentState = EMPTY;
  resize(n);
}

// creates map of globalIndex[ local ] = global
ScatterMap::ScatterMap( const int n, int* local2Global )
  : loc2glob( local2Global, n ) {
  incrementInstanceCount( ScatterMap::MetaData );
  currentState = UNKNOWN;
  changedMostRecent = LOC2GLOB;
  validate();
}

ScatterMap::ScatterMap( const int n, const int* local2Global ) 
  : loc2glob( local2Global, n ) {
  incrementInstanceCount( ScatterMap::MetaData );
  currentState = UNKNOWN;
  changedMostRecent = LOC2GLOB;
  validate();
}

ScatterMap::~ScatterMap() {
  decrementInstanceCount( ScatterMap::MetaData );
} 
 
int ScatterMap::queryMaxGlobal() const {
  if ( !isValid() ) { 
    return -1;
  } 
  int max = -1;
  for( const int* cur = loc2glob.begin(), *stop = loc2glob.end(); cur != stop; ++cur ) {
    if ( *cur > max ) {
      max = *cur;
    }
  } 
  return max;
}


int ScatterMap::queryMinGlobal() const {
  if ( !isValid() ) { 
    return -1;
  } 
  if ( loc2glob.size() < 1 ) {
    return -1;
  }
  int min = *(loc2glob.begin());
  for( const int* cur = loc2glob.begin(), *stop = loc2glob.end(); cur != stop; ++cur ) {
    if ( *cur < min ) {
      min = *cur;
    }
  } 
  return min;
}

SharedArray<int>& ScatterMap::getLoc2Glob() {
  if ( (!isValid() ) && ( changedMostRecent == GLOB2LOC) ) {
    validate();
    if ( !isValid() ) {
      loc2glob.init(0);
    }
  }
  currentState = UNKNOWN;
  changedMostRecent = LOC2GLOB;
  return loc2glob;
}

SharedPtr<ScatterMap::glob2loc_t>& ScatterMap::getGlob2Loc() {
  if ( (!isValid() ) && ( changedMostRecent == LOC2GLOB) ) {
    validate();
    if ( !isValid() ) {
      if ( glob2loc.notNull() ) { 
	glob2loc->erase( glob2loc->begin(), glob2loc->end() );
      }
    }
  }
  currentState = UNKNOWN;
  changedMostRecent = GLOB2LOC;
  return glob2loc;
}

bool ScatterMap::reset() {
  loc2glob.init(-1);
  if ( glob2loc.notNull() ) {
    glob2loc->erase( glob2loc->begin(), glob2loc->end() );
  } else { 
    glob2loc.take( new glob2loc_t() );
  }
  currentState = EMPTY;
  changedMostRecent = NEITHER;
  return true;
}

void ScatterMap::resize( const int n ) {
  currentState = EMPTY;
  changedMostRecent = NEITHER;
  loc2glob.resize( n );
  if ( glob2loc.notNull() ) {
    glob2loc->erase( glob2loc->begin(), glob2loc->end() );
  }
}

void ScatterMap::validate() { 
  if ( currentState == VALID ) { return ; }
  if ( changedMostRecent == NEITHER ) {
    // try to detect if something has 
    // meaningful information in it
    if ( loc2glob.size() > 0 ) {
      changedMostRecent = LOC2GLOB;
    } else { 
      changedMostRecent = GLOB2LOC;
    }
  }
  if ( changedMostRecent == LOC2GLOB ) {
    // load glob2loc
    if ( glob2loc.notNull() ) { 
      glob2loc->erase( glob2loc->begin(), glob2loc->end() );
    } else { 
      glob2loc.take( new glob2loc_t() );
    }
    glob2loc_t & g2l = (*glob2loc);
    int i = 0;
    for( const int* cur = loc2glob.begin(), *stop = loc2glob.end(); cur != stop; ++cur,++i ) {
      g2l[*cur] = i;
    }
    if ( g2l.size() != (size_t) loc2glob.size() ) {
      // size mismatch, must have double entries
      currentState = INVALID;
    } else {
      currentState = VALID;
    }
    return;
  } else if ( changedMostRecent == GLOB2LOC ) {
    currentState = VALID;
    glob2loc_t & g2l = (*glob2loc);

    int max = g2l.size();
    loc2glob.resize( max );
    loc2glob.init(-1);
    for( glob2loc_t::const_iterator cur = g2l.begin(), stop = g2l.end(); cur != stop; ++cur ) {
      if ( (*cur).second < max ) { 
	loc2glob[ (*cur).second ] = (*cur).first;
      }
    }
    if ( find( loc2glob.begin(), loc2glob.end(), -1 ) != loc2glob.end() ) {
      // found a -1 in the range
      currentState = INVALID;
    } else { 
      currentState = VALID;
    }
    return;
  }
  // should never get here
  currentState = INVALID;
  return ;
} 

void ScatterMap::dump( FILE * stream ) const { 
  if ( isValid() ) {
    for( int i =0; i<loc2glob.size(); ++i ) {
      fprintf( stream, " scatter[ %d ] = %d\n", i, loc2glob[i] );
    }
  }
}
 
void ScatterMap::storeObject( SpindleArchive& ar ) const {
  if ( isValid() ) {
    const int *p = loc2glob.lend();
    size_t my_size = loc2glob.size();
    pair< const int*, size_t> my_pair( p,my_size );
    ar << my_pair;
  } else {
    ar << -1;
  }
}

void ScatterMap::loadObject( SpindleArchive& ar ) {
  size_t new_size = ar.peekLength();
  if ( (int)new_size == -1 ) {
    int junk;
    ar >> junk;
    currentState = INVALID;
  } else {
    resize(new_size);
    int* p = getLoc2Glob().begin();
    pair<int*, size_t> new_pair( p, new_size );
    
    ar >> new_pair; 
    changedMostRecent = LOC2GLOB;
    validate();
  }
}


SPINDLE_IMPLEMENT_PERSISTANT( ScatterMap, SpindlePersistant )
