//
// ClassMetaData.h
//
// $Id: ClassMetaData.cc,v 1.2 2000/02/18 01:32:00 kumfert Exp $
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

#include "spindle/ClassMetaData.h"

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

SpindleBaseClass*
ClassMetaData::createObject() {
  if (pfnCreateObject == 0) { 
    return 0;
  } else { 
    return (*pfnCreateObject)();
  }
}

bool 
ClassMetaData::isDerivedFrom( const ClassMetaData* p ) const {
  for ( const ClassMetaData* parent = this; parent != 0; parent = parent->parentMetaData ) {
    if ( parent == p ) { 
      return true;
    }
  }
  return false;
}
