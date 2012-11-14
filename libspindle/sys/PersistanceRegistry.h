//
// PersistanceRegistry.h -- 
//
// $Id: PersistanceRegistry.h,v 1.2 2000/02/18 01:32:01 kumfert Exp $
//
//  Gary Kumfert, Old Dominion University
//  Copyright(c) 1997, Old Dominion University.  All rights reserved.
// 
//  Permission to use, copy, modify , distribute and sell this software and
//  its documentation for any purpose is hereby granted without fee, 
//  provided that the above copyright notice appear in all copies and
//  that both that copyright notice and this permission notice appear
//  in supporting documentation.  Old Dominion University makes no
//  representations about the suitability of this software for any 
//  purpose.  It is provided "as is" without express or implied warranty.
//
///////////////////////////////////////////////////////////////////////
//

#ifndef SPINDLE_PERSISTANCE_REGISTRY_H_
#define SPINDLE_PERSISTANCE_REGISTRY_H_

#ifndef SPINDLE_H_
#include "spindle/spindle.h"
#endif

#ifdef REQUIRE_OLD_CXX_HEADER_SUFFIX
#include <stdio.h>
#include "map.h"
#else
#include <cstdio>
#include <map>
using namespace std;
#endif

#ifndef SPINDLE_FUNC_H_
#include "spindle/spindle_func.h"
#endif

SPINDLE_BEGIN_NAMESPACE

class SpindleSystem;
class ClassMetaData;

class PersistanceRegistry { 
  friend SpindleSystem;
public:
  typedef map< const char*, const ClassMetaData*, str_less > RegistryType;

private:
  RegistryType registry;
  PersistanceRegistry( const PersistanceRegistry& src ); // no implementation
  void operator=( const PersistanceRegistry& src );      // no implementation
  PersistanceRegistry();
  FILE * dumpOnDestroy;

public:
  static PersistanceRegistry& get();

  ~PersistanceRegistry();
  void registerClass( const ClassMetaData* metaData );
  const ClassMetaData* findClass( const char* className );
  void dump( FILE * stream ) const;  
  void setDumpOnDestroy( FILE* fp) { dumpOnDestroy = fp; }
  
};

SPINDLE_END_NAMESPACE

#endif 
