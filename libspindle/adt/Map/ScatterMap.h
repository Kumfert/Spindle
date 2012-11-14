//
//  ScatterMap.h -- an abstraction of a loc2glob | glob2loc mapping.
//
//  $Id: ScatterMap.h,v 1.2 2000/02/18 01:31:49 kumfert Exp $
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
 
#ifndef SPINDLE_SCATTER_MAP_H_
#define SPINDLE_SCATTER_MAP_H_
 
#ifndef SPINDLE_H_
#include "spindle/spindle.h"
#endif

#ifndef SPINDLE_PERSISTANT_H_
#include "spindle/SpindlePersistant.h"
#endif

#ifndef SPINDLE_SHARED_ARRAY_H_
#include "spindle/SharedArray.h"
#endif

#ifndef SPINDLE_SHARED_PTR_H_
#include "spindle/SharedPtr.h"
#endif
 
#include <vector.h>
#include <hash_map.h>

SPINDLE_BEGIN_NAMESPACE

class ScatterMap : public SpindlePersistant {
  SPINDLE_DECLARE_PERSISTANT( ScatterMap )

public: 
  typedef hash_map< int, int > glob2loc_t;
protected: 
  SharedArray<int>      loc2glob;
  SharedPtr<glob2loc_t> glob2loc;
  
  enum { NEITHER, LOC2GLOB, GLOB2LOC } changedMostRecent;
  ScatterMap();
public:
  ScatterMap( int n );  // create an empty map of size n
  ScatterMap( const int n, int* local2Global ); 
  ScatterMap( const int n, const int* local2Global ); 
  virtual ~ScatterMap();
  
  int size() const ; // return number of items
  int queryMaxGlobal() const;
  int queryMinGlobal() const;

  const SharedArray<int>& getLoc2Glob() const;
  SharedArray<int>& getLoc2Glob();

  const SharedPtr<glob2loc_t>& getGlob2Loc() const;
  SharedPtr<glob2loc_t>& getGlob2Loc();

  virtual void validate();
  virtual void dump( FILE * stream ) const;
 
  virtual bool reset();
  void resize( const int n );
};

inline
int ScatterMap::size() const {
  return loc2glob.size();
}


inline 
const SharedArray<int>& ScatterMap::getLoc2Glob() const {
  return loc2glob;
}

inline
const SharedPtr<ScatterMap::glob2loc_t>& ScatterMap::getGlob2Loc() const {
  return glob2loc;
}

SPINDLE_END_NAMESPACE

#endif
